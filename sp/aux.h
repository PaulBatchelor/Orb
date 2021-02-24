typedef struct sp_auxdata {
    size_t size;
    void *ptr;
} sp_auxdata;

int sp_auxdata_alloc(sp_auxdata *aux, size_t size);
int sp_auxdata_free(sp_auxdata *aux);
