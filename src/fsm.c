#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fsm.h"

void fsm_create(fsm_data *fs, uint32_t max)
{
    size_t rules_size = sizeof(uint32_t) * max * max;
    size_t matrix_size = sizeof(uint32_t) * (1 << max) * (1 << max);
    size_t minpaths_size = sizeof(int) * (1 << max);
    size_t obj_size = sizeof(int) * max;

    fs->rules = malloc(rules_size);
    memset(fs->rules, 0, rules_size);

    fs->matrix = malloc(matrix_size);
    memset(fs->matrix, 0, matrix_size);

    fs->minpaths = malloc(minpaths_size);
    memset(fs->minpaths, 0, minpaths_size);
    
    fs->obj = malloc(obj_size);
    memset(fs->rules, 0, obj_size);

    fs->max = max;
}

void fsm_destroy(fsm_data *fs)
{
    free(fs->rules);
    free(fs->matrix);
    free(fs->minpaths);
    free(fs->obj);
}

void fsm_init(fsm_data *fs, uint32_t N)
{
    uint32_t i;
    fs->N = N;

    /* zero out current state */
    fs->state = 0;
    for(i = 0; i < N; i++) {
        /* set rules to 0 */
        fs->rules[i * N] = 0;
    }
    fs->max_depth = 10;
}

void fsm_add_rule(fsm_data *fs, uint32_t pos, uint32_t target)
{
    uint32_t r;
    uint32_t off;
    
    r = (pos - 1) * fs->N;
    off = r + fs->rules[r] + 1;

    fs->rules[off] = target;
    fs->rules[r]++;

}

void fsm_set_state_pos(fsm_data *fs, uint32_t pos, uint32_t state)
{
    fs->state = 1 << (pos - 1);
}

void fsm_set_state(fsm_data *fs, uint32_t state)
{
    fs->state = state;
}

uint32_t fsm_get_state(fsm_data *fs)
{
    return fs->state;
}

void fsm_compute_state(fsm_data *fs, uint32_t pos)
{
    uint32_t r;
    uint32_t nrules;
    uint32_t off;

    if(fsm_get_bit(fs, pos- 1)) {
        off = (pos - 1) * fs->N;
        nrules = fs->rules[off];
        fsm_toggle(fs, pos);
        for(r = 1; r <= nrules; r++) {
            fsm_toggle(fs, fs->rules[off + r]);
        }
    }
}

void fsm_compute_matrix(fsm_data *fs)
{
    int state;
    uint32_t i;
    uint32_t off;
    uint32_t s;
    uint32_t n;

    n = 1 << fs->N;
    s = 2;


    for(s = 0; s < n; s++) {
        off = n * s;
        state = s;
        for(i = 1; i <= fs->N; i++) {
            fs->state = state;
            fsm_compute_state(fs, i);
            if(fs->state != state) {
                fs->matrix[fs->state + off] = 1;
            }
        }
    }
}

static void img_print_header(fsm_data *fs, uint32_t n)
{
    printf(
        "\"img\" load\n"
        "255 255 255 img_color img_fill\n"
        "0 0 0 img_color\n"
        "[\n");
    printf(
        ": xy %d img_xy 10 * %d + \n"
        "swap 10 * %d + swap ;\n", n, 5, 5);
    printf(": odot xy 5 img_ocirc ;\n"
    ": dot xy 5 img_circ ;\n"
    "]\n"
    );

    printf("%d %d img_setsize\n", n * 10 + 1, n * 10 + 1);
}

static void img_print_state(fsm_data *fs, 
    unsigned char state, 
    uint32_t s,
    uint32_t n,
    uint32_t off)
{
    if(state) {
        printf("%d dot\n", off);
    } else {
        printf("%d odot\n", off);
    }
}

static void img_new_line(fsm_data *fs)
{
    printf("\n");
}

static void img_footer(fsm_data *fs)
{
    printf("\"dots.png\" img_write\n");
}

void fsm_print_img(fsm_data *fs)
{
    fsm_print_matrix(fs,
        img_print_header,
        img_print_state,
        img_new_line,
        img_footer);
}


void fsm_print_matrix(fsm_data *fs,
    void (*header)(fsm_data *, uint32_t),
    void (*state)(fsm_data *, unsigned char, uint32_t, uint32_t, uint32_t),
    void (*newline)(fsm_data *),
    void (*footer)(fsm_data *))
{
    uint32_t i;
    uint32_t n;
    uint32_t off;
    uint32_t s;

    n = 1 << fs->N;

    header(fs, n);
    for(s = 0; s < n; s++) {
        off = s * n;
        for(i = 0; i < n; i++) {
            state(fs, fs->matrix[i + off], s, i, i + off);
        }
        newline(fs);
    }
    footer(fs);
}


void fsm_print_state(fsm_data *fs)
{
    uint32_t i;
    unsigned char s;
    for(i = 0; i < fs->N; i++) {
        s = fsm_get_bit(fs, i);
        printf("%d", s);
    }
    printf("\n");
}

void fsm_toggle(fsm_data *fs, uint32_t pos)
{
    fs->state ^= 1 << (pos - 1);
}

unsigned char fsm_get_bit(fsm_data *fs, uint32_t pos)
{
    return ((1 << pos) & fs->state) >> pos;
}

static void text_header(fsm_data *fs, uint32_t n)
{

}

static void text_line(fsm_data *fs)
{
    printf("\n");
}

static void text_state(fsm_data *fs,
    unsigned char state, 
    uint32_t s,
    uint32_t n,
    uint32_t off)
{
    printf("%d", state);
}

static void text_footer(fsm_data *fs) { };

void fsm_print_text(fsm_data *fs)
{
    fsm_print_matrix(fs,
        text_header,
        text_state,
        text_line,
        text_footer);
}

static void dot_header(fsm_data *fs, uint32_t n)
{
    printf("digraph {\n");
}

static void dot_line(fsm_data *fs)
{
    printf("\n");
}

static void dot_state(fsm_data *fs,
    unsigned char state, 
    uint32_t s,
    uint32_t n,
    uint32_t off)
{
    if(state != 0) {
        printf("\t%d -> %d;\n", s, n);
    }
}

static void dot_footer(fsm_data *fs) { 
    printf("}\n");
};

void fsm_print_dot(fsm_data *fs)
{
    fsm_print_matrix(fs,
        dot_header,
        dot_state,
        dot_line,
        dot_footer);
}

int fsm_find_min(fsm_data *fs, uint32_t s1, uint32_t s2, uint32_t count, int min)
{
    uint32_t i;
    uint32_t new;
    int tmp;

    if(count > fs->max_depth) return -1;

    //printf("finding %d->%d\n", s1, s2);
    for(i = 1; i <= fs->N; i++) {
        //print_spaces(count, i);
        fsm_set_state(fs, s1);
        fsm_compute_state(fs, i);
        new = fsm_get_state(fs);

        if(new != s1) {
            if(new == s2) {
                if(count < min) {
                    return count;
                } else {
                    return min;
                }
            } else {
                tmp = fsm_find_min(fs, new, s2, count + 1, min);
                if(tmp < min && tmp > 0) min = tmp;
            }
        }
    }

    return min;
}

void fsm_compute_minpaths(fsm_data *fs)
{
    uint32_t n2;
    uint32_t s;
    int min;

    n2 = 1 << fs->N;

    for(s = 1; s < n2; s++) {
        min = fsm_find_min(fs, s, 0, 1, 100);
        if(min == 100) fs->minpaths[s] = -1;
        else fs->minpaths[s] = min;
    }
}

static void mp_text_print(fsm_data *fs, uint32_t s, int num)
{
    printf("%d:%d\n", s, num);
}

static void mp_nil(fsm_data *fs) {}

void fsm_print_minpaths(fsm_data *fs, 
        void (*header)(fsm_data*),
        void (*text)(fsm_data*, uint32_t, int),
        void (*footer)(fsm_data*))
{
    uint32_t s;
    uint32_t n2;
    n2 = 1 << fs->N;
    header(fs);
    for(s = 1; s < n2; s++) {
        text(fs, s, fs->minpaths[s]);
    }
    footer(fs);
}

void fsm_print_minpaths_text(fsm_data *fs)
{
    fsm_print_minpaths(fs, mp_nil, mp_text_print, mp_nil);
}

static void mp_latex_print(fsm_data *fs, uint32_t s, int num)
{

    if(num > 0) {
        printf("%d & %d \\\\\n", s, num);
    } else {
        printf("%d & $\\infty$ \\\\\n", s);
    }
}

static void mp_latex_header(fsm_data *fs) 
{
    printf("\\subsection {Minimum Moves to Zero}\n");
    printf("\\begin{tabular}{ c | c } \n");
    printf("\\textbf{State} & \\textbf{Minimum}\\\\\n");
    printf("\\hline\n");
}


static void mp_latex_footer(fsm_data *fs) 
{
    printf("\\end{tabular}\n");
}

void fsm_print_minpaths_latex(fsm_data *fs)
{
    fsm_print_minpaths(fs, mp_latex_header, mp_latex_print, mp_latex_footer);
}

void fsm_assign_id(fsm_data *fs, uint32_t pos, int id)
{
    fs->obj[pos - 1] = id;
}

int fsm_get_id(fsm_data *fs, uint32_t pos)
{
    return fs->obj[pos - 1];
}
