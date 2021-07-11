
#include <gtk/gtk.h>

#include "mod.h"
#include "wire.h"
#include "gui.h"
#include "misc.h"

#include "mymodbox.h"
#include "dragpad.h"
#include "mods.h"
#include "modpeer.h"
#include "wirepeer.h"
#include "rect.h"
#include "util.h"

/* how close we have to click to a wire to consider the wire clicked on */
#define CLOSE_DIST	10

static GdkGC *redgc = 0, *yellowgc = 0;


// XXX maybe this should be in dragpad.cpp?
static void
layout_draw_line(GtkLayout *lay, GdkGC *gc, int x1, int y1, int x2, int y2)
{
    gdk_draw_line(lay->bin_window, gc,
                  x1 - lay->xoffset, y1 - lay->yoffset,
                  x2 - lay->xoffset, y2 - lay->yoffset);
}

static void
layout_clear_area(GtkLayout *lay, int x, int y, int width, int height)
{
    if(lay->bin_window)
        gdk_window_clear_area(lay->bin_window, 
                          x - lay->xoffset, y - lay->yoffset, width, height);
}


void
make_wire(dragpad *p, GtkWidget *w1, int num1, int out1,
                      GtkWidget *w2, int num2, int out2)
{
    extern dragpad *curpad;
    dragpad *save;
    modpeer *p1, *p2;
    struct descr_connect *con1, *con2;
    int idx1, idx2;

    p1 = get_modpeer(w1);
    p2 = get_modpeer(w2);
    if(!p1 || !p2)
        die("make_wire: no peers for widgets!");

    idx1 = connect_idx_fromio(p1->mod->descr->connect, num1, out1);
    idx2 = connect_idx_fromio(p2->mod->descr->connect, num2, out2);
    if(idx1 == -1 || idx2 == -1)
        die("make_wire: bad indices!");

    con1 = &p1->mod->descr->connect[idx1];
    con2 = &p2->mod->descr->connect[idx2];

    save = curpad;
    curpad = p;
    if((con1->type & CONTYPE_OUT) && !(con2->type & CONTYPE_OUT))
        add_wire(p1->mod, idx1, p2->mod, idx2);
    if((con2->type & CONTYPE_OUT) && !(con1->type & CONTYPE_OUT))
        add_wire(p2->mod, idx2, p1->mod, idx1);
    curpad = save;
}

int
exposewires(GtkWidget *widget, GdkEventExpose *exp, GList **wires)
{
    GtkLayout *lay;
    GList *t;
    wirepeer *wire;
    int x1, y1, x2, y2, minx, miny, maxx, maxy;

    lay = GTK_LAYOUT(widget);
    x1 = exp->area.x + lay->xoffset;
    y1 = exp->area.y + lay->yoffset;
    x2 = x1 + exp->area.width;
    y2 = y1 + exp->area.height;

//g_print("expose %d %d to %d %d\n", x1, y1, x2, y2);
    for(t = *wires; t; t = t->next) {
        wire = (wirepeer *)t->data;
        bounding_box(wire->x1, wire->y1, wire->x2, wire->y2,
                     &minx, &miny, &maxx, &maxy);
        if(rect_overlap(x1, y1, x2, y2, minx, miny, maxx, maxy))
            wire->draw();
    }
    return FALSE;
}

static GList **
get_wirelist(dragpad *pad)
{
    GList **wires;

    wires = (GList **)gtk_object_get_data(GTK_OBJECT(pad->layout), "wirelist");
    if(!wires)
        die("no wirelist in wirepeer constructor");
    return wires;
}

void
delete_selected_wires(dragpad *pad)
{
    GList *t, *next;
    wirepeer *w;

    for(t = *get_wirelist(pad); t; t = next) {
        next = t->next;
        w = (wirepeer *)t->data;

        if(w->selectcnt)
            rem_wire(w->mod1, w->idx1, w->mod2, w->idx2);
    }
    return;
}

static void
wirechange(dragpad *pad, dragentry *ent, dragent_cbtype type, void *ctx)
{
    wirepeer *wire = (wirepeer *)ctx;
    int x, y, minx, miny, maxx, maxy;

#ifdef nope
    if(type == SELECTED)
        wire->select(1);

    if(type == UNSELECTED)
        wire->select(0);
#endif

    if(type == MOVED) {
        dragpad_getpos(pad, ent->widget, &x, &y);
        bounding_box(wire->x1, wire->y1, wire->x2, wire->y2,
                     &minx, &miny, &maxx, &maxy);

        // erase the old wire
        layout_clear_area(GTK_LAYOUT(pad->layout), minx, miny,
                          maxx - minx + 1, maxy - miny + 1);

        // XXX move this code into a class member function?
        // XXX we should recompute xoffs and yoffs once in a while,
        //     or somehow detect when a widget changes size (ie.
        //     when the name of a modbox gets changed)
        if(ent->widget == ((modpeer *)wire->mod1->peer)->mb) {
            wire->x1 = x + wire->xoff1;
            wire->y1 = y + wire->yoff1;
            x = wire->x1;
            y = wire->y1;
        }
        if(ent->widget == ((modpeer *)wire->mod2->peer)->mb) {
            wire->x2 = x + wire->xoff2;
            wire->y2 = y + wire->yoff2;
            x = wire->x2;
            y = wire->y2;
        }

        /* grow bounding box if necessary and force refresh */
        minx = MIN(minx, x);
        maxx = MAX(maxx, x);
        miny = MIN(miny, y);
        maxy = MAX(maxy, y);
        dragpad_refresh(pad, minx, miny, maxx + 1, maxy + 1);
    }
}

/*  return the square of distance between and point and a line */
static float
line_pt_dist2(int x1, int y1, int x2, int y2,
             int xp, int yp)
{
    float dxl, dyl, dxp, dyp;
    float r2, dot, len2;

    dxl = x2 - x1;
    dyl = y2 - y1;
    dxp = xp - x1;
    dyp = yp - y1;

    /* dist^2 = r^2 - (dot/len)^2 */
    r2 = dxp * dxp + dyp * dyp;
    dot = dxl * dxp + dyl * dyp;
    len2 = dxl * dxl + dyl * dyl; 
    return r2 - (dot * dot / len2);
}

/* return the closest wire to the given point, or null if nothing is close */
static wirepeer *
wire_at(dragpad *pad, int x, int y)
{
    GList *t;
    wirepeer *wire, *bestwire;
    float dist2, bestdist2;
    int minx, miny, maxx, maxy, diff;

    bestwire = 0;
    bestdist2 = CLOSE_DIST * CLOSE_DIST;
    for(t = *get_wirelist(pad); t; t = t->next) {
        wire = (wirepeer *)t->data;

        /* get a bounding box, and make sure its a reasonable size */
        bounding_box(wire->x1, wire->y1, wire->x2, wire->y2,
                     &minx, &miny, &maxx, &maxy);
        diff = maxx - minx;
        if(diff < 2 * CLOSE_DIST) {
            minx -= (2 * CLOSE_DIST - diff) >> 1;
            maxx += (2 * CLOSE_DIST - diff) >> 1;
        }
        diff = maxy - miny;
        if(diff < 2 * CLOSE_DIST) {
            miny -= (2 * CLOSE_DIST - diff) >> 1;
            maxy += (2 * CLOSE_DIST - diff) >> 1;
        }

        /* skip this wire if point isnt in the box */
        if(x < minx || x > maxx || y < miny || y > maxy)
            continue;

        dist2 = line_pt_dist2(wire->x1, wire->y1, wire->x2, wire->y2, x, y);
        if(dist2 < bestdist2) {
            bestdist2 = dist2;
            bestwire = wire;
        }
    }
    return bestwire;
}

// XXX this needs to be fixed to deal w/ the selection ref
// counting we are doing
static void
wire_select(dragpad *pad, wirepeer *wire, int clear)
{
    GList *t;
    wirepeer *w2;

    if(clear) {
        /* unselect all other wires */
        for(t = *get_wirelist(pad); t; t = t->next) {
            w2 = (wirepeer *)t->data;
            w2->select(0);
        }
    }

    if(wire)
        wire->select(1);
}

int
presswires(GtkWidget *widget, GdkEventButton *event, dragpad *pad)
{
    GtkLayout *lay;
    GdkModifierType mods;
    int x, y;

    if(event->type == GDK_BUTTON_PRESS && event->button == 3) {
        lay = GTK_LAYOUT(pad->layout);
        gdk_window_get_pointer(pad->layout->window, &x, &y, &mods);
        x += lay->xoffset;
        y += lay->yoffset;

        wire_select(pad, wire_at(pad, x, y), (event->state != 1));
    }
    return FALSE;
}

wirepeer::wirepeer(class module *m1, int id1, class module *m2, int id2,
                   dragpad *p)
{
    GList **wires;
    modpeer *p1, *p2;

    mod1 = m1;
    mod2 = m2;
    idx1 = id1;
    idx2 = id2;

    pad = p;
    selectcnt = 0;

    wires = get_wirelist(pad);

    p1 = (modpeer *)mod1->peer;
    p2 = (modpeer *)mod2->peer;
    if(!p1 || !p2)
        die("wirepeer: wire added between modules with no peers!");

    // convert to modbox indicies
    if(connect_io_fromidx(mod1->descr->connect, id1, &num1, &out1) == -1 ||
       connect_io_fromidx(mod2->descr->connect, id2, &num2, &out2) == -1)
        die("wirepeer: bad indices");

    // get positions of associated widgets and modbox terminals
    if(mymodbox_gettermpos(MYMODBOX(p1->mb), num1, out1, 
                           &xoff1, &yoff1) == -1 ||
       mymodbox_gettermpos(MYMODBOX(p2->mb), num2, out2, 
                           &xoff2, &yoff2) == -1 ||
       dragpad_getpos(pad, p1->mb, &x1, &y1) == -1 ||
       dragpad_getpos(pad, p2->mb, &x2, &y2) == -1)
        die("wirepeer: positions not found");

    x1 += xoff1;
    y1 += yoff1;
    x2 += xoff2;
    y2 += yoff2;

    *wires = g_list_prepend(*wires, this);
    cb1 = dragpad_addcallback(pad, p1->mb, wirechange, this);
    cb2 = dragpad_addcallback(pad, p2->mb, wirechange, this);

    draw();
}

wirepeer::~wirepeer()
{
    GList **wires;
    modpeer *p1, *p2;

    p1 = (modpeer *)mod1->peer;
    p2 = (modpeer *)mod2->peer;
    if(!p1 || !p2)
        die("wirepeer: wire added between modules with no peers!");
    dragpad_remcallback(pad, p1->mb, cb1);
    dragpad_remcallback(pad, p2->mb, cb2);

    // remove from list
    wires = (GList **)gtk_object_get_data(GTK_OBJECT(pad->layout), "wirelist");
    if(!wires)
        die("no wirelist in wirepeer constructor");
    *wires = g_list_remove(*wires, this);

    undraw();    
}

void
wirepeer::select(int on)
{
    selectcnt = on;
#ifdef nope
    if(on)
        selectcnt ++;
    else
        selectcnt --;
#endif
    draw();
}

void
wirepeer::draw()
{
    if(pad->layout->window) {
        if(!redgc)
            redgc = alloc_color_gc(pad->layout, 0xffff, 0, 0);
        if(!yellowgc)
            yellowgc = alloc_color_gc(pad->layout, 0xffff, 0xffff, 0);

        layout_draw_line(GTK_LAYOUT(pad->layout), 
                         selectcnt ? yellowgc : redgc, x1, y1, x2, y2);
    }
}

void
wirepeer::undraw()
{
    int minx, miny, maxx, maxy;

    bounding_box(x1, y1, x2, y2, &minx, &miny, &maxx, &maxy);
    layout_clear_area(GTK_LAYOUT(pad->layout), minx, miny,
                      maxx - minx + 1, maxy - miny + 1);
    dragpad_refresh(pad, minx, miny, maxx + 1, maxy + 1);
}

void
wirepeer::notify(guipeer_notification type)
{
    if(type == DELETED) {
        delete this;
        return;
    }
}

