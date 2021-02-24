SP = {
    name = "var", 
    module="foo", 
    args = {},
    h = nil,
    c = nil,
    mode = 0,
    nchan = 1
}


function SP:begin(o)
    o = o or {}
    setmetatable(o, self)
    self.__index = self
    return o
end

function SP:set_header(ft)
    self.h = ft
end

function SP:set_source(ft)
    self.c = ft
end

function SP:new(name, module, args, init)
    o = self:begin()
    o.name = name
    o.module = module
    o.args = args
    o.icb = init or nil
    return o
end

function SP:var()
    return string.format("sp_%s *%s;", self.module, self.name)
end

function SP:create()
    return string.format("sp_%s_create(&%s);", self.module, self:obj())
end

function SP:destroy()
    return string.format("sp_%s_destroy(&%s);", self.module, self:obj())
end

function SP:compute(args)
    str = string.format("sp_%s_compute(sp, %s", self.module, self:obj())
    for _,v in pairs(args) do
        if(type(v) ~= "number") then
            str = str .. string.format(", &%s", v:obj())
        else
            if(v < 0) then
                str = str .. string.format(", NULL", v)
            else
                str = str .. string.format(", &tmp[%d]", v)
            end
        end
    end
    str = str .. ");"
    print8(self.c, str)
end

function SP:set(val, pos)
    local var
    if(type(pos) == "string") then
        var = pos
    else
        var = string.format("tmp[%d]", pos)
    end
    str = string.format("synth->%s->%s = %s;", self.name, val, var)
    print8(self.c, str)
end

-- For pesky faust objects
function SP:setstar(val, pos)
    str = string.format("*synth->%s->%s = tmp[%d];", self.name, val, pos)
    print8(self.c, str)
end

function SP:assign(val, var)
    str = string.format("synth->%s->%s = %s;", self.name, val, var)
    return str
end

function SP:init()

    str = string.format("sp_%s_init(sp, %s", self.module, self:obj())

    for k,v in pairs(self.args) do
        str = str .. string.format(", %s", v)
    end

    str = str .. ");"
    return str
end

function SP:obj()
    return string.format("synth->%s", self.name)
end

function print_struct(synth)
    fp = SP.h
    fp:write("struct sp_synth {\n")
    fp:write("    sp_data *sp;\n") 
    for _, v in pairs(synth) do
        str = v:var()
        if(str ~= nil) then fp:write("    "..v:var().."\n") end
    end

    fp:write("};\n\n")
end

FT = SP:begin()
GEN = SP:begin()
VAR = SP:begin()

function FT:new(name, size)
    o = self:begin()
    o.size = size
    o.name = name
    o.module = "ftbl"
    return o
end

function FT:create()
    return string.format("sp_ftbl_create(sp, &%s, %d);", self:obj(), self.size)
end

function FT:init()
    return nil
end

function GEN:new(ft, module, args)
    o = self:begin()
    o.args = args or {}
    o.name = ft.name
    o.module = module
    return o
end

function GEN:var() return nil end
function GEN:create() return nil end
function GEN:destroy() return nil end
function GEN:compute() return nil end

function GEN:init()
    str = string.format("sp_%s(sp, %s", self.module, self:obj())

    for k,v in pairs(self.args) do
        if(type(v) == "string") then
            str = str .. string.format(", \"%s\"", v)
        else
            str = str .. string.format(", %s", v)
        end
    end

    str = str .. ");"
    print4(self.c, str)
end

function VAR:new(name, ival) 
    o = self:begin()
    o.name = name
    o.args[1] = ival
    return o
end

function VAR:var()
    return string.format("SPFLOAT %s;", self.name)
end

function VAR:compute() return nil end
function VAR:create() return nil end
function VAR:destroy() return nil end
function VAR:init() return string.format("synth->%s = %g;", self.name, self.args[1]) end

function print_create(fp, synth)
    fp:write("int sp_synth_create(sp_data *sp, sp_synth *synth)\n")
    fp:write("{\n")
    for k,v in pairs(synth) do
        str = v:create()
        if(str ~= nil) then fp:write("    " .. str .. "\n") end
    end

    fp:write("    return SP_OK;\n")
    fp:write("}\n\n")
end

function print_destroy(fp, synth)
    fp:write("int sp_synth_destroy(sp_data *sp, sp_synth *synth)\n")
    fp:write("{\n")
    for k,v in pairs(synth) do
        str = v:destroy()
        if(str ~= nil) then fp:write("    " .. str .. "\n") end
    end

    fp:write("    return SP_OK;\n")
    fp:write("}\n\n")
end

function print_init(fp, synth)
    fp:write("int sp_synth_init(sp_data *sp, sp_synth *synth)\n")
    fp:write("{\n")

    fp:write("    " .. "synth->sp = sp;\n")
    for k,v in pairs(synth) do
        str = v:init()
        if(str ~= nil) then fp:write("    " .. str .. "\n") end
        if(v.icb ~= nil) then v.icb(fp, v) end
    end

    fp:write("    return SP_OK;\n")
    fp:write("}\n\n")
end

function print_compute(fp, synth, stack_size, cb)
    if(SP.mode == 0) then
        fp:write("int sp_synth_computef(sp_data *sp, sp_synth *synth, int bufsize, SPFLOAT *buf)\n")
    else
        fp:write("int sp_synth_computei(sp_data *sp, sp_synth *synth, int bufsize, short *buf)\n")
    end
    fp:write("{\n")
    fp:write(string.format("    SPFLOAT tmp[%d];\n", stack_size))
    fp:write("    int i;\n");
    if(SP.nchan > 1) then
    fp:write(string.format("    for(i = 0; i < bufsize / %d; i+=%d) {\n", 
        SP.nchan,
        SP.nchan))
    else
    fp:write(string.format("    for(i = 0; i < bufsize ; i++) {\n", 
        SP.nchan,
        SP.nchan))
    end
    cb(fp, synth)
    fp:write("    }\n")
    fp:write("    return SP_OK;\n")
    fp:write("}\n")
end


function SP:print_header(synth)
    fp = self.h

    fp:write("#ifndef SP_SYNTH_TYPEDEF\n")
    fp:write("#define SP_SYNTH_TYPEDEF\n")
    fp:write("typedef struct sp_synth sp_synth;\n")
    fp:write("#endif\n\n")

    print_struct(synth)
    if(SP.mode == 0) then
        fp:write("int sp_synth_computef(sp_data *sp, sp_synth *synth, int bufsize, SPFLOAT *buf);\n")
    else
        fp:write("int sp_synth_computei(sp_data *sp, sp_synth *synth, int bufsize, short *buf);\n")
    end
    fp:write("int sp_synth_create(sp_data *sp, sp_synth *synth);\n")
    fp:write("int sp_synth_destroy(sp_data *sp, sp_synth *synth);\n")
    fp:write("int sp_synth_init(sp_data *sp, sp_synth *synth);\n")
end

function SP:print_synth(synth, callback, num)
    fp = self.c
    fp:write("#include <math.h>\n")
    fp:write("#include \"soundpipe.h\"\n")
    fp:write("#include \"modal.h\"\n")
    fp:write("#include \"revscm.h\"\n")
    fp:write("#include \"critter.h\"\n")
    fp:write("#include \"synth.h\"\n")

    num = num or 4
    print_create(fp, synth)
    print_destroy(fp, synth)
    print_init(fp, synth)
    self.mode = 0
    print_compute(fp, synth, num, callback)
    fp:write("\n")
    self.mode = 1
    print_compute(fp, synth, num, callback)
end

function print8(fp, s)
    fp:write("        " .. s .. "\n")
end

function print4(fp, s)
    fp:write("    " .. s .. "\n")
end

function SP:output(num)
    if(self.mode == 0) then
        print8(self.c, string.format("buf[i] = tmp[%d];", num))
    else
        print8(self.c, string.format("buf[i] = tmp[%d] * 32767;", num))
    end
end

function SP:outputN(num, n)
    if(self.output == 0) then
        print8(self.c, string.format("buf[i + %d] = tmp[%d];", num, n))
    else
        print8(self.c, string.format("buf[i + %d] = tmp[%d] * 32767;", num, n))
    end
end

function SP:var_set(x, val)
    varstr, valstr = SP:get_vars(x, val)
    print8(self.c, string.format("%s = %s;", varstr, valstr))
end

function SP:var_bias(x, val)
    print8(self.c, string.format("tmp[%d] += %g;", x, val))
end

function SP:var_scale(x, val)
    print8(self.c, string.format("tmp[%d] *= %g;", x, val))
end

function SP:var_copy(v1, v2)
    print8(self.c, string.format("tmp[%d] = tmp[%d];", v1, v2))
end

function ivar(fp, n, var, name)
    if(var ~= nil) then print4(fp, n:assign(name, var)) end
end

-- for faust-generated modules
function ivar_star(fp, n, var, name)
    if(var ~= nil) then print4(fp, "*"..n:assign(name, var)) end
end

require("spgen.mods")
