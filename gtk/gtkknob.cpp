
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "mod.h"

#include "myknob.h"
#include "gtkknob.h"

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
    return new gtkknobmodule(name);
}

struct descr_mod descr_gtkknobmodule = {"knob", C, P, SLOW, B };

gtkknobmodule::gtkknobmodule(char *name) :
    module(&descr_gtkknobmodule, name)
{
    knobname = 0;
    widget = 0;
    min = 0;
    max = 0;
    val = 0;
}

gtkknobmodule::~gtkknobmodule()
{
    if(knobname)
        free(knobname);
}

int
gtkknobmodule::proc_params()
{
    if(knobname)
        free(knobname);
    knobname = strdup(param[P_NAME].s);
    min = param[P_MIN].i;
    max = param[P_MAX].i;
    if(min >= max)
        return -1;
    val = min;
    set_steps();
    return 0;
}

static int
conv_percent(double perc, int min, int max)
{
    return (int) (0.5 + min + (max - min) * perc / 100.0);
}

void
gtkknobmodule::slowtick()
{
    if(widget)
        val = conv_percent(widget->adjust->value, min, max);
    connect[C_VAL] = val;
}

void
gtkknobmodule::set(int v)
{
    if(v < min)
        v = min;
    else if(v > max)
        v = max;

    if(val != v) {
        val = v;
        if(widget)
            myknob_set(widget, 100 * (val - min) / (max - min));
    }
}

char *
gtkknobmodule::get_state()
{
    static char buf[15];

    sprintf(buf, "%d", val);
    return buf;
}

int
gtkknobmodule::set_state(char *state)
{
    char *end;
    int val;

    val = strtoul(state, &end, 10);
    if(end == state || *end != '\0')
        return -1;
    set(val);
    return 0;
}

void
gtkknobmodule::set_steps(void)
{
    int steps;

    steps = max - min + 1;
    if(widget && steps <= 10)
        myknob_set_steps(widget, steps);
}

void 
gtkknobmodule::set_widget(MyKnob *p)
{
    widget = p;
    set_steps();
}

