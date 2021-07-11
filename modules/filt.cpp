
#include <math.h>

#include "mod.h"
#include "filt.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_TYPE 0
    {"type", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_IN 0
    {"in", "Input", CONTYPE_IN | CONTYPE_FAST},
#define C_FC 1
    {"fc", "Filter Center Frequency", CONTYPE_IN | CONTYPE_SLOW},
#define C_RESO 2
    {"reso", "Resolution", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 3
    {"out", "Output", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new filtmodule(name);
}

struct descr_mod descr_filtmodule = { "filt", C, P, FAST, B };

filtmodule::filtmodule(char *name) :
    module(&descr_filtmodule, name)
{
    type = 0;
    a1 = a2 = 0.0;
    b0 = b1 = b2 = 0.0;
    store1 = store2 = 0.0;
    last_fc = last_res = -1.0;
}

filtmodule::~filtmodule()
{
}

int
filtmodule::proc_params()
{
    type = param[P_TYPE].i;
    if(type > 4 || type < 0)
        return -1;

    a1 = a2 = 0.0;
    b0 = b1 = b2 = 0.0;
    store1 = store2 = 0.0;
    last_fc = last_res = -1.0;
    return 0;
}

// Update the constants based on the filters type and parameters
void
filtmodule::slowtick()
{
    float fc, res, alpha, x, cosfc;

    fc = connect[C_FC] * (2.0 * M_PI) / sampfreq;
    res = 0.01 * connect[C_RESO] + 0.000001;

    // dont bother with the math if we already know the answer
    if(fc == last_fc && res == last_res)
        return;
    last_fc = fc;
    last_res = res;

    alpha = sin(fc) / res;
    cosfc = cos(fc);
    x = 1.0 / (1.0 + alpha);

    a1 = 2.0 * x * cosfc;
    a2 = (alpha - 1.0) * x;

    switch(type) {
    case 0:  /* BANDPASS */
        b0 = alpha * x;
        b1 = 0.0;
        b2 = -alpha * x;
        break;
    case 1: /* LOWPASS */
        b1 = x * (1.0 - cosfc);
        b0 = b1 * 0.5;
        b2 = b0;
        break;
    case 2: /* HIPASS */
        b1 = -x * (1.0 + cosfc);
        b0 = -b1 * 0.5;
        b2 = b0;
        break;
    case 3: /* NOTCH */
        b0 = x;
        b1 = -2.0 * x * cosfc;
        b2 = x;
        break;
    }
}

// Update the output based on input and storage
//
// two-pole, two-zero filter
//
//  OUT         z^2 + (b1/b0) z + (b2/b0)
// ----- = b0 * -------------------------
//  IN          z^2 -    a1 z   -   a2
//
// for poles P and P*, and zeros N and N*:
//
//                     b0    = anything
//    a1 = 2 * Re{P}   b1/b0 = -2 * Re{N}
//    a2 = - |P|^2     b2/b0 = |N|^2
// 
void
filtmodule::fasttick()
{
    float v, out;

    v = connect[C_IN] + a1 * store1 + a2 * store2;
    out = b0 * v + b1 * store1 + b2 * store2;
    connect[C_OUT] = (int)out;

    store2 = store1;
    store1 = v;
}

