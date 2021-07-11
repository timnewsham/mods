
#include "mod.h"
#include "const.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_VAL 0
    {"val", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_OUT 0
    {"out", "Constant Out", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new constmodule(name);
}

struct descr_mod descr_constmodule = { "const", C, P, SLOW, B };

constmodule::constmodule(char *name) :
    module(&descr_constmodule, name)
{
    connect[C_OUT] = 0;
}

constmodule::~constmodule()
{
}

int
constmodule::proc_params()
{
    connect[C_OUT] = param[P_VAL].i;
    return 0;
}

