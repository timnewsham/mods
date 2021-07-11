/*
 * generator - we generate sin, square, triangle and sawtooth waveforms
 * at specified frequencies using fixed point math..  we use 0x200
 * phase ticks per cycle, and keep a 16.16 formatted phase counter
 * and phase advance to keep track of the phase and frequency
 */

#include "mod.h"
#include "gen.h"
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
#define C_GATE 3
    {"gate", "Gate", CONTYPE_IN | CONTYPE_SLOW},
#define C_FUNC 4
#define FUNC_SIN 0
#define FUNC_TRI 1
#define FUNC_SAW 2
#define FUNC_SQUARE 3
    {"func", "Function", CONTYPE_IN | CONTYPE_SLOW},
#define C_GENOUT 5
    {"out", "Generator Output", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new genmodule(name);
}

struct descr_mod descr_genmodule = { "gen", C, P, FAST, B };


genmodule::genmodule(char *name) :
    module(&descr_genmodule, name)
{
    genfunc = intsin;
}

genmodule::~genmodule()
{
}

/* update frequency, duty and function */
void
genmodule::slowtick()
{
    int freq, duty, func;

    freq = connect[C_FREQ];
    duty = connect[C_DUTY];
    func = connect[C_FUNC];

    adv = ((CYCLE << 16) * (u64_t)freq / (sampfreq * 100));
    dutytheta = (duty * CYCLE) / 100;
    switch(func) {
    case FUNC_SIN: genfunc = intsin; break;
    case FUNC_TRI: genfunc = inttri; break;
    case FUNC_SAW: genfunc = intsaw; break;
    case FUNC_SQUARE: genfunc = intsquare; break;
    default:
        die("bad genfunction given");
    }
}

/* update output */
void
genmodule::fasttick()
{
    int amp, gate;

    gate = (connect[C_GATE] != 0);
    if(gate) {
        amp = connect[C_AMP];
        connect[C_GENOUT] = (amp * genfunc(theta >> 16, dutytheta)) >> 16;

        theta += adv;
    }
}

