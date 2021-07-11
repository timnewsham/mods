
#include "mod.h"
#include "split.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_BREAK1 0
    {"break1", 'i'},
#define P_BREAK2 1
    {"break2", 'i'},
#define P_BREAK3 2
    {"break3", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_IN 0
    {"in", "Input", CONTYPE_IN | CONTYPE_SLOW},
#define C_GATE 1
    {"gate", "Gate", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT1 2
    {"out1", "Output 1", CONTYPE_OUT | CONTYPE_SLOW},
#define C_GATE1 3
    {"gate1", "Gate 1", CONTYPE_OUT | CONTYPE_SLOW},
#define C_OUT2 4
    {"out2", "Output 2", CONTYPE_OUT | CONTYPE_SLOW},
#define C_GATE2 5
    {"gate2", "Gate 2", CONTYPE_OUT | CONTYPE_SLOW},
#define C_OUT3 6
    {"out3", "Output 3", CONTYPE_OUT | CONTYPE_SLOW},
#define C_GATE3 7
    {"gate3", "Gate 3", CONTYPE_OUT | CONTYPE_SLOW},
#define C_OUT4 8
    {"out4", "Output 4", CONTYPE_OUT | CONTYPE_SLOW},
#define C_GATE4 9
    {"gate4", "Gate 4", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new splitmodule(name);
}

struct descr_mod descr_splitmodule = { "split", C, P, SLOW, B };


splitmodule::splitmodule(char *name) :
    module(&descr_splitmodule, name)
{
    break1 = break2 = break3 = 0;
}

splitmodule::~splitmodule()
{
}

int
splitmodule::proc_params()
{
    break1 = param[P_BREAK1].i;
    break2 = param[P_BREAK2].i;
    break3 = param[P_BREAK3].i;
    return 0;
}

void
splitmodule::slowtick()
{
    connect[C_GATE1] = 0;
    connect[C_GATE2] = 0;
    connect[C_GATE3] = 0;
    connect[C_GATE4] = 0;

    if(connect[C_GATE]) {
        int in = connect[C_IN];
        if(in < break1) {
            connect[C_OUT1] = in;
            connect[C_GATE1] = 1;
        } else if(in < break2) {
            connect[C_OUT2] = in;
            connect[C_GATE2] = 1;
        } else if(in < break3) {
            connect[C_OUT3] = in;
            connect[C_GATE3] = 1;
        } else {
            connect[C_OUT4] = in;
            connect[C_GATE4] = 1;
        }
    }
}

