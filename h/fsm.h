#ifndef FSM_H 
#define FSM_H
#include <stdint.h>

typedef struct {
    uint32_t state;
    uint32_t N;
    uint32_t max;
    uint32_t max_depth;
    uint32_t *rules;
    int *minpaths;
    char *matrix;
    int *obj;
} fsm_data; 

void fsm_create(fsm_data *fs, uint32_t max);
void fsm_destroy(fsm_data *fs);
void fsm_init(fsm_data *fs, uint32_t N);
void fsm_add_rule(fsm_data *fs, uint32_t pos, uint32_t target);
void fsm_set_state_pos(fsm_data *fs, uint32_t pos, uint32_t state);
void fsm_set_state(fsm_data *fs, uint32_t state);
uint32_t fsm_get_state(fsm_data *fs);
void fsm_compute_state(fsm_data *fs, uint32_t pos);
void fsm_compute_matrix(fsm_data *fs);
void fsm_compute_minpaths(fsm_data *fs);
void fsm_print_matrix(fsm_data *fs,
    void (*header)(fsm_data *, uint32_t),
    void (*state)(fsm_data *, unsigned char, uint32_t, uint32_t, uint32_t),
    void (*newline)(fsm_data *),
    void (*footer)(fsm_data *));

void fsm_print_minpaths_text(fsm_data *fs);
void fsm_print_minpaths_latex(fsm_data *fs);
void fsm_print_minpaths(fsm_data *fs, 
        void (*header)(fsm_data*),
        void (*text)(fsm_data*, uint32_t, int),
        void (*footer)(fsm_data*));

void fsm_print_state(fsm_data *fs);
void fsm_toggle(fsm_data *fs, uint32_t pos);
unsigned char fsm_get_bit(fsm_data *fs, uint32_t pos);

void fsm_print_img(fsm_data *fs);
void fsm_print_text(fsm_data *fs);
void fsm_print_dot(fsm_data *fs);

int fsm_find_min(fsm_data *fs, uint32_t s1, uint32_t s2, uint32_t count, int min);

void fsm_assign_id(fsm_data *fs, uint32_t pos, int id);
int fsm_get_id(fsm_data *fs, uint32_t pos);
#endif
