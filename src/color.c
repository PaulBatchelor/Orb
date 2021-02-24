#include "nanovg.h"
#include "orb.h"

void orb_color_set(orb_data *orb, NVGcolor color1, NVGcolor color2)
{
    orb_color_old(orb, orb->new_color[0], orb->new_color[1]);
    orb_color_new(orb, color1, color2);
}

void orb_color_old(orb_data *orb, NVGcolor color1, NVGcolor color2)
{
    orb->old_color[0] = color1;
    orb->old_color[1] = color2;
}

void orb_color_new(orb_data *orb, NVGcolor color1, NVGcolor color2)
{
    orb->new_color[0] = color1;
    orb->new_color[1] = color2;
}

void orb_color_lerp(orb_data *orb, double alpha)
{
    NVGcolor *clr= orb->color;
    NVGcolor *nclr = orb->new_color;
    NVGcolor *oclr = orb->old_color;

    clr[0].r = nclr[0].r * alpha + oclr[0].r * (1 - alpha);
    clr[1].r = nclr[1].r * alpha + oclr[1].r * (1 - alpha);

    clr[0].g = nclr[0].g * alpha + oclr[0].g * (1 - alpha);
    clr[1].g = nclr[1].g * alpha + oclr[1].g * (1 - alpha);
    
    clr[0].b = nclr[0].b * alpha + oclr[0].b * (1 - alpha);
    clr[1].b = nclr[1].b * alpha + oclr[1].b * (1 - alpha);
}

void orb_color_blood(orb_data *orb)
{
    orb->color[0] = nvgRGB(251, 0, 0);
}
