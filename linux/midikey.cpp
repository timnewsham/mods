
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "mod.h"
#include "misc.h"

#include "midikey.h"


static struct descr_param P[] = {
    { 0 }
};
static struct descr_connect C[] = {
#define C_NOTE 0
    { "note", "Note", CONTYPE_OUT | CONTYPE_SLOW },
#define C_VEL 1
    { "vel", "Velocity", CONTYPE_OUT | CONTYPE_SLOW},
    { 0 }
};

static module *
B(char *name)
{
    return new midikeymodule(name);
}

struct descr_mod descr_midikeymodule = {"midikey", C, P, SLOW, B };

static void
add_note(struct midinotes **headp, int note, int vel)
{
    struct midinotes *t;

    t = new midinotes;
    t->next = *headp;
    *headp = t;

    t->note = note;
    t->vel = vel;
    return;
}

static void
rem_note(struct midinotes **headp, int note)
{
    struct midinotes *t, **prevp;

    prevp = headp;
    for(t = *headp; t; t = t->next) {
        if(t->note == note)
            break;
        prevp = &t->next;
    }

    if(t) {
        *prevp = t->next;
        delete t;
    }
    return;
}

static int
midi_read(int fd)
{
    struct timeval timeo;
    fd_set rd;
    int n;
    unsigned char c;

    if(fd >= FD_SETSIZE)
        die("midi fd too large");

    /* poll for input, then read - ugh, I wish I had FIONREAD */
    FD_ZERO(&rd);
    FD_SET(fd, &rd);
    timeo.tv_sec = 0;
    timeo.tv_usec = 0;
    n = select(fd + 1, &rd, 0, 0, &timeo);
    if(n == 1 && FD_ISSET(fd, &rd)) {
        if(read(fd, &c, 1) != 1)
            return -1;
        return c;
    }
    return -2;
}

midikeymodule::midikeymodule(char *name) :
    module(&descr_midikeymodule, name)
{
    notelist = 0;

    // XXX this should be a param
    midifd = open("/dev/midi", O_RDONLY);
    if(midifd == -1)
        die("open midi failed");
}

midikeymodule::~midikeymodule()
{
    if(midifd != -1)
        close(midifd);

    /* destroy the notelist */
    while(notelist)
        rem_note(&notelist, notelist->note);
}

void
midikeymodule::slowtick()
{
    int x;

    while((x = midi_read(midifd)) >= 0) {
        if(x >= 0xf8)
            continue;

        if(x & 0x80) {
            mcmd = x;
            if(mcmd >= 0x80 && mcmd <= 0x9f) {
                mcmd &= 0xf0;
                note = -1;
            }
        } else {
            if(mcmd == 0x80 || mcmd == 0x90) {
                if(note == -1)
                     note = x;
                else
                    proc_cmd(mcmd, note, x);
            }
        }
    }
}

void
midikeymodule::proc_cmd(int cmd, int note, int vel)
{
    if(cmd == 0x80 || (cmd == 0x90 && vel == 0))
        rem_note(&notelist, note);
    else if(cmd == 0x90)
        add_note(&notelist, note, vel);

    if(notelist) {
        connect[C_NOTE] = (notelist->note - 36) * 100;
        connect[C_VEL] = notelist->vel;
    } else {
        connect[C_VEL] = 0;
    }
}

