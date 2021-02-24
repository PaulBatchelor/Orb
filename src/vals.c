#include <stdlib.h>
#include <math.h>

#include "nanovg.h"
#include "orb.h"

#include "soundpipe.h"
#include "mode.h"
#include "modal.h"
#include "aux.h"
#include "revscm.h"
#include "critter.h"
#include "rspline.h"
#include "synth.h"

void orb_synth_create(void *ud, orb_data *orb)
{
    sp_data *sp = ud; 
    orb->synth = malloc(sizeof(sp_synth));
    sp_synth_create(sp, orb->synth);
    sp_synth_init(sp, orb->synth);
}

void orb_synth_destroy(orb_data *orb)
{
    orb->synth = malloc(sizeof(sp_synth));
}

void orb_synth_set_vals(orb_data *orb)
{
    sp_synth *synth = orb->synth;
    synth->gate = (synth->gate == 0) ? 1 : 0;
}

void orb_synth_collide(orb_data *orb, orb_object *obj)
{
    sp_synth *synth;
    sp_modal *m;
    orb_motion *motion;
    SPFLOAT amp;

    synth = orb->synth;
    m = synth->mode;

    motion = &orb->motion;

    amp = ((fabs(motion->vel_y) + fabs(motion->vel_x)) * 0.5) * 0.3;

    if(amp > 1.0) amp = 1.0; 

    m->amp = amp;
    //sp_modal_reset(m);
    LOGI("amp is %g\n", amp);

    switch(obj->type) {
        case ORB_SQUARE:
            sp_modal_type(m, 0);
            sp_modal_scale(m, amp);
            break;
        case ORB_OFFSQUARE:
            if(amp < 0.2) {
                amp = 0.2;
                m->amp = 0.2;
            }
            sp_modal_type(m, 1);
            sp_modal_scale(m, amp);
            break;
        case ORB_AVOIDSQUARE:
            sp_modal_type(m, 2);
            sp_modal_scale(m, amp * 0.4);
            break;
    }

    if(m->gate == 0) {
        m->gate = 1;
    } else {
        m->gate = 0;
    }

}

void orb_synth_set_notes(orb_data *orb, int n1, int n2, int n3)
{
    sp_synth *synth;
   
    synth = orb->synth;

    synth->freq1 = sp_midi2cps(n1);
    synth->freq2 = sp_midi2cps(n2);
    synth->freq3 = sp_midi2cps(n3);

}

void orb_synth_set_topnote(orb_data *orb, int top)
{
    sp_synth *synth;
   
    synth = orb->synth;
    synth->mode->metal[0] = sp_midi2cps(top) * 3.001;
    //synth->mode->metal[1] = sp_midi2cps(top) * 0.5;
    synth->mode->metal[2] = sp_midi2cps(top) * 1.998;
    //synth->mode->metal[3] = sp_midi2cps(top) * 0.5;
    
    synth->mode->metal[4] = sp_midi2cps(top);
    synth->mode->metal[6] = sp_midi2cps(top) * 0.5;
}
