/*
 * convert note values into frequencies
 */

#include <math.h>

#include "mod.h"
#include "freq.h"
#include "misc.h"

static struct descr_param P[] = {
    {0}
};
static struct descr_connect C[] = {
#define C_NOTEIN 0
    {"note", "Note In", CONTYPE_IN | CONTYPE_SLOW},
#define C_FREQOUT 1
    {"freq", "Frequency Out", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new freqmodule(name);
}

struct descr_mod descr_freqmodule = { "freq", C, P, SLOW, B };

freqmodule::freqmodule(char *name) :
    module(&descr_freqmodule, name)
{
}

freqmodule::~freqmodule()
{
}

/* update frequency */
void
freqmodule::slowtick()
{
    // XXX Using floating point math here, this can be done
    // more efficiently without the pow() function with a table
    // or something for one (high) octave and then dividing by
    // a power of two (right shifts) for changing octaves
    int note, freq;

    note = connect[C_NOTEIN] - ((4 * 12 - 3)  * 100);
    freq = (int) (100.0 * 440.0 * pow(2, note * (1.0 / 1200)) + 0.5);
    connect[C_FREQOUT] = freq;
}

