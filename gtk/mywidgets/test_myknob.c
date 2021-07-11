
#include <stdio.h>
#include <gtk/gtk.h>

#include "myknob.h"

static GtkWidget *k1, *k2, *k3;

static int
mytimeout(gpointer unused)
{
    static int x = 0, dir = 1;

    x += dir;
    if(x == 100)
        dir = -1;
    if(x == 0)
        dir = 1;
    myknob_set(MYKNOB(k1), x);
    return TRUE;
}

static gboolean
destroyaction(GtkWidget *item, GdkEvent *ev, gpointer user)
{
    gtk_main_quit();
    return TRUE;
}

static void
relabel(GtkAdjustment *adjust, GtkWidget *label)
{
    char buf[15];

    sprintf(buf, "%d %%", (int)adjust->value);
    gtk_label_set_text(GTK_LABEL(label), buf);
}

static void
setup()
{
    GtkWidget *top, *box, *label, *vbox;
    GtkAdjustment *adj;

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

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);

    k1 = myknob_new(0);
    gtk_widget_show(k1);
    k2 = myknob_new(0);
    gtk_widget_show(k2);
    k3 = myknob_new(0);
    gtk_widget_show(k3);
    gtk_box_pack_start(GTK_BOX(box), k1, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), vbox, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), k3, FALSE, TRUE, 0);

    label = gtk_label_new("");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(vbox), k2, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);

    adj = MYKNOB(k2)->adjust;
    gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
                       GTK_SIGNAL_FUNC(relabel), label);

    myknob_set_steps(MYKNOB(k3), 3);

    gtk_widget_show(top);
}

int
main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    setup();
    gtk_timeout_add(50, mytimeout, 0);
    gtk_main();
    return 0;
}

