
#include <gtk/gtk.h>

#include "mod.h"
#include "misc.h"

#include "gtkoscope.h"
#include "dragpad.h"
#include "util.h"

#define O_WIDTH		100	/* geometry */
#define O_HEIGHT	50
#define STICKS		10	/* how often we draw */
#define TTICKS		10	/* how often we force a trigger */

extern dragpad *controlpad;

static GdkGC *bluegc = 0;

static struct descr_param P[] = {
#define P_NAME 0
    { "name", 's'},
#define P_MAX 1
    { "max", 'i'},
#define P_SKIP 2
    { "skip", 'i'},
#define P_TRIG 3
    { "trig", 'i'},
    { 0 }
};
static struct descr_connect C[] = {
#define C_IN 0
    { "in", "Input", CONTYPE_IN | CONTYPE_FAST },
    { 0 }
};

static module *
B(char *name)
{
    return new gtkoscopemodule(name);
}

struct descr_mod descr_gtkoscopemodule = {"oscope", C, P, FAST, B };

static void
oscope_draw(gtkoscopemodule *o)
{
    GdkGC *fg, *bg;
    GdkDrawable *d;
    float scale;
    int i, val, y, lasty;

    d = o->draw->window;
    if(d) {
        if(!bluegc)
            bluegc = alloc_color_gc(o->draw, 0, 0, 0xffff);
        bg = o->draw->style->bg_gc[GTK_WIDGET_STATE(o->draw)];
        fg = o->draw->style->fg_gc[GTK_WIDGET_STATE(o->draw)];
        gdk_draw_rectangle(d, bg, TRUE, 0, 0, O_WIDTH, O_HEIGHT);
        gdk_draw_rectangle(d, fg, FALSE, 0, 0, O_WIDTH - 1, O_HEIGHT - 1);
        gdk_draw_line(d, fg, 0, O_HEIGHT / 2, O_WIDTH, O_HEIGHT / 2);

        scale = O_HEIGHT / (2.0 * o->max);
        lasty = O_HEIGHT / 2;
        for(i = 0; i < O_WIDTH; i++) {
            val = o->buf[i];
            if(val > o->max)
                val = o->max;
            if(val < -o->max)
                val = -o->max;
            y = (O_HEIGHT / 2) - (int)(val * scale);
            gdk_draw_line(d, bluegc, i - 1, lasty, i, y);
            lasty = y;
        }
    }
    return;
}

int
oscope_expose(GtkWidget *widget, GdkEventExpose *exp, gtkoscopemodule *mod)
{
    oscope_draw(mod);
    return FALSE;
}

gtkoscopemodule::gtkoscopemodule(char *name) :
    module(&descr_gtkoscopemodule, name)
{
    trig = 1000;
    skip = 1000;
    max = 1000;
    pos = -1;
    stick = STICKS;
    ftick = skip;

    vb = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vb);

    draw = gtk_drawing_area_new();
    gtk_widget_show(draw);
    gtk_drawing_area_size(GTK_DRAWING_AREA(draw), O_WIDTH, O_HEIGHT);
    gtk_widget_add_events(draw, GDK_BUTTON_PRESS_MASK |
                                GDK_BUTTON_RELEASE_MASK |
                                GDK_POINTER_MOTION_MASK |
                                GDK_POINTER_MOTION_HINT_MASK);
    gtk_signal_connect(GTK_OBJECT(draw), "expose_event",
                       GTK_SIGNAL_FUNC(oscope_expose), this);

    label = gtk_label_new("unnamed");
    gtk_widget_show(label);

    gtk_box_pack_start(GTK_BOX(vb), draw, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vb), label, FALSE, FALSE, 0);
    dragpad_add(controlpad, vb, -1, -1, DE_FLAG_DRAG | DE_FLAG_SELECT);
}

gtkoscopemodule::~gtkoscopemodule()
{
    // XXX remove from pad and toss the draw widget
}

void
gtkoscopemodule::slowtick()
{
    if(pos > 0 && stick-- <= 0) {
        oscope_draw(this);
        stick = STICKS;
        if(pos == O_WIDTH)
            pos = -1;
    }

    // periodically force a retrigger
    if(pos <= 0) {
        if(ttick-- <= 0) {
            pos = 0;
            buf[pos++] = 0;
            ttick = TTICKS;
        }
    } else {
        ttick = TTICKS;
    }
}

void
gtkoscopemodule::fasttick()
{
    int in;

    if(pos < O_WIDTH && ftick-- <= 0) {
        ftick = skip; 
        in = connect[C_IN];

        if(pos < 0) {
            if(in < 0)
                pos = 0;
        } else if(pos == 0) {
            if(in > trig)
                buf[pos++] = in;
        } else {
            buf[pos++] = in;
        }
    }
}

int
gtkoscopemodule::proc_params()
{
    name = param[P_NAME].s;
    max = param[P_MAX].i;
    trig = param[P_TRIG].i;
    skip = param[P_SKIP].i;
    pos = 0;

    gtk_label_set_text(GTK_LABEL(label), name);
    return 0;
}

