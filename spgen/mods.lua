function SP:tenvx(name, atk, hold, rel)
    atk = atk or 0.001
    hold = hold or 0.01
    rel = rel or 0.01
    return SP:new(name, "tenvx", {}, 
    function (fp, n)
        print4(fp, n:assign("atk", atk))
        print4(fp, n:assign("hold", hold))
        print4(fp, n:assign("rel", rel))
    end)
end

function SP:maytrig(name, prob)
    atk = atk or 0.001
    hold = hold or 0.01
    rel = rel or 0.01
    return SP:new(name, "maygate", {}, 
    function (fp, n)
        print4(fp, n:assign("prob", prob))
        print4(fp, n:assign("mode", 1))
    end)
end

function SP:clock(name, bpm, sub)
    return SP:new(name, "clock", {}, 
    function (fp, n)
        ivar(fp, n, bpm, "bpm")
        ivar(fp, n, sub, "subdiv")
    end)
end

function SP:trand(name, min, max)
    return SP:new(name, "trand", {}, 
    function (fp, n)
        ivar(fp, n, min, "min")
        ivar(fp, n, max, "max")
    end)
end

function SP:fosc(name, ft, freq, amp, car, mod, indx)
    return SP:new(name, "fosc", {ft:obj()}, 
    function (fp, n)
        ivar(fp, n, freq, "freq")
        ivar(fp, n, amp, "amp")
        ivar(fp, n, car, "car")
        ivar(fp, n, mod, "mod")
        ivar(fp, n, indx, "indx")
    end)
end

function SP:blsaw(name, freq, amp)
    return SP:new(name, "blsaw", {}, 
    function (fp, n)
        ivar_star(fp, n, freq, "freq")
        ivar_star(fp, n, amp, "amp")
    end)
end

function SP:moogladder(name, freq, res)
    return SP:new(name, "moogladder", {}, 
    function (fp, n)
        ivar(fp, n, freq, "freq")
        ivar(fp, n, res, "res")
    end)
end

function SP:zrev(name, rt60_low, rt60_mid, hf_damping)
    return SP:new(name, "zitarev", {}, 
    function (fp, n)
        ivar_star(fp, n, rt60_low, "rt60_low")
        ivar_star(fp, n, rt60_mid, "rt60_mid")
        ivar_star(fp, n, hf_damping, "hf_damping")
    end)
end

function SP:tseq(name, ft, mode)
    return SP:new(name, "tseq", {ft:obj()}, 
    function (fp, n)
        ivar(fp, n, mode, "shuf")
    end)
end

function SP:buthp(name, freq)
    return SP:new(name, "buthp", {}, 
    function (fp, n)
        ivar(fp, n, freq, "freq")
    end)
end


function SP:buthp(name, freq)
    return SP:new(name, "buthp", {}, 
    function (fp, n)
        ivar(fp, n, freq, "freq")
    end)
end

function SP:port(name, htime)
    return SP:new(name, "port", {htime}, 
    function (fp, n)
    end)
end

function SP:mtof(freq, nn)
    return print8(self.c, string.format("tmp[%d] = sp_midi2cps(tmp[%d]);", freq, nn))
end

function SP:vfloor(x, y)
    return print8(self.c, string.format("tmp[%d] = floor(tmp[%d]);", x, y))
end

function SP:intmode()
    self.mode = 1
end

function SP:nchan(nchan)
    self.nchan = nchan
end

function SP:scale(var, min, max)
    if(type(var) ~= "number") then
        return string.format("%g + (%s * %g)", min, var:obj(), (max - min))
    else 
        return string.format("%g + (tmp[%d] * %g)", min, var, (max - min))
    end
end

function SP:biscale(var, min, max)
    if(type(var) ~= "number") then
        return string.format("0.5 * (%g + 1)  + (%s * %g)", min, var:obj(), (max - min))
    else 
        return string.format("0.5 * (%g + 1) + (tmp[%d] * %g)", min, var, (max - min))
    end
end

function SP:osc(name, ft, freq, amp, phase)
    freq = freq or 440
    amp = amp or 0.2
    phase = phase or 0
    return SP:new(name, "osc", {ftbl:obj(), phase}, 
        function (fp, sine)
            print4(fp, sine:assign("freq", freq))
            print4(fp, sine:assign("amp", amp))
        end)
end

function SP:randh(name, min, max, rate)
    min = min or 200
    max = max or 800
    rate = rate or 10

    return SP:new(name, "randh", {}, 

    function (fp, rh)
        print4(fp, rh:assign("min", min))
        print4(fp, rh:assign("max", max))
        print4(fp, rh:assign("freq", rate))
    end)
end

function SP:metro(name, rate)
    rate = rate or 10

    return SP:new(name, "metro", {}, 

    function (fp, rh)
        print4(fp, rh:assign("freq", rate))
    end)
end

function SP:gen_sine(ft)
    return GEN:new(ft, "gen_sine")
end

function SP:gen_rand(ft, argstring)
    return GEN:new(ft, "gen_rand", { argstring })
end

function SP:add(v1, v2, v3)
    print8(self.c, string.format("tmp[%d] = tmp[%d] + tmp[%d];", v1, v2, v3))
end

function SP:mul(v1, v2, v3)
    print8(self.c, string.format("tmp[%d] = tmp[%d] * tmp[%d];", v1, v2, v3))
end

function SP:var_biscale(var, min, max)
    print8(self.c, string.format("tmp[%d] = %g+ (0.5 * (tmp[%d] + 1)) * %g;", 
        var, min, var, (max - min)))
end


function SP:get_vars(x, val)
    local varstr
    local valstr

    if(type(x) == "string") then
        varstr = x 
    else
        varstr = string.format("tmp[%d]", x)
    end

    if(type(val) == "string") then
        valstr = val
    else
        valstr = string.format("%g", val)
    end
    return varstr, valstr
end

function SP:ftbl(name, size)
    return FT:new(name, size)
end

function SP:thresh(name, thresh, mode)
    return SP:new(name, "thresh", {}, 
    function (fp, n)
        ivar(fp, n, thresh, "thresh")
        ivar(fp, n, mode, "mode")
    end)
end

function SP:revsc(name, decay, cutoff)
    return SP:new(name, "revsc", {}, 
    function (fp, n)
        ivar(fp, n, decay, "feedback")
        ivar(fp, n, cutoff, "lpfreq")
    end)
end

function SP:delay(name, feedback, dtime)
    return SP:new(name, "delay", {dtime}, 
    function (fp, n)
        ivar(fp, n, feedback, "feedback")
    end)
end

function SP:butlp(name, freq)
    return SP:new(name, "butlp", {}, 
    function (fp, n)
        ivar(fp, n, freq, "freq")
    end)
end

function SP:modal(name, freq)
    return SP:new(name, "modal", {}, 
    function (fp, n)
    end)
end

function SP:randi(name, min, max, freq)
    return SP:new(name, "randi", {}, 
    function (fp, n)
        ivar(fp, n, min, "min")
        ivar(fp, n, max, "max")
        ivar(fp, n, freq, "cps")
    end)
end

function SP:tenv(name, atk, hold, rel)
    return SP:new(name, "tenv", {}, 
    function (fp, n)
        ivar(fp, n, atk, "atk")
        ivar(fp, n, hold, "hold")
        ivar(fp, n, rel, "rel")
    end)
end

function SP:dmetro(name, time)
    return SP:new(name, "dmetro", {}, 
    function (fp, n)
        ivar(fp, n, time, "time")
    end)
end


function SP:osc(name, ft, freq, amp, phs)
    return SP:new(name, "osc", {ft:obj(), phs}, 
    function (fp, n)
        ivar(fp, n, freq, "freq")
        ivar(fp, n, amp, "amp")
    end)
end

function SP:revscm(name, decay, cutoff)
    return SP:new(name, "revscm", {}, 
    function (fp, n)
        ivar(fp, n, decay, "feedback")
        ivar(fp, n, cutoff, "lpfreq")
    end)
end

function SP:critter(name, ft)
    return SP:new(name, "critter", {ft:obj()}, 
    function (fp, n)
    end)
end

function SP:rspline(name, min, max, min_cps, max_cps)
    return SP:new(name, "rspline", {}, 
    function (fp, n)
        ivar(fp, n, min, "min")
        ivar(fp, n, max, "max")
        ivar(fp, n, min_cps, "cps_min")
        ivar(fp, n, max_cps, "cps_max")
    end)
end
