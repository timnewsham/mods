
#include <curses.h>
#include <string.h>
#include <stdlib.h>

#include "mod.h"
#include "gui.h"
#include "misc.h"

#include "cursgui.h"
#include "cursknob.h"

#define METERSIZE 50
#define KNOBS_VISIBLE 6

#define ROW_KNOB 0

struct knoblist {
    struct knoblist *next;
    struct knoblist *prev;
    cursknobmodule *knob;
};

static struct descr_param P[] = {
#define P_NAME 0
    {"name", 's'},
#define P_MIN 1
    {"min",  'i'},
#define P_MAX 2
    {"max",  'i'},
    {0}
};
static struct descr_connect C[] = {
#define C_VAL 0
    {"val", "Knob Value", CONTYPE_OUT | CONTYPE_SLOW},
    {0}
};

static module *
B(char *name)
{
    return new cursknobmodule(name);
}

struct descr_mod descr_cursknobmodule = {"knob", C, P, SLOW, B };


static struct knoblist *knobshead = 0, *knobstail = 0;
static struct knoblist *curknob;

static void
redraw()
{
    struct knoblist *t;
    int i;

    // show up to KNOBS_VISIBLE knobs, with curknob as centered as is
    // possible
    t = curknob;
    for(i = 0; i < (KNOBS_VISIBLE / 2) && t && t->next; i++)
        t = t->next;
    for(i = 0; i < (KNOBS_VISIBLE - 1) && t && t->prev; i++)
        t = t->prev;
    for(i = 0; i < KNOBS_VISIBLE; i++) {
        if(t) {
            if(t == curknob) {
                attron(A_BOLD);
                mvaddstr(i + ROW_KNOB, 0, "* ");
            } else
                mvaddstr(i + ROW_KNOB, 0, "  ");
            t->knob->draw();
            attroff(A_BOLD);
            t = t->next;
        } else {
            move(i + ROW_KNOB, 0);
            clrtoeol();
        }
    }
    refresh();
}

void
move_curknob(int dir)
{
    if(dir == 1 && curknob && curknob->next)
        curknob = curknob->next;
    if(dir == -1 && curknob && curknob->prev)
        curknob = curknob->prev;
    redraw();
}

void
curknob_adjust(int dir)
{
    if(curknob)
        curknob->knob->adjusttick(dir);
    redraw();
}

static void
add_knob(cursknobmodule *x)
{
    knoblist *n = new knoblist;

    n->knob = x;
    if(!knobshead)
        knobshead = n;
    n->prev = knobstail;
    n->next = 0;
    if(knobstail)
        knobstail->next = n;
    knobstail = n;

    if(!curknob)
        curknob = n;
}

static void
rem_knob(cursknobmodule *x)
{
    knoblist *t;

    // find the knob
    for(t = knobshead; t; t = t->next) {
        if(t->knob == x)
            break;
    }
    if(!t) 
        die("rem_knob: removing unknown knob from list!");

    // if it was the selected one, select next or previous one
    if(t == curknob) {
        curknob = t->next;
        if(!curknob)
            curknob = t->prev;
    }
    redraw();

    if(t->prev)
        t->prev->next = t->next;
    else
        knobshead = t->next;
    if(t->next)
        t->next->prev = t->prev;
    else
        knobstail = t->prev;
    delete t;
}

cursknobmodule::cursknobmodule(char *name) :
    module(&descr_cursknobmodule, name)
{
    knobname = 0;
    min = 0;
    max = 0;
    val = 0;
    add_knob(this);
}

cursknobmodule::~cursknobmodule()
{
    free(knobname);
    rem_knob(this);
}

int
cursknobmodule::proc_params()
{
    if(knobname)
        free(knobname);
    knobname = strdup(param[P_NAME].s);
    min = param[P_MIN].i;
    max = param[P_MAX].i;
    if(min >= max)
        return -1;
    val = min;
    redraw();
    return 0;
}

void
cursknobmodule::slowtick()
{
    connect[C_VAL] = val;
}

void
cursknobmodule::adjusttick(int dir)
{
    int tick;

    tick = (int)(0.99 + ((float)max - min) / KNOBS_WIDTH);
    if(!tick)
        tick = 1;
    if(dir == -1)
        tick = -tick;
    adjust(tick);
}

void
cursknobmodule::adjust(int diff)
{
    set(val + diff);
}

void
cursknobmodule::draw()
{
    char buf[80], c;
    int pos, i;

    clrtoeol();
    sprintf(buf, "%12.12s : [%*.*s] : %d", knobname, KNOBS_WIDTH, KNOBS_WIDTH,
            "", val);
    pos = KNOBS_WIDTH * (val - min) / (max - min);
    if(pos == KNOBS_WIDTH)
        pos = KNOBS_WIDTH - 1;
    for(i = 0; i < KNOBS_WIDTH; i++) {
        if(i < pos)
            c = '=';
        else if(i == pos)
            c = '>';
        else
            c = ' ';
        buf[i+16] = c;
    }
    addstr(buf);
}

void
cursknobmodule::set(int v)
{
    val = v;
    if(val < min)
        val = min;
    else if(val > max)
        val = max;
}

char *
cursknobmodule::get_state()
{
    static char buf[15];

    sprintf(buf, "%d", val);
    return buf;
}

int
cursknobmodule::set_state(char *state)
{
    char *end;
    int val;

    val = strtoul(state, &end, 10);
    if(end == state || *end != '\0')
        return -1;
    set(val);

    // XXX do this every time?  ugh.  should have a general update
    // after all modules are updated
    redraw();
    return 0;
}

