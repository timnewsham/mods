
#include "mod.h"
#include "switch.h"


static struct descr_param P[] = {
    {0}
};
static struct descr_connect C[] = {
#define C_INA 0
    {"a", "Input A", CONTYPE_IN | CONTYPE_SLOW},
#define C_INB 1
    {"b", "Input B", CONTYPE_IN | CONTYPE_SLOW},
#define C_INC 2
    {"c", "Input C", CONTYPE_IN | CONTYPE_SLOW},
#define C_IND 3
    {"d", "Input D", CONTYPE_IN | CONTYPE_SLOW},
#define C_SEL 4
    {"sel", "Select", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 5
    {"out", "Output", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new switchmodule(name);
}

struct descr_mod descr_switchmodule = { "switch", C, P, SLOW, B };

switchmodule::switchmodule(char *name) :
    module(&descr_switchmodule, name)
{
}

switchmodule::~switchmodule()
{
}

void
switchmodule::slowtick()
{
    switch(connect[C_SEL] & 0x3) {
    case 0:
        connect[C_OUT] = connect[C_INA];
        break;
    case 1:
        connect[C_OUT] = connect[C_INB];
        break;
    case 2:
        connect[C_OUT] = connect[C_INC];
        break;
    case 3:
        connect[C_OUT] = connect[C_IND];
        break;
    }
}

