#ifndef SP_SYNTH_TYPEDEF
#define SP_SYNTH_TYPEDEF
typedef struct sp_synth sp_synth;
#endif

struct sp_synth {
    sp_data *sp;
    SPFLOAT freq2;
    sp_modal *mode;
    sp_smoother *port2;
    SPFLOAT freq3;
    SPFLOAT freq1;
    sp_rspline *lfo3;
    sp_butlp *lpf;
    SPFLOAT gate;
    sp_randh *randh;
    sp_fmpair *voice2;
    sp_smoother *port3;
    sp_critter *critter;
    sp_rspline *lfo2;
    sp_fmpair *voice3;
    sp_rspline *lfo1;
    sp_thresh *thresh;
    sp_delay *delay;
    sp_ftbl *sine;
    sp_revscm *rev;
    sp_fmpair *voice1;
    sp_smoother *port1;
    sp_butlp *noisefilt;
    sp_trand *tr;
    sp_tenv *env;
};

int sp_synth_computef(sp_data *sp, sp_synth *synth, int bufsize, SPFLOAT *buf);
int sp_synth_create(sp_data *sp, sp_synth *synth);
int sp_synth_destroy(sp_data *sp, sp_synth *synth);
int sp_synth_init(sp_data *sp, sp_synth *synth);
