/*
 * sample - we play samples sort of like we play sin waves from
 * a sine table - we compute the advance rate such that a freq of
 * 440Hz will play the sample at "normal" speed.
 * We keep a 16.16 formatted sample counter and sample advance to keep 
 * track of the position and frequency
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "mod.h"
#include "samp.h"
#include "misc.h"

static struct descr_param P[] = {
#define P_FILE 0
    {"file", 's'},
#define P_FREQ 1
    {"freq", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_FREQ 0
    {"freq", "Frequency", CONTYPE_IN | CONTYPE_SLOW},
#define C_AMP 1
    {"amp", "Amplitude", CONTYPE_IN | CONTYPE_SLOW},
#define C_GATE 2
    {"gate", "Gate", CONTYPE_IN | CONTYPE_SLOW},
#define C_TRIG 3
    {"trig", "Trigger", CONTYPE_IN | CONTYPE_SLOW},
#define C_OUT 4
    {"out", "Sample Output", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new sampmodule(name);
}

struct descr_mod descr_sampmodule = { "samp", C, P, FAST, B };


// read a sample from a file
// XXX add error checking in place of abort
static void
readsampfile(char *fname, short **bufp, int *lenp)
{
    FILE *fp;
    short *buf;
    int len;

    fp = fopen(fname, "r");
    if(!fp) 
        die("Error opening %s: %s\n", fname, strerror(errno));

    // get file length
    fseek(fp, 0, SEEK_END);
    len = ftell(fp) / sizeof(short);
    rewind(fp);

    buf = new short[len];
    if((int)fread(buf, sizeof(short), len, fp) != len)
        die("Short read on %s\n", fname);
    fclose(fp);

    *bufp = buf;
    *lenp = len;
}

sampmodule::sampmodule(char *name) :
    module(&descr_sampmodule, name)
{
    origfreq = 0;
    pos = 0;
    lasttrig = 0;
    adv = 0;
    buf = 0;
    buflen = 0;
}

sampmodule::~sampmodule()
{
    delete [] buf;
}

int
sampmodule::proc_params()
{
    origfreq = param[P_FREQ].i;
    pos = 0;
    lasttrig = 0;
    adv = 0;

    if(buf)
        delete [] buf;
    // XXX error checking, not die()
    readsampfile(param[P_FILE].s, &buf, &buflen);
    return 0;
}

/* update frequency, duty and function */
void
sampmodule::slowtick()
{
    int freq, trig;

    // check retrigger
    trig = (connect[C_TRIG] != 0);
    if(trig && !lasttrig)
        pos = 0;
    lasttrig = trig;

    freq = connect[C_FREQ];
    adv = (((u64_t)freq) << 16) * origfreq / (44000 * sampfreq);
}

/* update output */
void
sampmodule::fasttick()
{
    int amp;

    if(connect[C_GATE] != 0) {
        amp = connect[C_AMP];
        connect[C_OUT] = (amp * buf[pos >> 16]) >> 16;

        pos += adv;
        while((pos >> 16) > buflen)
            pos -= ((u64_t)buflen) << 16;
    }

}

