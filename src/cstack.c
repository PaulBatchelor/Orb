#include <math.h>
#include "nanovg.h"
#include "orb.h"

void orb_cstack_init(orb_data *orb, orb_cstack *stack)
{
    stack->ncirc = 0;
    stack->pos = 0;
    stack->growth = 3.0;
    stack->decay = 0.01;
    stack->irad = (double)orb_grid_size(orb);
}

void orb_cstack_add(orb_data *orb, orb_cstack *stack, double x, double y)
{
    int id;
    orb_circle *circ;
    if(stack->ncirc >= CSTACK_MAX) return;

    stack->ncirc++;
    id = (stack->pos + (stack->ncirc - 1)) % CSTACK_MAX;
    circ = &stack->circ[id];

    circ->x = x;
    circ->y = y;
    circ->alpha = 1;
    circ->radius = stack->irad;

}

static void draw_circle(NVGcontext *vg, orb_data *orb, orb_cstack *stack, orb_circle *circ)
{
    if(circ->alpha < 0.001) {
       stack->pos = (stack->pos + 1) % CSTACK_MAX; 
       stack->ncirc--;
       return;
    }

    nvgBeginPath(vg);
    nvgArc(vg, circ->x, circ->y, circ->radius, 0, 2 * M_PI, NVG_CCW);
    nvgClosePath(vg);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255 * circ->alpha));
    nvgFill(vg);

    circ->radius *= pow(stack->growth, orb->dtime);
    circ->alpha *= pow(stack->decay, orb->dtime);
}

void orb_cstack_display(NVGcontext *vg, orb_data *orb, orb_cstack *stack)
{
    int i;
    int id;
    int pos;
    int ncirc;

    pos = stack->pos;
    ncirc = stack->ncirc;
    for(i = 0; i < ncirc; i++) {
        id = (pos + i) % CSTACK_MAX;
        draw_circle(vg, orb, stack, &stack->circ[id]);
    }
}

