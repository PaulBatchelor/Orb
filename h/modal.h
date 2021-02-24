typedef struct {
    sp_mode *mode[4];
    sp_dcblocker *dc;
    int type;
    int ptype;
    SPFLOAT gate;
    SPFLOAT prev;
    SPFLOAT amp;
    SPFLOAT metal[8];
    SPFLOAT plastic[8];
    SPFLOAT avoid[8];
    SPFLOAT *mat[3];
} sp_modal;

int sp_modal_create(sp_modal **m);
int sp_modal_destroy(sp_modal **m);
int sp_modal_init(sp_data *sp, sp_modal *m);
int sp_modal_compute(sp_data *sp, sp_modal *m, SPFLOAT *in, SPFLOAT *out);

void sp_modal_scale(sp_modal *m, SPFLOAT amp);

void sp_modal_type(sp_modal *m, int type);

void sp_modal_reset(sp_modal *m);
