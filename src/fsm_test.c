#include <stdio.h>
#include "fsm.h"

static void rule3(fsm_data *fs)
{
    fsm_add_rule(fs, 1, 2);
    fsm_add_rule(fs, 1, 3);
    fsm_add_rule(fs, 1, 4);

    fsm_add_rule(fs, 2, 3);
    fsm_add_rule(fs, 2, 1);
    fsm_add_rule(fs, 3, 4);
    fsm_add_rule(fs, 4, 1);
}

static void rule2b(fsm_data *fs)
{
    fsm_add_rule(fs, 1, 2);
    fsm_add_rule(fs, 2, 3);
    fsm_add_rule(fs, 3, 4);
    fsm_add_rule(fs, 4, 1);
}

static void rule2a(fsm_data *fs)
{
    fsm_add_rule(fs, 1, 2);
    fsm_add_rule(fs, 2, 3);
    fsm_add_rule(fs, 3, 4);
}

static void rule1(fsm_data *fs)
{
    fsm_add_rule(fs, 1, 2);
    fsm_add_rule(fs, 2, 3);
    fsm_add_rule(fs, 2, 4);
    fsm_add_rule(fs, 3, 1);
    fsm_add_rule(fs, 4, 1);
    fsm_add_rule(fs, 4, 3);
}

int main(int argc, char *argv[])
{
    fsm_data fsm;
    int min;
    uint32_t n2;
    uint32_t s;
    fsm_create(&fsm, 5);

    fsm_init(&fsm, 4);

    fsm_add_rule(&fsm, 1, 4);
    fsm_add_rule(&fsm, 2, 1);

    fsm_add_rule(&fsm, 3, 1);
    fsm_add_rule(&fsm, 3, 2);
    fsm_add_rule(&fsm, 3, 4);
    
    fsm_add_rule(&fsm, 4, 2);
    fsm_add_rule(&fsm, 4, 3);


    fsm_compute_matrix(&fsm);
    fsm_compute_minpaths(&fsm);

    //fsm_print_dot(&fsm);

    fsm_print_minpaths_text(&fsm);

    fsm_destroy(&fsm);
    return 0;
}
