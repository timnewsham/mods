
#include <gtk/gtk.h>

#include "mymodbox.h"
#include "util.h"

/* geometry of the layout */
#define MBPERLINE       10
#define MBLDASH         3
#define MBRDASH         5
#define MBMARGIN        4
#define MBVMARGIN       2
#define MBIOGAP         14
#define MBFONTOFF       8
#define MBSHADOW        2

enum {
    INPUTSEL,
    OUTPUTSEL,
    LAST_SIGNAL
};

static GtkWidgetClass *parent_class = NULL;
static int mymodbox_signals[LAST_SIGNAL] = {0};

static void
mymodbox_draw(GtkWidget *w)
{
    MyModbox *mb = MYMODBOX(w);
    GdkDrawable *d = w->window;
    GdkGC *fg, *dark, *mid, *light, *text;
    int left, right, i;

    if(!d)
        return;

    fg = w->style->fg_gc[GTK_WIDGET_STATE(w)];
    dark = w->style->dark_gc[GTK_WIDGET_STATE(w)];
    mid = w->style->mid_gc[GTK_WIDGET_STATE(w)];
    light = w->style->light_gc[GTK_WIDGET_STATE(w)];
    text = w->style->text_gc[GTK_WIDGET_STATE(w)];

    /* clear the text portion, the rest gets overwritten anyway */
    gdk_window_clear_area(d, 0, mb->boxheight, mb->width, 
                          mb->height - mb->boxheight);

    left = MBLDASH;
    right = left + mb->boxwidth;

    /* shaded box */
    gdk_draw_rectangle(d, light, TRUE, 0, 0, mb->width - 1, mb->boxheight);
    gdk_draw_rectangle(d, mid, TRUE,
                       MBSHADOW, mb->boxheight - MBSHADOW,
                       mb->width - 1, MBSHADOW);
    gdk_draw_rectangle(d, mid, TRUE,
                       mb->width - MBSHADOW - 1, MBSHADOW,
                       MBSHADOW, mb->boxheight - MBSHADOW);
    gdk_draw_rectangle(d, dark, FALSE, 
                       0, 0, mb->width - 1, mb->boxheight);

    /* title below */
    gdk_draw_string(d, mb->font, text,
                    left + ((mb->boxwidth - mb->namewidth) >> 1),
                    mb->boxheight + MBFONTOFF, mb->name);

    /* note: the following code could be optimized if it had to be
     * but this is probably not much work compared to the actual
     * drawing being done
     */

    /* inputs on left */
    for(i = 0; i < mb->num_in; i++) {
        gdk_draw_string(d, mb->font, fg,
                        left + MBMARGIN,
                        mb->in[i].ypos + MBFONTOFF,
                        mb->in[i].name);
        gdk_draw_line(d, fg,
                      0, mb->in[i].ypos + (MBPERLINE / 2),
                      left, mb->in[i].ypos + (MBPERLINE / 2));
    }

    /* outputs on right */
    for(i = 0; i < mb->num_out; i++) {
        gdk_draw_string(d, mb->font, fg,
                        right - MBMARGIN - mb->out[i].width,
                        mb->out[i].ypos + MBFONTOFF,
                        mb->out[i].name);
        gdk_draw_line(d, fg,
                      right, mb->out[i].ypos + (MBPERLINE / 2),
                      right + MBRDASH, mb->out[i].ypos + (MBPERLINE / 2));
    }
}

static int
mymodbox_expose(GtkWidget *widget, GdkEventExpose *exp)
{
    if(!widget || !IS_MYMODBOX(widget) || !exp)
        return FALSE;
    if(exp->count > 0)
        return FALSE;

    mymodbox_draw(widget);
    return FALSE;
}

static int
mymodbox_button(GtkWidget *widget, GdkEventButton *event)
{
    MyModbox *mb;
    int num;

    if(!widget || !event || !IS_MYMODBOX(widget))
        return FALSE;
    mb = MYMODBOX(widget);

    if(event->type == GDK_BUTTON_PRESS) {
        if(event->x < (MBRDASH + MBLDASH)) {
            num = (event->y - MBVMARGIN) / MBPERLINE;
            if(num >= 0 && num < mb->num_in) {
                gtk_signal_emit(GTK_OBJECT(mb), mymodbox_signals[INPUTSEL],
                                num, 0, event);
                return TRUE;
            }
        }
        if(event->x > (mb->width - (MBRDASH + MBLDASH))) {
            num = (event->y - MBVMARGIN) / MBPERLINE;
            if(num >= 0 && num < mb->num_out) {
                gtk_signal_emit(GTK_OBJECT(mb), mymodbox_signals[OUTPUTSEL],
                                num, 1, event);
                return TRUE;
            }
        }
    } else if(event->type == GDK_BUTTON_RELEASE) {
    }
    return FALSE;
}

static void
mymodbox_init(MyModbox *mb)
{
    mb->font = gdk_font_load("-*-fixed-*-*-*-*-*-80-*-*-*-*-iso646.1991-*");
    if(!mb->font) {
        g_error("Couldnt load font");
        return;
    }
    gdk_font_ref(mb->font);

    mb->num_in = 0;
    mb->num_out = 0;
    mb->boxwidth = 0;
    mb->boxheight = 0;
    mb->width = 0;
    mb->height = 0;
}

/* build up an allocated array describing the names */
static struct _mbconnect *
init_connects(GdkFont *font, int num, char **names)
{
    struct _mbconnect *tab;
    int i, ypos;

    tab = g_malloc((num + 1) * sizeof tab[0]);
    ypos = MBVMARGIN;
    for(i = 0; i < num; i++) {
        tab[i].name = g_strdup(names[i]);
        tab[i].width = gdk_string_width(font, tab[i].name);
        tab[i].ypos = ypos;
        ypos += MBPERLINE;
    }
    tab[i].name = 0;

    return tab;
}

GtkWidget *
mymodbox_new(char *name, int num_ins, char **ins, int num_outs, char **outs)
{
    MyModbox *mb;
    int i, x, inwidth, outwidth;

    mb = gtk_type_new(mymodbox_get_type());

    mb->in = init_connects(mb->font, num_ins, ins);
    mb->num_in = num_ins;
    mb->out = init_connects(mb->font, num_outs, outs);
    mb->num_out = num_outs;
    mb->name = g_strdup(name);
    mb->namewidth = gdk_string_width(mb->font, name);

    inwidth = outwidth = 0;
    for(i = 0; i < mb->num_in; i++) {
        if(mb->in[i].width > inwidth)
            inwidth = mb->in[i].width;
    }
    for(i = 0; i < mb->num_out; i++) {
        if(mb->out[i].width > outwidth)
            outwidth = mb->out[i].width;
    }
    mb->boxwidth = inwidth + outwidth + MBIOGAP + 2 * MBMARGIN;
    mb->boxheight = MAX(num_ins, num_outs) * MBPERLINE + 2 * MBVMARGIN;
    x = mb->namewidth + 2 * MBMARGIN;
    if(mb->boxwidth < x)
        mb->boxwidth = x;

    mb->width = mb->boxwidth + (MBLDASH + MBRDASH);
    mb->height = mb->boxheight + MBPERLINE;

    return GTK_WIDGET(mb);
}

int
mymodbox_gettermpos(MyModbox *mb, int num, int out, int *xp, int *yp)
{
    if(out) {
        if(num < 0 || num >= mb->num_out)
            return -1;
        *xp = mb->width;
        *yp = num * MBPERLINE + MBVMARGIN + MBPERLINE / 2;
    } else {
        if(num < 0 || num >= mb->num_in)
            return -1;
        *xp = 0;
        *yp = num * MBPERLINE + MBVMARGIN + MBPERLINE / 2;
    }
    return 0;
}

void
mymodbox_set_name(MyModbox *mb, char *name)
{
    int x;

    if(mb->name)
        g_free(mb->name);
    mb->name = g_strdup(name);

    /* recompute width */
    mb->namewidth = gdk_string_width(mb->font, name);
    x = mb->namewidth + 2 * MBMARGIN;
    if(mb->boxwidth < x) {
        mb->boxwidth = x;
        mb->width = mb->boxwidth + (MBLDASH + MBRDASH);
        gtk_widget_queue_resize(GTK_WIDGET(mb));
    }
}

static void
mymodbox_destroy(GtkObject *obj)
{
    MyModbox *mb;

    if(!obj || !IS_MYMODBOX(obj))
        return;
    mb = MYMODBOX(obj);

    g_free(mb->in);
    g_free(mb->out);
    gdk_font_unref(mb->font);

    if(GTK_OBJECT_CLASS(parent_class)->destroy)
        (*GTK_OBJECT_CLASS(parent_class)->destroy)(obj);
}

static void
mymodbox_sizereq(GtkWidget *widget, GtkRequisition *req)
{
    MyModbox *mb;

    if(!widget || !IS_MYMODBOX(widget))
        return;
    mb = MYMODBOX(widget);

    req->width = mb->width;
    req->height = mb->height;
}

static void
mymodbox_sizealloc(GtkWidget *widget, GtkAllocation *alloc)
{
    widget->allocation = *alloc;
    if(GTK_WIDGET_REALIZED(widget)) {
        gdk_window_move_resize(widget->window, alloc->x, alloc->y,
                               alloc->width, alloc->height);
    }
}

static void
mymodbox_realize(GtkWidget *widget)
{
    MyModbox *mb;

    if(!widget || !IS_MYMODBOX(widget))
        return;
    mb = MYMODBOX(widget);

    realize_window(widget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
}

static void
mymodbox_class_init(MyModboxClass *class)
{
    GtkObjectClass *object_class = (GtkObjectClass *)class;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)class;

    parent_class = gtk_type_class(gtk_widget_get_type());

    mymodbox_signals[INPUTSEL] = gtk_signal_new("input_select",
        GTK_RUN_FIRST, object_class->type,
        GTK_SIGNAL_OFFSET(MyModboxClass, inputsel),
        gtk_marshal_NONE__INT_INT_POINTER,
        GTK_TYPE_NONE, 3, GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_GDK_EVENT);
    mymodbox_signals[OUTPUTSEL] = gtk_signal_new("output_select",
        GTK_RUN_FIRST, object_class->type,
        GTK_SIGNAL_OFFSET(MyModboxClass, outputsel),
        gtk_marshal_NONE__INT_INT_POINTER,
        GTK_TYPE_NONE, 3, GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_GDK_EVENT);
    gtk_object_class_add_signals(object_class, mymodbox_signals, LAST_SIGNAL);

    object_class->destroy = mymodbox_destroy;
    widget_class->realize = mymodbox_realize;
    widget_class->expose_event = mymodbox_expose;
    widget_class->size_request = mymodbox_sizereq;
    widget_class->size_allocate = mymodbox_sizealloc;
    widget_class->button_press_event = mymodbox_button;
    class->inputsel = NULL;
    class->outputsel = NULL;
}

int
mymodbox_get_type()
{
    static int mymodbox_type = 0;

    if(!mymodbox_type) {
        GtkTypeInfo mymodbox_info = {
            "MyModbox", sizeof(MyModbox), sizeof(MyModboxClass),
            (GtkClassInitFunc) mymodbox_class_init,
            (GtkObjectInitFunc) mymodbox_init,
            (GtkArgSetFunc) NULL, (GtkArgGetFunc) NULL,
            (GtkClassInitFunc) NULL,
        };

        mymodbox_type = gtk_type_unique(gtk_widget_get_type(), &mymodbox_info);
    }
    return mymodbox_type;
}


