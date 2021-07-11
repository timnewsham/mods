
#include <stdio.h>
#include <gtk/gtk.h>

#include "mymodbox.h"

static gboolean
destroyaction(GtkWidget *item, GdkEvent *ev, gpointer user)
{
    gtk_main_quit();
    return TRUE;
}

static void
termselect(GtkWidget *widget, int num, int out, GdkEventButton *ev)
{
    printf("%p: terminal %d/%d selected button %d\n",
           widget, num, out, ev->button);
}

static GtkWidget *
make_modbox(char *name, int ins, char **in, int outs, char **out)
{
    GtkWidget *x;

    x = mymodbox_new(name, ins, in, outs, out);
    gtk_widget_show(x);
    gtk_signal_connect(GTK_OBJECT(x), "input_select",
                       GTK_SIGNAL_FUNC(termselect), 0);
    gtk_signal_connect(GTK_OBJECT(x), "output_select",
                       GTK_SIGNAL_FUNC(termselect), 0);
    return x;
}

static void
setup()
{
    GtkWidget *top, *box, *mb1, *mb2, *mb3, *mb4;

    // create a window
    top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(top, 200, 200);
    gtk_window_set_title(GTK_WINDOW(top), "Knob");
    gtk_window_set_policy(GTK_WINDOW(top), TRUE, TRUE, FALSE);
    gtk_signal_connect(GTK_OBJECT(top), "delete_event",
                       GTK_SIGNAL_FUNC(destroyaction), 0);

    box = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(top), box);
    gtk_widget_show(box);

    { static char *in[] = {"hi", "there", "bob"};
      static char *out[] = {"out"};
      mb1 = make_modbox("Test 1", 3, in, 1, out); 
    }
    { static char *in[] = {"in"};
      static char *out[] = {"out1", "out2"};
      mb2 = make_modbox("A Longer Name Than Usual", 1, in, 2, out); 
    }
    { static char *x[] = {"hi"};
      mb3 = make_modbox("Test 3", 1, x, 0, 0);
    }
    { static char *x[] = {"hi"};
      mb4 = make_modbox("Test 4", 0, 0, 1, x);
    }

    gtk_box_pack_start(GTK_BOX(box), mb1, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), mb2, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), mb3, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), mb4, FALSE, TRUE, 0);

    gtk_widget_show(top);
}

int
main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    setup();
    gtk_main();
    return 0;
}

