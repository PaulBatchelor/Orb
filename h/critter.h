/*
_clk get 0.01 0.1 0.1 tenv 
_clk get 10 30 trand metro 0.001 0.001 0.001 tenvx 
_clk get 800 3000 trand
0.4 1 1 1 fm * *

dup 
0.3 0.9 1 randi
0.001 0.3 10 randi
1 vdelay + 
*/
typedef struct {
    sp_tenv *tenv;
    sp_trand *trand1;
    sp_tenvx *tenvx;
    sp_metro *metro;
    sp_trand *trand2;
    sp_osc *fm;
    sp_rline *rline1;
    sp_rline *rline2;
    sp_vardelay *vdelay;
    SPFLOAT prev;
} sp_critter;

int sp_critter_create(sp_critter **crit);
int sp_critter_destroy(sp_critter **crit);
int sp_critter_init(sp_data *sp, sp_critter *crit, sp_ftbl *ft);
int sp_critter_compute(sp_data *sp, sp_critter *crit, SPFLOAT *in, SPFLOAT *out);
