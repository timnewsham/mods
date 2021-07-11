
#include "mod.h"
#include "mix.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_SIGN 0
    {"sign", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_INA 0
    {"a", "A", CONTYPE_IN | CONTYPE_FAST},
#define C_INB 1
    {"b", "B", CONTYPE_IN | CONTYPE_FAST},
#define C_OUT 2
    {"out", "A+B", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new mixmodule(name);
}

struct descr_mod descr_mixmodule = { "mix", C, P, FAST, B };

mixmodule::mixmodule(char *name) :
    module(&descr_mixmodule, name)
{
    sign = 0;
}

mixmodule::~mixmodule()
{
}

int
mixmodule::proc_params()
{
    sign = param[P_SIGN].i;
    return 0;
}

void
mixmodule::fasttick()
{
    // XXX which is faster here, multiply or conditional?
    if(sign)
        connect[C_OUT] = (connect[C_INA] - connect[C_INB]) >> 1;
    else
        connect[C_OUT] = (connect[C_INA] + connect[C_INB]) >> 1;
}

