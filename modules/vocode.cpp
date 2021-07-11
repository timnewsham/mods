
#include <math.h>

#include "mod.h"
#include "vocode.h"
#include "misc.h"

#define MAXORDER	30

/* There is probably a better place for this than here */
static float rand_tab[256] = {
0.561870, 0.468459, 0.918728, 0.838229, 0.832475, 0.262188, 0.493355, 0.962525, 
0.800524, 0.210615, 0.059413, 0.161677, 0.161782, 0.539682, 0.335628, 0.820303, 
0.527257, 0.980635, 0.127923, 0.683801, 0.575639, 0.143094, 0.797275, 0.084834, 
0.247218, 0.419249, 0.141058, 0.655674, 0.164940, 0.178296, 0.072558, 0.726810, 
0.646755, 0.991286, 0.565039, 0.479230, 0.253474, 0.058394, 0.441755, 0.053997, 
0.269009, 0.501168, 0.215674, 0.430791, 0.040851, 0.551301, 0.251093, 0.568108, 
0.531937, 0.379017, 0.251908, 0.107576, 0.522111, 0.049184, 0.192410, 0.769329, 
0.468432, 0.333468, 0.425003, 0.633373, 0.511764, 0.497561, 0.360183, 0.158520, 
0.488847, 0.925222, 0.637750, 0.742321, 0.983616, 0.079505, 0.796318, 0.252625, 
0.580673, 0.011992, 0.683416, 0.621524, 0.563293, 0.934509, 0.189631, 0.095230, 
0.313526, 0.441540, 0.202805, 0.835637, 0.490724, 0.395215, 0.604966, 0.959156, 
0.728683, 0.029970, 0.592528, 0.240448, 0.527531, 0.952711, 0.398967, 0.016378, 
0.877933, 0.036717, 0.758699, 0.861550, 0.116223, 0.555017, 0.114175, 0.696896, 
0.567009, 0.797591, 0.318420, 0.130302, 0.732100, 0.508051, 0.225532, 0.045627, 
0.949591, 0.428338, 0.881264, 0.440315, 0.823553, 0.486230, 0.399471, 0.552236, 
0.516200, 0.991999, 0.792684, 0.043731, 0.944710, 0.191651, 0.060109, 0.822644, 
0.228368, 0.818808, 0.684193, 0.344591, 0.373825, 0.798368, 0.041487, 0.940834, 
0.595959, 0.359907, 0.071136, 0.328059, 0.867958, 0.296669, 0.373686, 0.817549, 
0.725006, 0.254949, 0.257864, 0.548559, 0.741179, 0.657335, 0.100795, 0.257379, 
0.649334, 0.893479, 0.301110, 0.594044, 0.085129, 0.361219, 0.416688, 0.313498, 
0.180028, 0.100881, 0.658089, 0.553853, 0.899249, 0.699575, 0.494687, 0.495208, 
0.059482, 0.565824, 0.823267, 0.927440, 0.862492, 0.196953, 0.744990, 0.587498, 
0.451902, 0.002854, 0.136057, 0.193082, 0.660188, 0.236852, 0.450461, 0.309522, 
0.130330, 0.751571, 0.903566, 0.215460, 0.112790, 0.320254, 0.528958, 0.292818, 
0.421134, 0.187046, 0.846670, 0.320383, 0.886622, 0.341358, 0.815591, 0.946104, 
0.907181, 0.638858, 0.873544, 0.769673, 0.835811, 0.618533, 0.357171, 0.287713, 
0.621387, 0.493228, 0.480795, 0.281576, 0.730080, 0.931256, 0.591098, 0.860411, 
0.682826, 0.494664, 0.075871, 0.795616, 0.814918, 0.604828, 0.088434, 0.236052, 
0.791874, 0.935104, 0.556435, 0.678496, 0.276462, 0.372026, 0.624600, 0.183643, 
0.010884, 0.498144, 0.953316, 0.846695, 0.116677, 0.310487, 0.134408, 0.738064, 
0.803716, 0.615203, 0.019640, 0.533796, 0.546458, 0.610737, 0.394207, 0.229285, 
0.105401, 0.470078, 0.024901, 0.920319, 0.074906, 0.113335, 0.156371, 0.866780, 
};

static int randpos = 0;

static struct descr_param P[] = {
#define P_ORDER 0
    {"order", 'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_IN 0
    {"in", "Input", CONTYPE_IN | CONTYPE_FAST},
#define C_VOICE 1
    {"voice", "Voice Input", CONTYPE_IN | CONTYPE_FAST},
#define C_OUT 2
    {"out", "Output", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new vocodemodule(name);
}

struct descr_mod descr_vocodemodule = { "vocoder", C, P, FAST, B };

/*
 * compute cnt reflection coefficients given the autocorrelation
 * values from 0 to cnt.  Results placed in k[0] through k[cnt - 1].
 * The error is left in k[cnt].
 */
static int
levinson_durbin(int cnt, float *autocor, float *k)
{
    float a[MAXORDER + 1], b[MAXORDER + 1], x, err;
    int i, j;

    if(cnt > MAXORDER)
        die("oops, order is too high for levinson-durbin");

    err = autocor[0];
    for(i = 1; i <= cnt; i++) {
        if(err == 0) {
            return -1;
            //k[i - 1] = 0.0;
            //break;
        }

        x = autocor[i];
        for(j = 1; j < i; j++)
            x -= a[j] * autocor[i - j];
        x /= err;
#ifdef nope
        if(x <= -1.0 || x >= 1.0)
            return -1;
#endif
        k[i - 1] = -x;

        a[i] = x;
        for(j = 1; j < i; j++)
            b[j] = a[j] - x * a[i - j];
        for(j = 1; j < i; j++)
            a[j] = b[j];

        err *= (1 - x * x);
    }
    k[cnt] = sqrt(err);
    return 0;
}

vocodemodule::vocodemodule(char *name) :
    module(&descr_vocodemodule, name)
{
    int i;

    store = 0;
    k = 0;
    autocorr = 0;
    order = 0;
    valid = 0;

    for(i = 0; i < SAMPLES; i++)
        sample[i] = 0;
    pos = 0;
}

vocodemodule::~vocodemodule()
{
    if(store)
        delete [] store;
    if(autocorr)
        delete [] autocorr;
    if(k)
        delete [] k;
}

int
vocodemodule::proc_params()
{
    int i;

    order = param[P_ORDER].i;
    if(order < 1 || order > MAXORDER)
        return -1;

    if(store)
        delete [] store;
    if(autocorr)
        delete [] autocorr;
    if(k)
        delete [] k;
    
    store = new float[order + 1];
    k = new float[order + 1];
    autocorr = new float[order + 1];
    valid = 0;

    for(i = 0; i < order; i++)
        store[i] = 0.0;

    for(i = 0; i < SAMPLES; i++)
        sample[i] = 0;
    pos = 0;
    return 0;
}

// analyze stored speech
void
vocodemodule::slowtick()
{
    int n, i, j, low, high;
    float sum, peak;

    if(pos != SAMPLES)
        return;
    pos = 0;

    /* compute first order+1 autocorrelation factors of samples */
    for(n = 0; n <= order; n++) {
        sum = 0.0;
        for(i = 0, j = n; j < SAMPLES; i++, j++)
            sum += sample[i] * sample[j];
        autocorr[n] = sum;
    }

    /* search autocorrelation for a peak from 3ms to 15ms */
    low = 3 * sampfreq / 1000;
    high = 15 * sampfreq / 1000;
    if(high > SAMPLES)
        high = SAMPLES;
    peak = 0.0;
    for(n = low; n < high; n++) {
        sum = 0.0;
        for(i = 0, j = n; j < SAMPLES; i++, j++)
            sum += sample[i] * sample[j];
        if(sum > peak)
            peak = sum;
    }

    if((autocorr[0] == 0.0) || ((peak / autocorr[0]) < 0.25))
        voiced = 0;
    else
        voiced = 1;

    /* use levinson-durbin recursion to solve for coefficients */
    valid = (levinson_durbin(order, autocorr, k) != -1);

    /* copy end of buf to start of buf and fill up rest */
    for(i = 0, j = SAMPLES - OVERLAP; j < SAMPLES; i++, j++)
        sample[i] = sample[j];
    pos = i;
}

void
vocodemodule::fasttick()
{
    float x;
    int i;

    if(pos < SAMPLES)
        sample[pos++] = connect[C_VOICE] * (1 / 32768.0);

    if(!valid)
        return;

    /* pass input through all-pole lattice filter */
    x = connect[C_IN] * k[order];
    if(!voiced)
        x *= rand_tab[randpos++ & 0xff] * 2.0;
    for(i = order - 1; i >= 0; i--) {
        x -= k[i] * store[i];
        store[i + 1] = store[i] + x * k[i];
    }
    store[0] = x;
    connect[C_OUT] = (int)x;
}

