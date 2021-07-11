
#include <string.h>

#include "mod.h"
#include "delay.h"
#include "misc.h"


static struct descr_param P[] = {
#define P_MAXDELAY 0
    {"maxdelay", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_IN 0
    {"in", "Input", CONTYPE_IN | CONTYPE_FAST},
#define C_DELAY 1
    {"delay", "Delay time", CONTYPE_IN | CONTYPE_SLOW},
#define C_FEEDBACK 2
    {"feedback", "Feedback", CONTYPE_IN | CONTYPE_SLOW},
#define C_MIX 3
    {"mix", "Mix wet/dry", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 4
    {"out", "Output", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new delaymodule(name);
}

struct descr_mod descr_delaymodule = {"delay", C, P, FAST, B };

delaymodule::delaymodule(char *name) :
    module(&descr_delaymodule, name)
{
    bufsize = 0;
    buf = 0;
    pos = 0;
    delaypos = 0;
    fbfactor = 0;
    interp = 0;
}

delaymodule::~delaymodule()
{
    if(buf)
        delete [] buf;
}

int
delaymodule::proc_params()
{
    if(buf)
        delete [] buf;

    bufsize = 1 + param[P_MAXDELAY].i * sampfreq / 1000;
    buf = new short[bufsize];
    memset(buf, 0, bufsize * sizeof buf[0]);
    pos = 0;
    delaypos = 0;
    fbfactor = 0;
    interp = 0;
    return 0;
}

// update the delaypos based on C_DELAY
void
delaymodule::slowtick()
{
    int x;

    /* calculate samples and 256ths of samples to delay by */
    x = connect[C_DELAY] * ((u64_t)sampfreq  << 8) / 1000000;
    interp = x & 0xff;
    x >>= 8;

    if(x >= bufsize) {
        x = bufsize - 1;
        interp = 0;
    }
    if(x < 0) {
        x = 0;
        interp = 0;
    }
    delaypos = pos - x;
    if(delaypos < 0)
        delaypos += bufsize;

    x = connect[C_FEEDBACK];
    if(x < 0)
        x = 0;
    if(x > 100)
        x = 100;
    fbfactor = (x << 8) / 100;
    ftfactor = ((100 - x) << 8) / 100;

    x = connect[C_MIX];
    if(x < 0)
        x = 0;
    if(x > 100)
        x = 100;
    mixwet = (x << 8) / 100;
    mixdry = ((100 - x) << 8) / 100;
}

// update output and positions
void
delaymodule::fasttick()
{
    int in, del, del2, delaypos2;

    pos ++;
    if(pos > bufsize)
        pos = 0;
    delaypos2 = delaypos;
    delaypos ++;
    if(delaypos > bufsize)
        delaypos = 0;

    /* interpolate between samples (ugh, linear, hope this isnt too noisy) */
    del = buf[delaypos];
    del2 = buf[delaypos2];
    del += ((del2 - del) * interp) >> 8;
    
    in = connect[C_IN];
    connect[C_OUT] =  (in * mixdry + del * mixwet) >> 8;
    del2 =  in + ((del * fbfactor) >> 8);
    if(del2 > 32767)
        del2 = 32767;
    if(del2 < -32768)
        del2 = -32768;
    buf[pos] = del2;
}


