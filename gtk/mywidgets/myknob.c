
#include <math.h>
#include <gtk/gtk.h>

#include "myknob.h"
#include "util.h"

/* geometry of the knob */
#define KNOBSIZE 40
#define KNOBYSIZE (KNOBSIZE + 2)
#define KNOBCENTER (KNOBSIZE / 2)
#define KNOBYTEXT (KNOBSIZE - 1)
#define KNOBXTEXT (KNOBCENTER + 1)
#define KNOBRAD (KNOBSIZE * 0.23)
#define KNOBGAP (KNOBRAD * 0.5)
#define KNOBNOTCH (KNOBRAD * 0.75)
#define KNOBMARK (KNOBRAD * 0.34)
#define KNOBBIGMARK (KNOBRAD * 0.25)
#define KNOBWIDTH (KNOBRAD * 0.15)

#define KNOBWIDTHALT (KNOBRAD * 0.3)
#define KNOBNOTCHALT (KNOBRAD * 0.3)

//#define ALT_STYLE

static GtkWidgetClass *parent_class;


static void
draw_notch(GdkDrawable *draw, GdkGC *gc,
           int angle, int xc, int yc, int radstart, int radend)
{
    float c, s;
    float fang = angle * M_PI / 180.0;

    c = cos(fang);
    s = sin(fang);
    gdk_draw_line(draw, gc, xc + radstart * c + 0.5, yc - radstart * s + 0.5,
                  xc + radend * c + 0.5, yc - radend * s + 0.5);
}

static void
draw_notchbox(GdkDrawable *draw, GdkGC *gc,
              int angle, int xc, int yc, int radstart, int radend, float width)
{
    float c, s;
    float fang = angle * M_PI / 180.0;
    GdkPoint pt[4], xpt[2];

    c = cos(fang);
    s = sin(fang);

    /* figure out the notch line */
    xpt[0].x = xc + c * radstart + 0.5;
    xpt[0].y = yc - s * radstart + 0.5;
    xpt[1].x = xc + c * radend + 0.5;
    xpt[1].y = yc - s * radend + 0.5;

    /* compute the box based on the line and its perpendicular */
    pt[0].x = xpt[0].x - s * width + 0.5;
    pt[0].y = xpt[0].y - c * width + 0.5;
    pt[1].x = xpt[1].x - s * width + 0.5;
    pt[1].y = xpt[1].y - c * width + 0.5;
    pt[2].x = xpt[1].x + s * width + 0.5;
    pt[2].y = xpt[1].y + c * width + 0.5;
    pt[3].x = xpt[0].x + s * width + 0.5;
    pt[3].y = xpt[0].y + c * width + 0.5;
    gdk_draw_polygon(draw, gc, TRUE, pt, 4);
}

static void
myknob_draw(MyKnob *k)
{
    char buf[16];
    GtkWidget *w;
    GdkGC *fg, *light;
    GdkDrawable *d = GTK_WIDGET(k)->window;
    int i, xtra, angle, xpos;

    if(!d)
        return;

    w = GTK_WIDGET(k);
    fg = w->style->fg_gc[GTK_WIDGET_STATE(w)];
    light = w->style->light_gc[GTK_WIDGET_STATE(w)];

    gdk_window_clear_area(d, 0, 0, KNOBSIZE, KNOBYSIZE);

    /* draw scale around the outside */
    for(i = -45; i <= 225; i += (270 / 10)) {
        xtra = KNOBMARK;
        if(i == -45 || i == 225)
            xtra += KNOBBIGMARK;
        draw_notch(d, fg, i, KNOBCENTER, KNOBCENTER, 
                   KNOBRAD + KNOBGAP, KNOBRAD + KNOBGAP + xtra);
    }

#ifdef ALT_STYLE 
    /* draw the knob and its notch */
    gdk_draw_arc(d, light, 1, KNOBCENTER - KNOBRAD, KNOBCENTER - KNOBRAD,
                 KNOBRAD * 2, KNOBRAD * 2, 0, 360 << 6);
    gdk_draw_arc(d, fg, 0, KNOBCENTER - KNOBRAD, KNOBCENTER - KNOBRAD,
                 KNOBRAD * 2, KNOBRAD * 2, 0, 360 << 6);

    /* draw the notch */
    angle = 225 - 270 * k->adjust->value / 100;
    draw_notchbox(d, k->col0, angle, KNOBCENTER, KNOBCENTER,
                  -KNOBRAD - 1, KNOBRAD + 1, KNOBWIDTHALT);
    draw_notch(d, light, angle, KNOBCENTER, KNOBCENTER,
               KNOBRAD - KNOBNOTCHALT, KNOBRAD + 2);
#else
    /* draw the knob and its notch */
    gdk_draw_arc(d, k->col0, 1, KNOBCENTER - KNOBRAD, KNOBCENTER - KNOBRAD,
                 KNOBRAD * 2, KNOBRAD * 2, 0, 360 << 6);
    gdk_draw_arc(d, fg, 0, KNOBCENTER - KNOBRAD, KNOBCENTER - KNOBRAD,
                 KNOBRAD * 2, KNOBRAD * 2, 0, 360 << 6);

    /* draw the notch */
    angle = 225 - 270 * k->adjust->value / 100;
    draw_notchbox(d, light, angle, KNOBCENTER, KNOBCENTER, 
                  KNOBRAD - KNOBNOTCH, KNOBRAD, KNOBWIDTH);
#endif

    /* draw the text */
    g_snprintf(buf, sizeof buf - 1, "%d%%", (int)k->adjust->value);
    xpos = KNOBXTEXT - (gdk_string_width(k->font, buf) >> 1);
    gdk_draw_string(d, k->font, k->col0, xpos, KNOBYTEXT, buf);
}

static int
myknob_expose(GtkWidget *widget, GdkEventExpose *exp)
{
    if(!widget || !IS_MYKNOB(widget) || !exp)
        return FALSE;
    if(exp->count > 0)
        return FALSE;
    myknob_draw(MYKNOB(widget));
    return FALSE;
}

static int
myknob_button(GtkWidget *widget, GdkEventButton *event)
{
    MyKnob *k;
    int dx, dy, dist2, ang;

    if(!widget || !event || !IS_MYKNOB(widget))
        return FALSE;
    k = MYKNOB(widget);

    if(event->type == GDK_BUTTON_PRESS && event->button == 1) {
        /* see if the click is inside the circle */
        dx = event->x - KNOBCENTER;
        dy = event->y - KNOBCENTER;
        dist2 = dx * dx + dy * dy;
        if(dist2 < KNOBRAD * KNOBRAD) {
            /* start button dragging */
            k->pressy = event->y;
            k->pressval = k->adjust->value;
            gtk_grab_add (widget);
        } else {
            /* jump to angle the user pressed at */
            ang = atan2(-dy, dx) * 180.0 / M_PI;
            if(ang <= -90)
                ang += 360;
            myknob_set(k, (225 - ang) * 100 / 270);
        }
    } else if(event->type == GDK_BUTTON_RELEASE && event->button == 1) {
        if(k->pressy >= 0) {
            gtk_grab_remove (widget);
            k->pressy = -1;
        }
    }
    return FALSE;
}

static int
myknob_motion(GtkWidget *widget, GdkEventMotion *event)
{
    MyKnob *k;
    GdkModifierType mods;
    int x, y;

    if(!widget || !event || !IS_MYKNOB(widget))
        return FALSE;
    k = MYKNOB(widget);

    if(k->pressy > 0) {
        x = event->x;
        y = event->y;

        mods = GDK_BUTTON1_MASK;
        if(event->is_hint || (event->window != widget->window))
            gdk_window_get_pointer (widget->window, &x, &y, &mods);

        if(mods & GDK_BUTTON1_MASK)
            myknob_set(k, k->pressval - (y - k->pressy));
    }
    return FALSE;
}

static void
adjust_changed(GtkAdjustment *adjust, MyKnob *k)
{
    myknob_draw(k);
}

static void
myknob_init(MyKnob *k)
{
    GTK_WIDGET_UNSET_FLAGS(k, GTK_NO_WINDOW);

    k->font = gdk_font_load("-*-fixed-*-*-*-*-*-80-*-*-*-*-iso646.1991-*");
    if(!k->font) {
        g_error("Couldnt load font");
        return;
    }
    gdk_font_ref(k->font);

    k->adjust = (GtkAdjustment *)gtk_adjustment_new(0.0, 0.0, 100.0, 
                                                    1.0, 1.0, 1.0);
    gtk_object_ref(GTK_OBJECT(k->adjust));

    k->adjust->value = 0;
    k->pressy = -1;
    k->pressval = 0;

    gtk_signal_connect(GTK_OBJECT(k->adjust), "value_changed",
                       GTK_SIGNAL_FUNC(adjust_changed), k);
}

void
myknob_set(MyKnob *k, int val)
{
    if(val < 0)
        val = 0;
    if(val > 100)
        val = 100;
    if(k->steps > 0)
        val = (int)(val * k->steps * 0.01 + 0.5) * 100 / k->steps;

    if(k->adjust->value != val) {
        k->adjust->value = val;
        gtk_signal_emit_by_name(GTK_OBJECT(k->adjust), "value_changed");
    }
}

void
myknob_set_steps(MyKnob *k, int val)
{
    k->steps = val - 1;
    myknob_set(k, k->adjust->value);
}

GtkWidget *
myknob_new(val)
{
    MyKnob *k;

    k = gtk_type_new(myknob_get_type());
    myknob_set(k, val);
    return GTK_WIDGET(k);
}

static void
myknob_destroy(GtkObject *obj)
{
    MyKnob *k;

    if(!obj || !IS_MYKNOB(obj))
        return;
    k = MYKNOB(obj);

    gdk_font_unref(k->font);
    gtk_object_unref(GTK_OBJECT(k->adjust));
    if(GTK_OBJECT_CLASS(parent_class)->destroy)
        (*GTK_OBJECT_CLASS(parent_class)->destroy)(obj);
}

static void
myknob_sizereq(GtkWidget *widget, GtkRequisition *req)
{
    req->width = KNOBSIZE;
    req->height = KNOBYSIZE;
}

static void
myknob_sizealloc(GtkWidget *widget, GtkAllocation *alloc)
{
    int diff;

    if(!widget || !alloc || !IS_MYKNOB(widget))
        return;

    /* if we have extra room, center ourselves */
    if(alloc->width > KNOBSIZE) {
        diff = alloc->width - KNOBSIZE;
        alloc->x += diff / 2;
        alloc->width = KNOBSIZE;
    }
    if(alloc->height > KNOBYSIZE) {
        diff = alloc->width - KNOBYSIZE;
        alloc->y += diff / 2;
        alloc->height = KNOBYSIZE;
    }

    widget->allocation = *alloc;
    if(GTK_WIDGET_REALIZED(widget)) {
         gdk_window_move_resize(widget->window, alloc->x, alloc->y,
                                alloc->width, alloc->height);
    }
}

static void
myknob_realize(GtkWidget *widget)
{
    MyKnob *k;

    if(!widget || !IS_MYKNOB(widget))
        return;
    k = MYKNOB(widget);

    realize_window(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);

    k->col0 = alloc_color_gc(widget, 0, 0, 0xffff);
}

static void
myknob_class_init(MyKnobClass *class)
{
    GtkObjectClass *object_class = (GtkObjectClass *)class;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)class;

    parent_class = gtk_type_class(gtk_widget_get_type());
    object_class->destroy = myknob_destroy;
    widget_class->realize = myknob_realize;
    widget_class->expose_event = myknob_expose;
    widget_class->size_request = myknob_sizereq;
    widget_class->size_allocate = myknob_sizealloc;
    widget_class->button_press_event = myknob_button;
    widget_class->button_release_event = myknob_button;
    widget_class->motion_notify_event = myknob_motion;
}

int
myknob_get_type()
{
    static int myknob_type = 0;

    if(!myknob_type) {
        GtkTypeInfo myknob_info = {
            "MyKnob", sizeof(MyKnob), sizeof(MyKnobClass),
            (GtkClassInitFunc) myknob_class_init,
            (GtkObjectInitFunc) myknob_init,
            (GtkArgSetFunc) NULL, (GtkArgGetFunc) NULL,
            (GtkClassInitFunc) NULL,
        };

        myknob_type = gtk_type_unique(gtk_widget_get_type(), &myknob_info);
    }
    return myknob_type;
}

