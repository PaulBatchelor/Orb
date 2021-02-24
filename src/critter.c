#include <stdlib.h>
#include "nanovg.h"
#include "orb.h"
#include "soundpipe.h"
#include "critter.h"

int sp_critter_create(sp_critter **crit)
{
    sp_critter *pcrit;
    *crit = malloc(sizeof(sp_critter));
    pcrit = *crit;

    sp_tenv_create(&pcrit->tenv);
    sp_trand_create(&pcrit->trand1);
    sp_trand_create(&pcrit->trand2);
    sp_tenvx_create(&pcrit->tenvx);
    sp_metro_create(&pcrit->metro);
    sp_osc_create(&pcrit->fm);
    sp_rline_create(&pcrit->rline1);
    sp_rline_create(&pcrit->rline2);
    sp_vardelay_create(&pcrit->vdelay);
    return SP_OK;
}

int sp_critter_destroy(sp_critter **crit)
{
    sp_critter *pcrit;
    pcrit = *crit;
    sp_tenv_destroy(&pcrit->tenv);
    sp_trand_destroy(&pcrit->trand1);
    sp_trand_destroy(&pcrit->trand2);
    sp_tenvx_destroy(&pcrit->tenvx);
    sp_metro_destroy(&pcrit->metro);
    sp_osc_destroy(&pcrit->fm);
    sp_rline_destroy(&pcrit->rline1);
    sp_rline_destroy(&pcrit->rline2);
    sp_vardelay_destroy(&pcrit->vdelay);
    free(*crit);
    return SP_OK;
}

int sp_critter_init(sp_data *sp, sp_critter *crit, sp_ftbl *ft)
{
    sp_tenv_init(sp, crit->tenv);
    crit->tenv->atk = 0.01;
    crit->tenv->hold = 0.05;
    crit->tenv->rel = 0.01;
    
    sp_tenvx_init(sp, crit->tenvx);
    crit->tenvx->atk = 0.001;
    crit->tenvx->hold = 0.001;
    crit->tenvx->rel = 0.001;

    sp_trand_init(sp, crit->trand1);
    crit->trand1->min = 20;
    crit->trand1->max = 60;

    sp_metro_init(sp, crit->metro);
    crit->metro->freq = 10;
    
    sp_trand_init(sp, crit->trand2);
    crit->trand2->min = 1000;
    crit->trand2->max = 4000;
    
    sp_osc_init(sp, crit->fm, ft, 0);
    //crit->fm->car = 1;
    //crit->fm->mod = 1;
    //crit->fm->indx = 1;
    crit->fm->amp = 0.25;

    sp_rline_init(sp, crit->rline1);
    crit->rline1->min = 0.2;
    crit->rline1->max = 0.9;
    crit->rline1->cps = 1;

    sp_rline_init(sp, crit->rline2);
    crit->rline2->min = 0.05;
    crit->rline2->max = 0.3;
    crit->rline2->cps = 10;

    sp_vardelay_init(sp, crit->vdelay, 1.0);

    crit->prev = 0.0;
    return SP_OK;
}

int sp_critter_compute(sp_data *sp, sp_critter *crit, SPFLOAT *in, SPFLOAT *out)
{
    /*
    _clk get 0.01 0.1 0.1 tenv 
    _clk get 10 30 trand metro 0.001 0.001 0.001 tenvx 
    _clk get 800 3000 trand
    0.4 1 1 1 fm * *

    dup 
    0.3 0.9 1 rline
    0.001 0.3 10 rline
    1 vdelay + 
    */
    SPFLOAT tenv;
    SPFLOAT rand1;
    SPFLOAT rand2;
    SPFLOAT metro;
    SPFLOAT tenvx;
    SPFLOAT fm;
    SPFLOAT rline1;
    SPFLOAT rline2;
    SPFLOAT delay_in;
    SPFLOAT delay_out;

    sp_tenv_compute(sp, crit->tenv, in, &tenv);
    sp_trand_compute(sp, crit->trand1, in, &rand1);
    crit->metro->freq = rand1;
    sp_metro_compute(sp, crit->metro, NULL, &metro);
    sp_tenvx_compute(sp, crit->tenvx, &metro, &tenvx);

    sp_trand_compute(sp, crit->trand2, in, &rand2);
    crit->fm->freq = rand2;
    sp_osc_compute(sp, crit->fm, NULL, &fm);
    fm = fm * tenvx * tenv;

    /* for now the feedback rline is disabled for less glitches on huawei */
    //sp_rline_compute(sp, crit->rline1, NULL, &rline1);
    rline1 = 0.6;
    sp_rline_compute(sp, crit->rline2, NULL, &rline2);

    delay_in = fm + crit->prev * rline1;

    crit->vdelay->del = rline2;

    sp_vardelay_compute(sp, crit->vdelay, &delay_in, &delay_out);

    crit->prev = delay_out;

    *out = delay_out * 0.3 + fm;
    //*out = fm;
    return SP_OK;
}
