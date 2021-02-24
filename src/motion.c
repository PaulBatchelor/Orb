#include <stdlib.h>
#include <math.h>
#include "nanovg.h"
#include "orb.h"

void orb_motion_init(orb_data *orb, orb_motion *m)
{
    m->vel_x = 0;
    m->vel_y = 0;
    m->acc = 0;
}

void orb_motion_step(orb_data *orb, 
    orb_motion *m,
    double *x,
    double *y)
{
    double dtime = orb->dtime;
    double dx = m->vel_x * dtime;
    double dy = m->vel_y * dtime;
    double a = pow(m->acc, dtime);
    
    *x += dx * orb_grid_size(orb);
    *y += dy * orb_grid_size(orb);

    m->vel_x *= a;
    m->vel_y *= a;

}

void orb_motion_jump(orb_data *orb, 
    orb_motion *m,
    double *x,
    double *y,
    double amt)
{
    double dtime = orb->dtime;
    double dx = m->vel_x * dtime;
    double dy = m->vel_y * dtime;
    *x += dx * orb_grid_size(orb) * amt;
    *y += dy * orb_grid_size(orb) * amt;
}


void orb_motion_add_force(orb_data *orb, orb_motion *m, double vx, double vy)
{
    m->vel_x += vx;
    m->vel_y += vy;
}

double orb_dtime(orb_data *orb)
{
    struct timeval newtime;
    double dtime = 0;

    gettimeofday(&newtime, NULL);
    
    dtime += (newtime.tv_sec - orb->tv.tv_sec);
    dtime += (double)(newtime.tv_usec - orb->tv.tv_usec) / 1000000.0;

    orb->tv = newtime;
    return dtime;
}

void orb_motion_set_acceleration(orb_data *orb, orb_motion *m, double acc)
{
    m->acc = acc;
}

void orb_motion_bounce_edges(orb_data *orb, orb_motion *m, 
        double *x, double *y, double r)
{
    if(*x + r > orb->width) {
        m->vel_x *= -1;
        *x = orb->width - r;
    } else if(*x - r < 0) {
        m->vel_x *= -1;
        *x = r;
    }

    if(*y + r > orb->height - orb->bias) {
        m->vel_y *= -1;
        *y = (orb->height - orb->bias) - r;
    }else if(*y - r < orb->bias) {
        m->vel_y *= -1;
        *y = orb->bias + r;
    }
}

void orb_motion_repel(orb_data *orb, orb_motion *m, double amt)
{
    m->vel_x *= -amt;
    m->vel_y *= -amt;
}

void orb_motion_stop(orb_data *orb, orb_motion *m)
{
    m->vel_x = 0;
    m->vel_y = 0;
}
