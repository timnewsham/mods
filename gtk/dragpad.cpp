
#include <gtk/gtk.h>

#include "dragpad.h"
#include "rect.h"


#define RECOMPUTE_TIMEOUT	(5 * 1000)
#define XTRASPACE		10

static int cbid = 1;

struct cbinfo {
    int id;
    dragent_callback cb;
    void *ctx;
};

/* return true if a refers to the same widget as b */
static int
samewidget(gconstpointer a, gconstpointer b)
{
    dragentry *xa = (dragentry *)a;

    if(xa->widget == (GtkWidget *)b)
        return 0;
    return -1;
}

dragentry *
dragpad_getent(dragpad *pad, GtkWidget *widget)
{
    GList *x;

    x = g_list_find_custom(pad->widgets, widget, samewidget);
    return x ? (dragentry *)x->data : 0;
}

/* run all callbacks with status change */
static void
dragent_notify(dragpad *pad, dragentry *ent, dragent_cbtype type)
{
    GList *t;
    struct cbinfo *cbinfo;

    for(t = ent->callbacks; t; t = t->next) {
        cbinfo = (struct cbinfo *)t->data;
        cbinfo->cb(pad, ent, type, cbinfo->ctx);
    }
}

static int
recompute_size(gpointer data)
{
    GtkRequisition req;
    dragpad *pad;
    GList *t;
    dragentry *ent;
    int maxx, maxy, a;

    pad = (dragpad *)data;
    maxx = maxy = 0;
    for(t = pad->widgets; t; t = t->next) {
        ent = (dragentry *)t->data;
        gtk_widget_size_request(ent->widget, &req);

        a = ent->x + req.width;
        if(a > maxx)
            maxx = a;
        a = ent->y + req.height;
        if(a > maxy)
            maxy = a;
    }

    if(pad->width != maxx || pad->height != maxy) {
        pad->width = maxx;
        pad->height = maxy;
        gtk_layout_set_size(GTK_LAYOUT(pad->layout), pad->width, pad->height);
    }

    return TRUE;
}

static void
adjsize(dragpad *pad, GtkWidget *widget, int x, int y)
{
    GtkRequisition req;
    int changed;

    changed = 0;
    gtk_widget_size_request(widget, &req);
    x += req.width;
    y += req.height;

    if(pad->width < x) {
        pad->width = x;
        changed = 1;
    }
    if(pad->height < y) {
        pad->height = y;
        changed = 1;
    }
    if(changed)
        gtk_layout_set_size(GTK_LAYOUT(pad->layout), pad->width, pad->height);
}

static void
select(dragpad *pad, dragentry *ent, int clear)
{
    GList *t;
    dragentry *ent2;

    if(clear) {
        /* unselect anything else */
        for(t = pad->selected; t; t = t->next) {
            ent2 = (dragentry *)t->data;

            if(ent2 != ent) {
                FLAG_CLR(ent2->flags, DE_FLAG_SELECTED);
                gtk_widget_set_state(ent2->widget, GTK_STATE_NORMAL);
                gtk_widget_queue_draw(ent2->widget);
                dragent_notify(pad, ent2, UNSELECTED);
            }
        }
        g_list_free(pad->selected);
        pad->selected = 0;
    }

    if(ent) {
        if(!g_list_find(pad->selected, ent)) {
            if(!FLAG_ISSET(ent->flags, DE_FLAG_SELECTED)) {
                FLAG_SET(ent->flags, DE_FLAG_SELECTED);
                gtk_widget_set_state(ent->widget, GTK_STATE_ACTIVE);
                gtk_widget_queue_draw(ent->widget);
                dragent_notify(pad, ent, SELECTED);
            }
            pad->selected = g_list_prepend(pad->selected, ent);
        }
    }
    return;
}

static int
press(GtkWidget *widget, GdkEventButton *event, dragpad *pad)
{
    GtkLayout *lay;
    GdkModifierType mods;
    dragentry *ent;

    if(event->type == GDK_BUTTON_PRESS && event->button == 3) {
        ent = dragpad_getent(pad, widget);
        if(!ent)
            g_error("couldn't find a pressed widget!");

        if(FLAG_ISSET(ent->flags, DE_FLAG_SELECT))
            select(pad, ent, (event->state != 1));

        /* set up for dragging */
        lay = GTK_LAYOUT(pad->layout);
        pad->drag_widget = widget;
        gdk_window_get_pointer(pad->layout->window, 
                               &pad->drag_x, &pad->drag_y, &mods);
        pad->drag_x += lay->xoffset;
        pad->drag_y += lay->yoffset;
        pad->drag_xslide = 0;
        pad->drag_yslide = 0;
        gtk_grab_add(widget);
    }

    if(event->type == GDK_BUTTON_RELEASE && event->button == 3) {
        if(pad->drag_widget) {
            gtk_grab_remove(pad->drag_widget);
            pad->drag_widget = 0;
        }
    }
    return FALSE;
}

int
dragpad_overlap(dragpad *x, dragentry *ent, int xpos, int ypos, 
                int skipsel, int strict)
{
    GtkRequisition req;
    GList *t;
    dragentry *tent;
    int max_x1, max_x2, max_y1, max_y2;

    gtk_widget_size_request(ent->widget, &req);
    max_x2 = xpos + req.width;
    max_y2 = ypos + req.height;
    for(t = x->widgets; t; t = t->next) {
        tent = (dragentry *)t->data;
        if(tent == ent)
            continue;
        if(!strict && FLAG_ISSET(tent->flags, DE_FLAG_OVERLAP))
            continue;
        if(skipsel && FLAG_ISSET(tent->flags, DE_FLAG_SELECTED))
            continue;

        gtk_widget_size_request(tent->widget, &req);
        max_x1 = tent->x + req.width;
        max_y1 = tent->y + req.height;

        if(!rect_overlap(tent->x, tent->y, max_x1, max_y1,
                         xpos, ypos, max_x2, max_y2))
            continue;
        return 1;
    }
    return 0;
}

static int
selection_move(dragpad *pad, int dx, int dy)
{
    GList *t;
    dragentry *sel;

    if(!dx && !dy)
        return 0;

    /* fail if there is an illegal overlap */
    for(t = pad->selected; t; t = t->next) {
        sel = (dragentry *)t->data;
        if(FLAG_ISSET(sel->flags, DE_FLAG_OVERLAP))
            continue;
        if(dragpad_overlap(pad, sel, sel->x + dx, sel->y + dy, 1, 0))
            return -1;
    }

    /* move all selected items */
    for(t = pad->selected; t; t = t->next) {
        sel = (dragentry *)t->data;

        sel->x += dx;
        sel->y += dy;
        gtk_layout_move(GTK_LAYOUT(pad->layout), sel->widget, sel->x, sel->y);
        adjsize(pad, sel->widget, sel->x, sel->y);
        dragent_notify(pad, sel, MOVED);
    }
    pad->drag_x += dx;
    pad->drag_y += dy;
    return 0;
}

static int
motion(GtkWidget *widget, GdkEventMotion *event, dragpad *pad)
{
    GtkLayout *lay;
    GdkModifierType mods;
    int x, y, dx, dy;

    if(pad->drag_widget) {
        /* get coordinates of widget in the layout */
        lay = GTK_LAYOUT(pad->layout);
        gdk_window_get_pointer(GTK_WIDGET(lay)->window, &x, &y, &mods);
        x += lay->xoffset;
        y += lay->yoffset;

        if(mods & GDK_BUTTON3_MASK) {
            dx = x - pad->drag_x - pad->drag_xslide;
            dy = y - pad->drag_y - pad->drag_yslide;
            if(selection_move(pad, dx, dy) == -1) {
                /* jump to unslid position or slide more */
                if(selection_move(pad, dx + pad->drag_xslide, 
                                       dy + pad->drag_yslide) == -1) {
                    pad->drag_xslide += dx;
                    pad->drag_yslide += dy;
                } else {
                    pad->drag_xslide = 0;
                    pad->drag_yslide = 0;
                }
            }
        }
    }
    return FALSE;
}

dragpad *
dragpad_new()
{
    GtkAdjustment *vadj, *hadj;
    dragpad *x;

    x = new dragpad;
    x->widgets = 0;
    x->selected = 0;
    x->width = 300;
    x->height = 200;
    x->drag_widget = 0;

    /* make scrolled window with a layout inside of it */
    x->scrollwin = gtk_scrolled_window_new(0, 0);
    gtk_widget_show(x->scrollwin);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(x->scrollwin),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(x->scrollwin));
    hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(x->scrollwin));
    x->layout = gtk_layout_new(hadj, vadj);
    gtk_layout_set_size(GTK_LAYOUT(x->layout), x->width, x->height);
    gtk_widget_show(x->layout);
    gtk_container_add(GTK_CONTAINER(x->scrollwin), x->layout);

    x->timeout = gtk_timeout_add(RECOMPUTE_TIMEOUT, recompute_size, x);

    return x;
}


/* 
 * this is an expensive operation, but it doesnt happen often.
 * The basic idea is to take vertical lines at the right most edge
 * of each widget, and the horizontal lines at the bottom most edge
 * and check if we can place a widget at any of the intersections,
 * and return the "best" one.
 *
 * best = closest to the middle of the visible part of the screen
 */
static void
autoplace(dragpad *pad, dragentry *ent)
{
    GtkRequisition req;
    GtkLayout *lay;
    GList *vlist, *hlist, *v, *h, *t;
    dragentry *ent2;
    int x, y, dist2, bestdist2, bestx, besty;
    int targx, targy;

    lay = GTK_LAYOUT(pad->layout);
    targx = lay->xoffset;
    if(pad->layout->allocation.width > 0)
        targx += pad->layout->allocation.width / 2;
    targy = lay->yoffset;
    if(pad->layout->allocation.height > 0)
        targy += pad->layout->allocation.height / 2;

    /* build lists */
    vlist = g_list_prepend(0, (gpointer)(0 + XTRASPACE));
    hlist = g_list_prepend(0, (gpointer)(0 + XTRASPACE));
    for(t = pad->widgets; t; t = t->next) {
        ent2 = (dragentry *)t->data;
        gtk_widget_size_request(ent2->widget, &req);
        vlist = g_list_prepend(vlist, 
            (gpointer)(ent2->x + req.width + XTRASPACE));
        hlist = g_list_prepend(hlist, 
            (gpointer)(ent2->y + req.height + XTRASPACE));
    }

    /* check intersection points */
    bestdist2 = G_MAXINT;
    bestx = XTRASPACE;
    besty = XTRASPACE;
    for(v = vlist; v; v = v->next) {
        for(h = hlist; h; h = h->next) {
            x = (int)v->data;
            y = (int)h->data;
            dist2 = (x - targx) * (x - targx) + (y - targy) * (y - targy);
            if(dist2 < bestdist2 && !dragpad_overlap(pad, ent, x, y, 0, 1)) {
                bestdist2 = dist2;
                bestx = x;
                besty = y;
            }
        }
    }

    g_list_free(vlist);
    g_list_free(hlist);

    ent->x = bestx;
    ent->y = besty;
}

void
dragpad_add(dragpad *x, GtkWidget *widget, int xpos, int ypos, int flags)
{
    dragentry *ent;

    FLAG_CLR(flags, DE_FLAG_SELECTED);

    ent = new dragentry;
    ent->widget = widget;
    ent->callbacks = 0;
    ent->flags = flags;
    if(xpos == -1 || ypos == -1) {
        autoplace(x, ent);
    } else {
        ent->x = xpos;
        ent->y = ypos;
    }
    x->widgets = g_list_prepend(x->widgets, ent);

    gtk_layout_put(GTK_LAYOUT(x->layout), widget, ent->x, ent->y);
    adjsize(x, widget, ent->x, ent->y);

    if(FLAG_ISSET(flags, DE_FLAG_SELECT)) {
        gtk_signal_connect(GTK_OBJECT(widget), "button_press_event",
                           GTK_SIGNAL_FUNC(press), x);
        gtk_signal_connect(GTK_OBJECT(widget), "button_release_event",
                           GTK_SIGNAL_FUNC(press), x);

        if(FLAG_ISSET(flags, DE_FLAG_DRAG))
            gtk_signal_connect(GTK_OBJECT(widget), "motion_notify_event",
                               GTK_SIGNAL_FUNC(motion), x);
    }
}

int
dragpad_remove(dragpad *pad, GtkWidget *widget)
{
    GList *t;
    dragentry *ent;

    ent = dragpad_getent(pad, widget);
    if(!ent)
        return -1;

    /* give fair warning */
    dragent_notify(pad, ent, REMOVED);

    /* take it off the pad */
    if(widget == pad->drag_widget) {
        gtk_grab_remove(pad->drag_widget);
        pad->drag_widget = 0;
    }
    pad->selected = g_list_remove(pad->selected, ent);
    pad->widgets = g_list_remove(pad->widgets, ent);
    gtk_container_remove(GTK_CONTAINER(pad->layout), ent->widget);

    /* destroy the entry */
    for(t = ent->callbacks; t; t = t->next)
        delete t->data;
    g_list_free(ent->callbacks);
    delete ent;

    return 0;
}

int
dragpad_addcallback(dragpad *pad, GtkWidget *widget, dragent_callback cb,
                    void *ctx)
{
    struct cbinfo *cbinfo;
    dragentry *ent;

    ent = dragpad_getent(pad, widget);
    if(ent) {
        cbinfo = new struct cbinfo;
        cbinfo->id = cbid++;
        cbinfo->cb = cb;
        cbinfo->ctx = ctx;
        ent->callbacks = g_list_prepend(ent->callbacks, cbinfo);
        return cbinfo->id;
    }

    g_warning("Callback not added because widget not found"); 
    return -1;
}

int
dragpad_remcallback(dragpad *pad, GtkWidget *widget, int id)
{
    GList *t;
    struct cbinfo *cbinfo;
    dragentry *ent;

    ent = dragpad_getent(pad, widget);
    if(!ent)
        return -1;

    for(t = ent->callbacks; t; t = t->next) {
        cbinfo = (struct cbinfo *)t->data;

        if(cbinfo->id == id)
            break;
    }
    if(t) {
        ent->callbacks = g_list_remove_link(ent->callbacks, t);
        delete cbinfo;
        return 0;
    }

    return -1;
}

int
dragpad_getpos(dragpad *pad, GtkWidget *widget, int *xp, int *yp)
{
    dragentry *ent;

    ent = dragpad_getent(pad, widget);
    if(ent) {
        *xp = ent->x;
        *yp = ent->y;
        return 0;
    }
    return -1;
}

int
dragpad_setpos(dragpad *pad, GtkWidget *widget, int x, int y)
{
    dragentry *ent;

    ent = dragpad_getent(pad, widget);
    if(ent) {
        ent->x = x;
        ent->y = y;
        gtk_layout_move(GTK_LAYOUT(pad->layout), widget, x, y);
        adjsize(pad, widget, x, y);
        dragent_notify(pad, ent, MOVED);
        return 0;
    }
    return -1;
}

void
dragpad_refresh(dragpad *pad, int x1, int y1, int x2, int y2)
{
    GdkEventExpose exp;
    GtkLayout *lay;

    lay = GTK_LAYOUT(pad->layout);
    exp.type = GDK_EXPOSE;
    exp.window = lay->bin_window;
    exp.send_event = 0;
    exp.area.x = x1 - lay->xoffset;
    exp.area.y = y1 - lay->yoffset;
    exp.area.width = x2 - x1;
    exp.area.height = y2 - y1;
    exp.count = 0;

    gtk_widget_event(pad->layout, (GdkEvent *)&exp);
}

