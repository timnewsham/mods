/*
 * Low frequence oscillator,  see notes for the generator in gen.cpp
 * which this is based on 
 */

#include "mod.h"
#include "lfo.h"
#include "misc.h"
#include "func.h"

static struct descr_param P[] = {
    {0}
};
static struct descr_connect C[] = {
#define C_FREQ 0
    {"freq", "Frequency", CONTYPE_IN | CONTYPE_SLOW},
#define C_DUTY 1
    {"duty", "Duty", CONTYPE_IN | CONTYPE_SLOW},
#define C_AMP 2
    {"amp", "Amplitude", CONTYPE_IN | CONTYPE_SLOW},
#define C_OFF 3
    {"off", "Offset", CONTYPE_IN | CONTYPE_SLOW},
#define C_GATE 4
    {"gate", "Gate", CONTYPE_IN | CONTYPE_SLOW},
#define C_FUNC 5
#define FUNC_SIN 0
#define FUNC_TRI 1
#define FUNC_SAW 2
#define FUNC_SQUARE 3
    {"func", "Function", CONTYPE_IN | CONTYPE_SLOW},
#define C_GENOUT 6
    {"out", "LFO Output", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new lfomodule(name);
}

struct descr_mod descr_lfomodule = { "lfo", C, P, SLOW, B };

lfomodule::lfomodule(char *name) :
    module(&descr_lfomodule, name)
{
}

lfomodule::~lfomodule()
{
}

/* update theta and output */
void
lfomodule::slowtick()
{
    int freq, duty, func, gate, amp, off;
    int adv, dutytheta, (*genfunc)(int, int);

    freq = connect[C_FREQ];
    duty = connect[C_DUTY];
    func = connect[C_FUNC];
    gate = (connect[C_GATE] != 0);
    amp = connect[C_AMP] * gate;
    off = connect[C_OFF];

    adv = ((CYCLE << 16) * (u64_t)freq * slowdiv / (sampfreq * 100));
    dutytheta = (duty * CYCLE) / 100;
    switch(func) {
    case FUNC_SIN: genfunc = intsin; break;
    case FUNC_TRI: genfunc = inttri; break;
    case FUNC_SAW: genfunc = intsaw; break;
    case FUNC_SQUARE: genfunc = intsquare; break;
    default:
        die("bad genfunction given");
    }

    connect[C_GENOUT] = off + ((amp * genfunc(theta >> 16, dutytheta)) >> 16);
    theta += adv;
}

