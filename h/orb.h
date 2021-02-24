#ifndef ORB_H 
#define ORB_H

#include <sys/time.h>

#include "fsm.h"

#ifdef BUILD_ANDROID
#include <android/log.h>
#define  LOG_TAG    "orb"
/* #define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__) */
#define  LOGI(...)
#else
#include <stdio.h>
#define LOGI(...) printf(__VA_ARGS__);
#endif

#ifndef SP_SYNTH_STRUCT
#define SP_SYNTH_STRUCT
typedef struct sp_synth sp_synth;
#endif

#define CSTACK_MAX 8
#define OBJECTS_MAX 16
/* 9 x 16 */
#define GRID_SIZE 144
#define GRID_WIDTH 16 
#define GRID_HEIGHT 9 

#define NLEVELS 9

typedef struct orb_data orb_data;

typedef void (* orb_level)(orb_data *);

enum {
ORB_SQUARE = 0,
ORB_OFFSQUARE,
ORB_AVOIDSQUARE,
ORB_MODE_EMPTY = 0,
ORB_MODE_PLAY,
ORB_MODE_FADE,
ORB_MODE_GAMEOVER
};

typedef struct {
    double x_pos;
    double y_pos;
} orb_mouse;

typedef struct {
    double x_pos;
    double y_pos;
    double radius;
    /* the initial radius */
    double ir;
    /* the collision radius */
    double cr;

    double phs;
    double env;

    /* oxygen */
    double oxygen;
    double deflation;
    double inflation;
    double recovery;

    int current_square;
} orb_avatar;

typedef struct {
    float vel_x;
    float vel_y;
    float acc;
} orb_motion;

typedef struct {
    double x;
    double y;
    double alpha;
    double radius;
} orb_circle;

typedef struct {
    int ncirc;
    int pos;
    double irad;
    double growth;
    double decay;
    orb_circle circ[CSTACK_MAX];
} orb_cstack;

typedef struct {
    int type;
    double x;
    double y;
    int next;
    int id;
    uint32_t fsm_pos;
    int note;
} orb_object;

typedef void (orb_draw) (NVGcontext *vg, orb_data *, orb_object *);

typedef struct {
    orb_object obj[OBJECTS_MAX];
    orb_draw *draw[3];
    int start;
    int nobjects;
    int nextfree;
    signed char map[GRID_SIZE];
} orb_object_list;

struct orb_data {
    double x_pos;
    double y_pos;
    int width; 
    int height;
    sp_synth *synth;
    NVGcolor old_color[2];
    NVGcolor new_color[2];
    NVGcolor color[2];
    orb_mouse mouse;
    orb_avatar av;
    double grid_size;
    double bias;
    orb_motion motion;
    orb_cstack cstack;
    struct timeval tv;
    double dtime;
    orb_object_list list;
    int id[OBJECTS_MAX];
    fsm_data fs;
    int mode;
    double wait;
    double alpha;
    double fade;

    orb_level level[NLEVELS];
    int lvl;
};


void orb_step(NVGcontext *vg, orb_data *orb);
void orb_audio(orb_data *orb, float **buf, int nframes);
void orb_audio_computei(orb_data *orb, short *buf, int nframes);
void orb_audio_create(orb_data *orb, int sr);
void orb_audio_destroy(orb_data *orb);
void orb_set_vals(orb_data *orb);
void orb_synth_set_vals(orb_data *orb);
void orb_init(orb_data *orb, int sr);
void orb_destroy(orb_data *orb);
void orb_poke(orb_data *orb);
void orb_resize(orb_data *orb);

/* orb avatar */

void orb_avatar_init(orb_data *orb, orb_avatar *av);
void orb_avatar_step(NVGcontext *vg, orb_data *orb, orb_avatar *av);
orb_data * orb_data_create();

void orb_avatar_poke(
    orb_data *orb,
    orb_avatar *av,
    orb_motion *m,
    double mx,
    double my,
    double ax,
    double ay);
int orb_avatar_find(orb_data *orb, orb_avatar *av, int *x, int *y);
void orb_avatar_collisions(orb_data *orb, 
        orb_object_list *list, 
        orb_avatar *av);
int orb_avatar_check_collision(orb_data *orb, 
    orb_object_list *list, 
    orb_avatar *av,
    int pos);

void orb_avatar_set_pos(orb_data *orb, orb_avatar *av, int x, int y);

void orb_avatar_center_x(orb_data *orb, orb_avatar *av);
void orb_avatar_center_y(orb_data *orb, orb_avatar *av);

void orb_avatar_compute_oxygen(orb_data *orb, orb_avatar *av);

/* synth */

void orb_synth_destroy(orb_data *orb);
void orb_synth_create(void *ud, orb_data *orb);
void orb_synth_collide(orb_data *orb, orb_object *obj);
void orb_synth_set_notes(orb_data *orb, int n1, int n2, int n3);
void orb_synth_set_topnote(orb_data *orb, int top);

/* grid */

void orb_grid_calculate(orb_data *orb);
double orb_grid_size(orb_data *orb);
double orb_grid_x(orb_data *orb, double n);
double orb_grid_y(orb_data *orb, double n);
int orb_grid_pos(orb_data *orb, int x, int y);
void orb_grid_bounds_detection(orb_data *orb, 
        int x, int y,
        int *t, int *b, int *l, int *r);

/* motion */

void orb_motion_init(orb_data *orb, orb_motion *m);
void orb_motion_step(orb_data *orb, 
    orb_motion *m,
    double *x,
    double *y);
void orb_motion_jump(orb_data *orb, 
    orb_motion *m,
    double *x,
    double *y,
    double amt);
void orb_motion_add_force(orb_data *orb, orb_motion *m, double vx, double vy);
void orb_motion_set_acceleration(orb_data *orb, orb_motion *m, double acc);
void orb_motion_bounce_edges(orb_data *orb, orb_motion *m, 
    double *x, double *y, double r);
void orb_motion_repel(orb_data *orb, orb_motion *m, double amt);
double orb_dtime(orb_data *orb);
void orb_motion_stop(orb_data *orb, orb_motion *m);

/* circle stack */

void orb_cstack_init(orb_data *orb, orb_cstack *stack);
void orb_cstack_add(orb_data *orb, orb_cstack *stack, double x, double y);
void orb_cstack_display(NVGcontext *vg, orb_data *orb, orb_cstack *stack);

/* objects */

void orb_object_set(orb_data *orb, orb_object *obj, int x, int y, int type);
void orb_object_draw(NVGcontext *vg, orb_data *orb, orb_object *obj);
void orb_object_set_type(orb_data *orb, int id, int type);
void orb_object_set_fsm_pos(orb_data *orb, int id, uint32_t pos);
void orb_object_set_note(orb_data *orb, int id, int note);

void orb_object_list_init(orb_data *orb, orb_object_list *list);
void orb_object_list_draw(NVGcontext *vg, orb_data *orb, orb_object_list *list);

int orb_object_new(orb_data *orb, orb_object_list *list, orb_object **obj);
int orb_object_add_square(orb_data *orb, orb_object_list *list, int x, int y);
int orb_object_add_avoidsquare(orb_data *orb, orb_object_list *list, int x, int y);
int orb_object_add_offsquare(orb_data *orb, orb_object_list *list, int x, int y);
void orb_object_list_map(orb_data *orb, orb_object_list *list, orb_object *obj);
int orb_object_list_get(orb_data *orb, orb_object_list *list, 
    orb_object **obj, int pos);

void orb_collide(orb_data *orb, 
    orb_object_list *list, orb_avatar *av, 
    orb_object *obj,
    int id);


/* Finite State Machine */

void orb_fsm_update(orb_data *orb);


/* levels */
void orb_level_init(orb_data *orb);
void orb_level_next(orb_data *orb);
void orb_level_load(orb_data *orb);
void orb_level_set(orb_data *orb, int level);

/* color */
void orb_color_set(orb_data *orb, NVGcolor color1, NVGcolor color2);
void orb_color_old(orb_data *orb, NVGcolor color1, NVGcolor color2);
void orb_color_new(orb_data *orb, NVGcolor color1, NVGcolor color2);
void orb_color_lerp(orb_data *orb, double alpha);
void orb_color_blood(orb_data *orb);
#endif
