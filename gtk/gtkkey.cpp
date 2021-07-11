
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "mod.h"
#include "misc.h"

#include "gtkkey.h"

struct _keydata {
    int code;
    int note;
};

#define TOPKEY		((struct _keydata *)keylist->data)

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
    return new gtkkeymodule(name);
}

struct descr_mod descr_gtkkeymodule = {"key", C, P, SLOW, B };

gtkkeymodule::gtkkeymodule(char *name) :
    module(&descr_gtkkeymodule, name)
{
    keylist = 0;
    setoctave(2);
}

gtkkeymodule::~gtkkeymodule()
{
    GList *t;

    for(t = keylist; t; t = t->next)
        delete t->data;
    g_list_free(keylist);
}

void
gtkkeymodule::setoctave(int val)
{
    octave = val;
}

char *
gtkkeymodule::get_state()
{
    static char buf[15];

    sprintf(buf, "%d", octave);
    return buf;
}

int
gtkkeymodule::set_state(char *state)
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
gtkkeymodule::play_note(int code, int note)
{
    struct _keydata *x;

    connect[C_NOTE] = note;
    connect[C_GATE] = 1;

    x = new struct _keydata;
    x->code = code;
    x->note = note;
    keylist = g_list_prepend(keylist, x);
}

void
gtkkeymodule::unplay_note(int code)
{
    GList *t;
    struct _keydata *x;

    if(code < 0) {
        /* remove all keys */
        for(t = keylist; t; t = t->next) {
            x = (struct _keydata *)t->data;
            delete x;
        }
        g_list_free(keylist);
        keylist = 0;
    } else {
        /* find the key on our stack */
        for(t = keylist; t; t = t->next) {
            x = (struct _keydata *)t->data;
            if(x->code == code)
                break;
        }

        if(t) {
            /* remove the key from the stack */
            keylist = g_list_remove(keylist, x);
            delete x;
        }
    }

    /* update note (although it may still be the same) */
    if(keylist)
        connect[C_NOTE] = TOPKEY->note;
    else
        connect[C_GATE] = 0;
}

void
gtkkeymodule::proc_key(int code, char ch, int press)
{
    int i;

    if(!press) {
        unplay_note(code);
        return;
    }

    if(ch == '+' && octave < 5)
        setoctave(octave + 1);
    else if(ch == '-' && octave > 0)
        setoctave(octave - 1);
    else {
        for(i = 0; notes[i]; i++) {
            if(ch == notes[i])
                play_note(code, (i + octave * 12) * 100);
        }
        for(i = 0; notes2[i]; i++) {
            if(ch == notes2[i])
                play_note(code, (i + (octave + 1) * 12) * 100);
        }
    }
}

