#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NO_LIBSNDFILE
#include <sndfile.h>
#endif
#include "soundpipe.h"
/* #include "kiss_fftr.h" */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define tpd360 0.0174532925199433

int sp_ftbl_create(sp_data *sp, sp_ftbl **ft, size_t size)
{
    sp_ftbl *ftp;
    *ft = malloc(sizeof(sp_ftbl));
    ftp = *ft;
    ftp->tbl = calloc(1, sizeof(SPFLOAT) * (size + 1));
    ftp->size = size;
    ftp->del = 1;
    return SP_OK;
}

/* like create, but use externally managed memory */

int sp_ftbl_bind(sp_data *sp, sp_ftbl **ft, SPFLOAT *tbl, size_t size)
{
    sp_ftbl *ftp;
    *ft = malloc(sizeof(sp_ftbl));
    ftp = *ft;
    ftp->tbl = tbl;
    ftp->size = size;
    ftp->del = 0;
    return SP_OK;
}

int sp_ftbl_destroy(sp_ftbl **ft)
{
    sp_ftbl *ftp = *ft;
    if (ftp->del) free(ftp->tbl);
    free(*ft);
    return SP_OK;
}

static char * tokenize(char **next, int *size)
{
    char *token;
    char *str;
    char *peak;

    if (*size <= 0) return NULL;
    token = *next;
    str = *next;

    peak = str + 1;

    while ((*size)--) {
        if (*str == ' ') {
            *str = 0;
            if (*peak != ' ') break;
        }
        str = str + 1;
        peak = str + 1;
    }
    *next = peak;
    return token;
}

int sp_gen_vals(sp_data *sp, sp_ftbl *ft, const char *string)
{
    int size;
    char *str;
    char *out;
    char *ptr;
    int j;

    size = strlen(string);
    str = malloc(sizeof(char) * size + 1);
    strcpy(str, string);
    ptr = str;
    j = 0;

    while (size > 0) {
        out = tokenize(&str, &size);
        if (ft->size < j + 1) {
            ft->tbl = realloc(ft->tbl, sizeof(SPFLOAT) * (ft->size + 2));
            /* zero out new tables */
            ft->tbl[ft->size] = 0;
            ft->tbl[ft->size + 1] = 0;
            ft->size++;
        }
        ft->tbl[j] = atof(out);
        j++;
    }
    free(ptr);
    return SP_OK;
}

int sp_gen_sine(sp_data *sp, sp_ftbl *ft)
{
    unsigned long i;
    SPFLOAT step = 2 * M_PI / ft->size;
    for (i = 0; i < ft->size; i++) {
        ft->tbl[i] = sin(i * step);
    }
    return SP_OK;
}
