#include <math.h>
#include "soundpipe.h"
#include "aux.h"
#include "mode.h"
#include "modal.h"
#include "revscm.h"
#include "critter.h"
#include "rspline.h"
#include "synth.h"

int sp_synth_create(sp_data *sp, sp_synth *synth)
{
    sp_modal_create(&synth->mode);
    sp_smoother_create(&synth->port2);
    sp_rspline_create(&synth->lfo3);
    sp_butlp_create(&synth->lpf);
    sp_randh_create(&synth->randh);
    sp_fmpair_create(&synth->voice2);
    sp_smoother_create(&synth->port3);
    sp_critter_create(&synth->critter);
    sp_rspline_create(&synth->lfo2);
    sp_fmpair_create(&synth->voice3);
    sp_rspline_create(&synth->lfo1);
    sp_thresh_create(&synth->thresh);
    sp_delay_create(&synth->delay);
    sp_ftbl_create(sp, &synth->sine, 2048);
    sp_revscm_create(&synth->rev);
    sp_fmpair_create(&synth->voice1);
    sp_smoother_create(&synth->port1);
    sp_butlp_create(&synth->noisefilt);
    sp_trand_create(&synth->tr);
    sp_tenv_create(&synth->env);
    return SP_OK;
}

int sp_synth_destroy(sp_data *sp, sp_synth *synth)
{
    sp_modal_destroy(&synth->mode);
    sp_smoother_destroy(&synth->port2);
    sp_rspline_destroy(&synth->lfo3);
    sp_butlp_destroy(&synth->lpf);
    sp_randh_destroy(&synth->randh);
    sp_fmpair_destroy(&synth->voice2);
    sp_smoother_destroy(&synth->port3);
    sp_critter_destroy(&synth->critter);
    sp_rspline_destroy(&synth->lfo2);
    sp_fmpair_destroy(&synth->voice3);
    sp_rspline_destroy(&synth->lfo1);
    sp_thresh_destroy(&synth->thresh);
    sp_delay_destroy(&synth->delay);
    sp_ftbl_destroy(&synth->sine);
    sp_revscm_destroy(&synth->rev);
    sp_fmpair_destroy(&synth->voice1);
    sp_smoother_destroy(&synth->port1);
    sp_butlp_destroy(&synth->noisefilt);
    sp_trand_destroy(&synth->tr);
    sp_tenv_destroy(&synth->env);
    return SP_OK;
}

static void sp_smoother_init_old(sp_data *sp,
                             sp_smoother *p,
                             SPFLOAT val)
{
    sp_smoother_init(sp, p);
    p->smooth = val;
}

int sp_synth_init(sp_data *sp, sp_synth *synth)
{
    synth->sp = sp;
    synth->freq2 = 880;
    sp_modal_init(sp, synth->mode);
    sp_smoother_init_old(sp, synth->port2, 0.08);
    synth->freq3 = 880;
    synth->freq1 = 880;
    sp_gen_sine(sp, synth->sine);
    sp_rspline_init(sp, synth->lfo3);
    synth->lfo3->min = 0;
    synth->lfo3->max = 1;
    synth->lfo3->cps_min = 0.5;
    synth->lfo3->cps_max = 3;
    sp_butlp_init(sp, synth->lpf);
    synth->lpf->freq = 4000;
    synth->gate = 880;
    sp_randh_init(sp, synth->randh);
    synth->randh->min = -0.02;
    synth->randh->max = 0.02;
    synth->randh->freq = 1000;
    sp_fmpair_init(sp, synth->voice2, synth->sine);
    synth->voice2->freq = 440;
    synth->voice2->amp = 0.03;
    synth->voice2->car = 1;
    synth->voice2->mod = 1.001;
    synth->voice2->indx = 0.7;
    sp_smoother_init_old(sp, synth->port3, 0.08);
    sp_critter_init(sp, synth->critter, synth->sine);
    sp_rspline_init(sp, synth->lfo2);
    synth->lfo2->min = 0;
    synth->lfo2->max = 1;
    synth->lfo2->cps_min = 3;
    synth->lfo2->cps_max = 10;
    sp_fmpair_init(sp, synth->voice3, synth->sine);
    synth->voice3->freq = 440;
    synth->voice3->amp = 0.03;
    synth->voice3->car = 1;
    synth->voice3->mod = 0.999;
    synth->voice3->indx = 0.8;
    sp_rspline_init(sp, synth->lfo1);
    synth->lfo1->min = 0;
    synth->lfo1->max = 1;
    synth->lfo1->cps_min = 0.1;
    synth->lfo1->cps_max = 1;
    sp_thresh_init(sp, synth->thresh);
    synth->thresh->thresh = 0.5;
    synth->thresh->mode = 2;
    sp_delay_init(sp, synth->delay, 2.0);
    synth->delay->feedback = 0.9;
    sp_revscm_init(sp, synth->rev);
    synth->rev->feedback = 0.98;
    synth->rev->lpfreq = 8000;
    sp_fmpair_init(sp, synth->voice1, synth->sine);
    synth->voice1->freq = 440;
    synth->voice1->amp = 0.03;
    synth->voice1->car = 1;
    synth->voice1->mod = 1;
    synth->voice1->indx = 0.8;
    sp_smoother_init_old(sp, synth->port1, 0.06);
    sp_butlp_init(sp, synth->noisefilt);
    synth->noisefilt->freq = 800;
    sp_trand_init(sp, synth->tr);
    synth->tr->min = 3000;
    synth->tr->max = 8000;
    sp_tenv_init(sp, synth->env);
    synth->env->atk = 0.1;
    synth->env->hold = 0;
    synth->env->rel = 0.9;
    return SP_OK;
}

int sp_synth_computef(sp_data *sp, sp_synth *synth, int bufsize, SPFLOAT *buf)
{
    SPFLOAT tmp[10];
    int i;
    for(i = 0; i < bufsize ; i++) {
        sp_thresh_compute(sp, synth->thresh, &synth->gate, &tmp[1]);
        sp_trand_compute(sp, synth->tr, &tmp[1], &tmp[5]);
        synth->randh->freq = tmp[5];
        sp_randh_compute(sp, synth->randh, NULL, &tmp[2]);
        sp_butlp_compute(sp, synth->noisefilt, &tmp[2], &tmp[3]);
        sp_tenv_compute(sp, synth->env, &tmp[1], &tmp[0]);
        tmp[5] = tmp[0] * tmp[3];
        tmp[7] = tmp[5];
        tmp[7] *= 0.6;
        sp_critter_compute(sp, synth->critter, &tmp[1], &tmp[2]);
        tmp[5] = tmp[5] + tmp[2];
        tmp[0] = tmp[5];
        tmp[0] *= 0.1;
        tmp[7] = tmp[7] + tmp[0];
        sp_modal_compute(sp, synth->mode, NULL, &tmp[4]);
        tmp[2] *= 0.21;
        tmp[2] = tmp[2] + tmp[4];
        tmp[2] *= 0.3;
        sp_delay_compute(sp, synth->delay, &tmp[2], &tmp[0]);
        tmp[7] = tmp[7] + tmp[0];
        tmp[0] *= 0.5;
        tmp[5] = tmp[5] + tmp[0];
        tmp[5] = tmp[5] + tmp[2];
        tmp[0] = tmp[2];
        tmp[0] *= 0.9;
        tmp[7] = tmp[7] + tmp[0];
        tmp[5] = tmp[5] + tmp[4];
        sp_smoother_compute(sp, synth->port1, &synth->freq1, &tmp[0]);
        synth->voice1->freq = tmp[0];
        sp_fmpair_compute(sp, synth->voice1, NULL, &tmp[4]);
        sp_rspline_compute(sp, synth->lfo1, NULL, &tmp[0]);
        tmp[0] = 0+ (0.5 * (tmp[0] + 1)) * 1;
        tmp[4] = tmp[4] * tmp[0];
        sp_smoother_compute(sp, synth->port2, &synth->freq2, &tmp[0]);
        synth->voice2->freq = tmp[0];
        sp_rspline_compute(sp, synth->lfo2, NULL, &tmp[0]);
        tmp[6] = tmp[0];
        tmp[6] = 0.1+ (0.5 * (tmp[6] + 1)) * 2.9;
        synth->voice2->indx = tmp[6];
        sp_fmpair_compute(sp, synth->voice2, NULL, &tmp[1]);
        tmp[1] = tmp[1] * tmp[0];
        tmp[4] = tmp[4] + tmp[1];
        sp_smoother_compute(sp, synth->port3, &synth->freq3, &tmp[0]);
        synth->voice3->freq = tmp[0];
        sp_fmpair_compute(sp, synth->voice3, NULL, &tmp[1]);
        sp_rspline_compute(sp, synth->lfo3, NULL, &tmp[0]);
        tmp[0] = 0+ (0.5 * (tmp[0] + 1)) * 1;
        tmp[1] = tmp[1] * tmp[0];
        tmp[4] = tmp[4] + tmp[1];
        tmp[4] *= 0.3;
        tmp[1] = tmp[4];
        tmp[1] *= 0.9;
        tmp[7] = tmp[7] + tmp[1];
        tmp[5] = tmp[5] + tmp[4];
        sp_revscm_compute(sp, synth->rev, &tmp[7], &tmp[4]);
        tmp[4] *= 0.4;
        tmp[5] = tmp[5] + tmp[4];
        buf[i] = tmp[5];
    }
    return SP_OK;
}

int sp_synth_computei(sp_data *sp, sp_synth *synth, int bufsize, short *buf)
{
    SPFLOAT tmp[10];
    int i;
    for(i = 0; i < bufsize ; i++) {
        sp_thresh_compute(sp, synth->thresh, &synth->gate, &tmp[1]);
        sp_trand_compute(sp, synth->tr, &tmp[1], &tmp[5]);
        synth->randh->freq = tmp[5];
        sp_randh_compute(sp, synth->randh, NULL, &tmp[2]);
        sp_butlp_compute(sp, synth->noisefilt, &tmp[2], &tmp[3]);
        sp_tenv_compute(sp, synth->env, &tmp[1], &tmp[0]);
        tmp[5] = tmp[0] * tmp[3];
        tmp[7] = tmp[5];
        tmp[7] *= 0.6;
        sp_critter_compute(sp, synth->critter, &tmp[1], &tmp[2]);
        tmp[5] = tmp[5] + tmp[2];
        tmp[0] = tmp[5];
        tmp[0] *= 0.1;
        tmp[7] = tmp[7] + tmp[0];
        sp_modal_compute(sp, synth->mode, NULL, &tmp[4]);
        tmp[2] *= 0.21;
        tmp[2] = tmp[2] + tmp[4];
        tmp[2] *= 0.3;
        sp_delay_compute(sp, synth->delay, &tmp[2], &tmp[0]);
        tmp[7] = tmp[7] + tmp[0];
        tmp[0] *= 0.5;
        tmp[5] = tmp[5] + tmp[0];
        tmp[5] = tmp[5] + tmp[2];
        tmp[0] = tmp[2];
        tmp[0] *= 0.9;
        tmp[7] = tmp[7] + tmp[0];
        tmp[5] = tmp[5] + tmp[4];
        sp_smoother_compute(sp, synth->port1, &synth->freq1, &tmp[0]);
        synth->voice1->freq = tmp[0];
        sp_fmpair_compute(sp, synth->voice1, NULL, &tmp[4]);
        sp_rspline_compute(sp, synth->lfo1, NULL, &tmp[0]);
        tmp[0] = 0+ (0.5 * (tmp[0] + 1)) * 1;
        tmp[4] = tmp[4] * tmp[0];
        sp_smoother_compute(sp, synth->port2, &synth->freq2, &tmp[0]);
        synth->voice2->freq = tmp[0];
        sp_rspline_compute(sp, synth->lfo2, NULL, &tmp[0]);
        tmp[6] = tmp[0];
        tmp[6] = 0.1+ (0.5 * (tmp[6] + 1)) * 2.9;
        synth->voice2->indx = tmp[6];
        sp_fmpair_compute(sp, synth->voice2, NULL, &tmp[1]);
        tmp[1] = tmp[1] * tmp[0];
        tmp[4] = tmp[4] + tmp[1];
        sp_smoother_compute(sp, synth->port3, &synth->freq3, &tmp[0]);
        synth->voice3->freq = tmp[0];
        sp_fmpair_compute(sp, synth->voice3, NULL, &tmp[1]);
        sp_rspline_compute(sp, synth->lfo3, NULL, &tmp[0]);
        tmp[0] = 0+ (0.5 * (tmp[0] + 1)) * 1;
        tmp[1] = tmp[1] * tmp[0];
        tmp[4] = tmp[4] + tmp[1];
        tmp[4] *= 0.3;
        tmp[1] = tmp[4];
        tmp[1] *= 0.9;
        tmp[7] = tmp[7] + tmp[1];
        tmp[5] = tmp[5] + tmp[4];
        sp_revscm_compute(sp, synth->rev, &tmp[7], &tmp[4]);
        tmp[4] *= 0.4;
        tmp[5] = tmp[5] + tmp[4];
        buf[i] = tmp[5] * 32767;
    }
    return SP_OK;
}
