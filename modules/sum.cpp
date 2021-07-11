
#include "mod.h"
#include "sum.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_SIGN 0
    {"sign", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_INA 0
    {"a", "A", CONTYPE_IN | CONTYPE_SLOW},
#define C_INB 1
    {"b", "B", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 2
    {"out", "A+B", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new summodule(name);
}

struct descr_mod descr_summodule = { "sum", C, P, SLOW, B };

summodule::summodule(char *name) :
    module(&descr_summodule, name)
{
    sign = 0;
}

summodule::~summodule()
{
}

int
summodule::proc_params()
{
    sign = param[P_SIGN].i;
    return 0;
}

void
summodule::slowtick()
{
    if(sign)
        connect[C_OUT] = connect[C_INA] - connect[C_INB];
    else
        connect[C_OUT] = connect[C_INA] + connect[C_INB];
}


