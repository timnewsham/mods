
#include <stdio.h> //XXX FILE* needed below..ugh,  fix this somehow

class module;

struct modlist {
    struct modlist *next;
    module *mod;
};

struct wire {
    struct wire *next;

    class guipeer *peer;
    int *frmp, *top;
    module *frm;
    module *to;
    int frmidx;
    int toidx;
    int fast;
};

module *find_module(char *);
void slowtick(void);
void fasttick(void);
void drive(void);
void add_wire(module*, int, module*, int);
void rem_wire(module*, int, module*, int);
void rem_all_wires(void);
void rem_wire_common(struct wire *);
void rem_module(module *);
void rem_all_modules(void);
void add_module(module *, int);

void write_modules(FILE *);
void write_wires(FILE *);

int save_state(char *);
int load_state(char *);

