
#include <stdlib.h>
#include <curses.h>

#include "mod.h"
#include "gui.h"
#include "misc.h"

#include "cursgui.h"
#include "curskey.h"

// we dont know how long the key is pressed for, so we fake it... # of ticks
#define DURATION 50

static int exists = 0;

static char notes[] = { 
    'z', 's', 'x', 'd', 'c', 'v', 'g', 'b', 'h', 'n',
    'j', 'm', ',', 'l', '.', ';', '/', 0 };
static char notes2[] = {
    'q', '2', 'w', '3', 'e', 'r', '5', 't', '6', 'y', 
    '7', 'u', 'i', '9', 'o', '0', 'p', '[', '=', ']', 0 };

static struct descr_param P[] = {
    { 0 }
};
static struct descr_connect C[] = {
#define C_NOTE 0
    { "note", "Note", CONTYPE_OUT | CONTYPE_SLOW },
#define C_GATE 1
    { "gate", "Gate", CONTYPE_OUT | CONTYPE_SLOW},
    { 0 }
};

static module *
B(char *name)
{
    return new curskeymodule(name);
}

struct descr_mod descr_curskeymodule = {"key", C, P, SLOW, B };

curskeymodule::curskeymodule(char *name) :
    module(&descr_curskeymodule, name)
{
    if(exists++)
        die("only one instance of curskeymodule allowed");

    onticks = 0;
    setoctave(2);
}

curskeymodule::~curskeymodule()
{
    die("destruction of curskeymodule is not allowed");
}

void
curskeymodule::setoctave(int val)
{
    octave = val;
    printat(ROW_OCTAVE, 30, "Octave: %d  ", octave);
}

void
curskeymodule::slowtick()
{
   // countdown duration of previous playing note
   if(onticks)
       if(--onticks == 0)
           connect[C_GATE] = 0;
}

char *
curskeymodule::get_state()
{
    static char buf[15];

    sprintf(buf, "%d", octave);
    return buf;
}

int
curskeymodule::set_state(char *state)
{
    char *end;
    int v;

    v = strtoul(state, &end, 10);
    if(end == state)
        return -1;
    setoctave(v);
    return 0;
}

void
curskeymodule::proc_key(int ch)
{
    int i;

    if(ch == '+' && octave < 5)
        setoctave(octave + 1);
    if(ch == '-' && octave > 0)
        setoctave(octave - 1);

    for(i = 0; notes[i]; i++) {
        if(ch == notes[i]) {
            connect[C_NOTE] = (i + octave * 12) * 100;
            connect[C_GATE] = 1;
            onticks = DURATION;
        }
    }
    for(i = 0; notes2[i]; i++) {
        if(ch == notes2[i]) {
            connect[C_NOTE] = (i + (octave + 1) * 12) * 100;
            connect[C_GATE] = 1;
            onticks = DURATION;
        }
    }
}

