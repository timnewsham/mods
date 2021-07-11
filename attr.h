
struct attrlist {
    struct attrlist *next;
    char *name;
    char *val;
};

void attrlist_add(struct attrlist **headp, char *name, char *val);
char *attrlist_find(struct attrlist **headp, char *name);
//void attrlist_rem(struct attrlist **headp, char *name);
char *attrlist_string(struct attrlist **headp);
int attrlist_parse(struct attrlist **headp, char *);

