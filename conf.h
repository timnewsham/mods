
struct modtype_list {
    struct modtype_list *next;
    struct descr_mod *modtype;
};

extern struct modtype_list *all_modtypes;

int readconf(char *fname);
int writeconf(char *fname);
module *build_modtype(char *tname, char *name);
int register_modtypes(struct descr_mod *mods[], int cnt);

