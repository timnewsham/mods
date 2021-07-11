
#include "mod.h"
#include "env.h"
#include "misc.h"


static struct descr_param P[] = {
    {0}
};
static struct descr_connect C[] = {
#define C_GATE 0
    {"gate", "Gate", CONTYPE_IN | CONTYPE_SLOW},
#define C_ATTACK 1
    {"attack", "Attack", CONTYPE_IN | CONTYPE_SLOW},
#define C_DECAY 2
    {"decay", "Decay", CONTYPE_IN | CONTYPE_SLOW},
#define C_SUSTAIN 3
    {"sustain", "Sustain", CONTYPE_IN | CONTYPE_SLOW},
#define C_RELEASE 4
    {"release", "Release", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 5
    {"out", "Output", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new envmodule(name);
}

struct descr_mod descr_envmodule = {"env", C, P, SLOW, B };

envmodule::envmodule(char *name) :
    module(&descr_envmodule, name)
{
    phase = release;
}

envmodule::~envmodule()
{
}

void
envmodule::slowtick()
{
    int gate, val, delta, suslevel;

    // XXX should we do this with 16.16 values for better
    // decay behavior?
    val = connect[C_OUT];
    gate = (connect[C_GATE] != 0);
    if(gate) {
        if(phase == release) {
            // start with new attack, from 0
            phase = attack;
            val = 0;
        }
        if(phase == attack && val == SAMPMAX)
            phase = decay;
    }
    if(!gate)
        phase = release;

    switch(phase) {
    case attack:
        delta = ((u64_t)1000 << 16) * slowdiv /
                (sampfreq * connect[C_ATTACK]);
        break;
    case decay:
        suslevel = SAMPMAX * connect[C_SUSTAIN] / 100;
        if(val > suslevel) {
            delta = - (val - suslevel) * connect[C_DECAY] / 1000;
            if(delta == 0)
                delta = -1;
        } else    
            delta = 0;
        break;
    case release:
        suslevel = SAMPMAX * connect[C_SUSTAIN] / 100;
        delta = -val * connect[C_RELEASE] / 1000;
        if(delta == 0)
            delta = -1;
        break;
    default:
        die("illegal phase in envelope slowtick");
    }
    val += delta;
    if(val > SAMPMAX)
        val = SAMPMAX;
    if(val < 0)
        val = 0;
    connect[C_OUT] = val;
}

