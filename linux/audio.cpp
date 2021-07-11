/*
 * linux audio driver
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "mod.h"
#include "audio.h"
#include "misc.h"

static struct descr_param P[] = {
#define P_FRAG 0
    {"frag", 'i'},
#define P_DEV 1
    {"device", 's'},
    {0}
};
static struct descr_connect C[] = {
#define C_LEFT 0
    {"left", "Left Speaker", CONTYPE_IN | CONTYPE_FAST},
#define C_RIGHT 1
    {"right", "Right Speaker", CONTYPE_IN | CONTYPE_FAST},
    {0}
};

static module *
B(char *name)
{
    return new audiomodule(name);
}

struct descr_mod descr_audiomodule = {"audio", C, P, FAST, B };


#ifndef NOAUDIO
static int
sound_setfmt(int fd, int freq, int fmt, int stereo, int fragsize)
{
    if(ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &fragsize) == -1)
        return -1;
    if(ioctl(fd, SNDCTL_DSP_SETFMT, &fmt) == -1)
        return -1;
    if(ioctl(fd, SNDCTL_DSP_STEREO, &stereo) == -1)
        return -1;
    if(ioctl(fd, SNDCTL_DSP_SPEED, &freq) == -1)
        return -1;
    return 0;
}

static int
init_sound(char *dev, int freq, int fmt, int stereo, int fragsize)
{
    int fd;

    if(!dev || !*dev)
        dev = DEF_DEVICE;
    fd = open(dev, O_WRONLY);
    if(fd == -1)
        return -1;

    if(sound_setfmt(fd, freq, fmt, stereo, fragsize) == -1) {
        close(fd);
        return -1;
    }
    return fd;
}
#endif

audiomodule::audiomodule(char *name) :
    module(&descr_audiomodule, name)
{
    sound_fd = -1;
    sbuf = 0;
}

audiomodule::~audiomodule()
{
    if(sound_fd != -1)
        close(sound_fd);
    if(sbuf)
        delete [] sbuf;
}

int
audiomodule::proc_params()
{
    int frag;

    if(sound_fd != -1)
        close(sound_fd);
    if(sbuf)
        delete [] sbuf;

    frag = param[P_FRAG].i;
#ifndef NOAUDIO
    sound_fd = init_sound(param[P_DEV].s, sampfreq, AFMT_S16_LE, 1, frag);
    if(sound_fd == -1)
        return -1;
#endif

    sound_sbufsize = (4 * (1 << frag)) * sizeof(sbuf[0]);
    sbuf = new short[sound_sbufsize / sizeof(sbuf[0])];
    end_sbuf = &sbuf[sound_sbufsize / sizeof(sbuf[0])];
    spos = sbuf;
    return 0;
}

/* output sample */
void
audiomodule::fasttick()
{
    int left, right;

    left = connect[C_LEFT];
    right = connect[C_RIGHT];

    // saturate
    if(left > 32767)
        left = 32767;
    if(left < -32768)
        left = -32768;
    if(right > 32767)
        right = 32767;
    if(right < -32768)
        right = -32768;

    *spos++ = (short)left;
    *spos++ = (short)right;
    if(spos >= end_sbuf) {
#ifdef NOAUDIO
        // simulate the amount of blocking that the write() would
        // do to keep in time w/ the sampfreq
        int x = sound_sbufsize / (2 * sizeof(sbuf[0]));
        usleep(x * (1000000 / sampfreq));
#else
        write(sound_fd, sbuf, sound_sbufsize);
#endif
        spos = sbuf;
    }
}

