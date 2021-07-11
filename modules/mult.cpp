
#include "mod.h"
#include "mult.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_SCALE 0
    {"scale", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_INA 0
    {"a", "A", CONTYPE_IN | CONTYPE_SLOW},
#define C_INB 1
    {"b", "B", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 2
    {"out", "A*B", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new multmodule(name);
}

struct descr_mod descr_multmodule = { "mult", C, P, SLOW, B };

multmodule::multmodule(char *name) :
    module(&descr_multmodule, name)
{
    scalefactor = 1;
}

multmodule::~multmodule()
{
}

int
multmodule::proc_params()
{
    scalefactor = param[P_SCALE].i;
    return 0;
}

void
multmodule::slowtick()
{
    connect[C_OUT] = connect[C_INA] * connect[C_INB] / scalefactor;
}

