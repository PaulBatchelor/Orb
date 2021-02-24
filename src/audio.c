#include <stdlib.h>
#include <time.h>
#include "nanovg.h"
#include "orb.h"
#include "soundpipe.h"

#ifndef MAX
#define MAX(A, B) (A > B ? A : B)
#endif

#ifndef MIN
#define MIN(A, B) (A < B ? A : B)
#endif

#ifndef CLAMP
#define CLAMP(N, A, B) (MIN(MAX(N, A), B))
#endif

static sp_data *sp = NULL;

int sp_synth_computef(sp_data *sp, sp_synth *synth, int bufsize, SPFLOAT *buf);
int sp_synth_computei(sp_data *sp, sp_synth *synth, int bufsize, short *buf);
int sp_synth_init(sp_data *sp, sp_synth *synth);

void orb_audio(orb_data *orb, float **buf, int nframes)
{
    int i;
    sp_synth_computef(sp, orb->synth, nframes, buf[0]);
    for(i = 0; i < nframes; i++) {
        buf[1][i] = buf[0][i];
    }
}

void orb_audio_computei(orb_data *orb, short *buf, int nframes)
{
    sp_synth_computei(sp, orb->synth, nframes, buf);
}

void orb_audio_create(orb_data *orb, int sr)
{
    sp_create(&sp);
    sp->sr = sr;
    sp_srand(sp, time(NULL));
    orb_synth_create(sp, orb);
}

void orb_audio_destroy(orb_data *orb)
{
    orb_synth_destroy(orb);
}
