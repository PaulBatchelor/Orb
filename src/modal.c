#include <stdlib.h>
#include <math.h>
#include "nanovg.h"
#include "orb.h"
#include "soundpipe.h"
#include "mode.h"
#include "modal.h"

int sp_modal_create(sp_modal **m)
{
    int i;
    *m = malloc(sizeof(sp_modal));
    for(i = 0; i < 4; i++) {
        sp_mode_create(&(*m)->mode[i]);
    }
    sp_dcblocker_create(&(*m)->dc);
    return SP_OK;
}

int sp_modal_destroy(sp_modal **m)
{
    int i;
    for(i = 0; i < 4; i++) {
        sp_mode_destroy(&(*m)->mode[i]);
    }
    sp_dcblocker_destroy(&(*m)->dc);
    free(*m);
    return SP_OK;
}

int sp_modal_init(sp_data *sp, sp_modal *m)
{
    int i;
    m->type = 1;
    m->ptype = 1;
    m->mat[0] = m->metal;
    m->mat[1] = m->plastic;
    m->mat[2] = m->avoid;

    m->metal[0] = 1000;
    m->metal[1] = 400;
    m->metal[2] = 1800;
    m->metal[3] = 1400;
    m->metal[4] = 440;
    m->metal[5] = 60;
    m->metal[6] = 880;
    m->metal[7] = 600;
    
    m->plastic[0] = 700;
    m->plastic[1] = 10;
    m->plastic[2] = 1800;
    m->plastic[3] = 2;
    m->plastic[4] = 440;
    m->plastic[5] = 60;
    m->plastic[6] = 630;
    m->plastic[7] = 30;
    
    m->avoid[0] = 800;
    m->avoid[1] = 40;
    m->avoid[2] = 1800;
    m->avoid[3] = 1400;
    m->avoid[4] = 300;
    m->avoid[5] = 100;
    m->avoid[6] = 880;
    m->avoid[7] = 100;
    

    for(i = 0; i < 4; i++) {
        sp_mode_init(sp, m->mode[i]);
        m->mode[i]->freq = m->plastic[2 * i];
        m->mode[i]->q = m->plastic[2 * i + 1];
    }
    m->gate = 0;
    m->prev = 0;
    m->amp = 0;

    sp_dcblocker_init(sp, m->dc);

    return SP_OK;
}

int sp_modal_compute(sp_data *sp, sp_modal *m, SPFLOAT *in, SPFLOAT *out)
{
    SPFLOAT trig;
    SPFLOAT tmp;
    SPFLOAT exc;
    SPFLOAT col;

    if(m->gate != m->prev) {
        if(m->type == 0) {
            if(m->amp > 0.6) {
                trig = m->amp;
            } else {
                trig = 0.6;
            }
        } else {
            trig = m->amp;
        }
    } else {
        trig = 0.0;
    }

    m->prev = m->gate;

    exc = 0;
    sp_mode_compute(sp, m->mode[0], &trig, &tmp);
    exc = tmp;
    sp_mode_compute(sp, m->mode[1], &trig, &tmp);
    exc += tmp;
    exc *= 0.10;

    col = 0;
    sp_mode_compute(sp, m->mode[2], &exc, &tmp);
    col = tmp;
    sp_mode_compute(sp, m->mode[3], &exc, &tmp);
    col += tmp;

    col *= 0.5;

    *out = col;
    //sp_dcblocker_compute(sp, m->dc, &col, out);
    return SP_OK;
}

void sp_modal_scale(sp_modal *m, SPFLOAT amp)
{
    //amp = (amp > 0.8 ? 0.8 : amp);
    SPFLOAT *mat = m->mat[m->type];
    m->mode[0]->q = 2 + mat[1] * amp;
    m->mode[1]->q = 8 + mat[3] * amp;
    m->mode[2]->q = 30 + mat[5] * amp;
    m->mode[3]->q = 13 + mat[7] * amp;
}

void sp_modal_type(sp_modal *m, int type)
{
    SPFLOAT *mat;
    int i;
    
    mat = m->mat[type];
    m->type = type;

    /* might not be needed anymore since collide is calling reset */
    if(m->type != m->ptype) {
        //sp_modal_reset(m);
    }

    m->ptype = m->type;

    for(i = 0; i < 4; i++) {
        m->mode[i]->freq = mat[2 * i];
        m->mode[i]->q = mat[(2 * i) + 1];
    }
}

void sp_modal_reset(sp_modal *m)
{
    int i;
    sp_mode *p;
    for(i = 0; i < 4; i++) {
        p = m->mode[i];
        p->xnm1 = p->ynm1 = p->ynm2 = 0.0;
        p->a0 = p->a1 = p->a2 = p->d = 0.0;
    }
}
