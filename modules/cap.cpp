
#include <stdio.h>

#include "mod.h"
#include "cap.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_FNAME 0
    {"fname",  's'},
#define P_THRESH 1
    {"thresh", 'i'},
#define P_LEN 2
    {"len",    'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_IN 0
    {"in", "Input", CONTYPE_IN | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new capmodule(name);
}

struct descr_mod descr_capmodule = {"cap", C, P, SLOW, B };

capmodule::capmodule(char *name) :
    module(&descr_capmodule, name)
{
    out = 0;
    len = 0;
    curlen = 0;
    thresh = 0;
}

capmodule::~capmodule()
{
    if(out)
        fclose(out);
}

int
capmodule::proc_params()
{
    len = param[P_LEN].i;
    thresh = param[P_THRESH].i;

    if(out)
        fclose(out);

    out = fopen(param[P_FNAME].s, "w");
    if(!out)
        return -1;
    curlen = 0;
    return 0;
}

void
capmodule::slowtick()
{
    short in;

    if(curlen < len) {
        in = connect[C_IN];
        if(curlen > 0 || (curlen == 0 && in > thresh)) {
            fwrite(&in, sizeof in, 1, out);
            curlen++;
            if(curlen == len) {
                fclose(out);
                out = 0;
            }
        }
    }
}

