#include "nanovg.h"
#include "orb.h"

void orb_fsm_update(orb_data *orb)
{
    fsm_data *fs = &orb->fs;
    uint32_t i;
    int id;

    for(i = 1; i <= fs->N; i++) {
        id = fsm_get_id(fs, i);
        if(fsm_get_bit(fs, i - 1)) {
            orb_object_set_type(orb, id, ORB_SQUARE);
        } else {
            orb_object_set_type(orb, id, ORB_OFFSQUARE);
        }
    }
}
