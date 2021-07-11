
GtkWidget *new_modwin(char *title, struct _dragpad **retpad);
int connect_idx_fromio(struct descr_connect *con, int num, int out);
int connect_io_fromidx(struct descr_connect *c, int idx, int *nump, int *outp);
GtkWidget *modbox_new_bydescr(struct descr_mod *descr);
void check_name(char *name);

