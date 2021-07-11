
#include <gtk/gtk.h>

#include "util.h"

GdkGC *
alloc_color_gc(GtkWidget *widget, int red, int green, int blue)
{
    GdkColor color;
    GdkGC *gc;

    memset(&color, 0, sizeof color);
    color.pixel = 0;
    color.red = red;
    color.green = green;
    color.blue = blue;
    if(!gdk_color_alloc(gtk_widget_get_colormap(widget), &color)) {
        g_error("Couldnt allocate color %x:%x:%x\n", red, green, blue);
        return 0;  
    }
    gc = gdk_gc_new(widget->window);
    gdk_gc_set_foreground(gc, &color);
    return gc;
}

void
realize_window(GtkWidget *widget, int eventmask)
{
    GdkWindowAttr attr;
    int attrmask;

    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
    attr.x = widget->allocation.x;
    attr.y = widget->allocation.y;
    attr.width = widget->allocation.width;
    attr.height = widget->allocation.height;
    attr.wclass = GDK_INPUT_OUTPUT;
    attr.window_type = GDK_WINDOW_CHILD;
    attr.event_mask = gtk_widget_get_events(widget) | eventmask;
    attr.visual = gtk_widget_get_visual(widget);
    attr.colormap = gtk_widget_get_colormap(widget);
    attrmask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new(gtk_widget_get_parent_window(widget), 
                                    &attr, attrmask);
    widget->style = gtk_style_attach(widget->style, widget->window);
    gdk_window_set_user_data(widget->window, widget);
    gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

