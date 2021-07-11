
#include "mod.h"
#include "modulate.h"
#include "misc.h"


static struct descr_param P[] = {
    {0}
};
static struct descr_connect C[] = {
#define C_INA 0
    {"a", "A", CONTYPE_IN | CONTYPE_FAST},
#define C_INB 1
    {"b", "B", CONTYPE_IN | CONTYPE_FAST},
#define C_OUT 2
    {"out", "A*B", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new modulatemodule(name);
}

struct descr_mod descr_modulatemodule = { "modulate", C, P, FAST, B };

modulatemodule::modulatemodule(char *name) :
    module(&descr_modulatemodule, name)
{
}

modulatemodule::~modulatemodule()
{
}

void
modulatemodule::fasttick()
{
    connect[C_OUT] = (connect[C_INA] * connect[C_INB]) >> 16;
}

