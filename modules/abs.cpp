
#include "mod.h"
#include "abs.h"


static struct descr_param P[] = {
    {0}
};
static struct descr_connect C[] = {
#define C_IN 0
    {"in", "Input", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 1
    {"out", "Absolute", CONTYPE_OUT | CONTYPE_SLOW},
#define C_SIGN 2
    {"sign", "Sign", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new absmodule(name);
}

struct descr_mod descr_absmodule = { "abs", C, P, SLOW, B };

absmodule::absmodule(char *name) :
    module(&descr_absmodule, name)
{
}

absmodule::~absmodule()
{
}

void
absmodule::slowtick()
{
    int in = connect[C_IN];

    if(in < 0) {
        connect[C_OUT] = -in;
        connect[C_SIGN] = -1;
    } else {
        connect[C_OUT] = in;
        connect[C_SIGN] = 1;
    }
}

