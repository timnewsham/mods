
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "mod.h"
#include "conf.h"
#include "gui.h"
#include "misc.h"

#include "mymodbox.h"
#include "dragpad.h"
#include "mods.h"
#include "modpeer.h"
#include "wirepeer.h"

static int nextmodid = 0;

int
connect_idx_fromio(struct descr_connect *con, int num, int out)
{
    int cnt, i, isout;

    out = (out != 0);
    cnt = 0;
    for(i = 0; con[i].name; i++) {
       isout = (con[i].type & CONTYPE_OUT != 0);
       if(isout != out)
           continue;

       if(cnt == num)
           return i;
       cnt++;
    }
    return -1;
}

int
connect_io_fromidx(struct descr_connect *con, int idx, int *nump, int *outp)
{
    int i, num[2];

    num[0] = num[1] = 0;
    for(i = 0; i != idx && con[i].name; i++) {
        if(con[i].type & CONTYPE_OUT)
            num[1] ++;
        else
            num[0] ++;
    }
    if(i != idx)
        return -1;
    *outp = (con[i].type & CONTYPE_OUT) ? 1 : 0;
    *nump = num[*outp];
    return 0;
}

static void
connect_proc_io(struct descr_connect *con, int *ip, int *op, 
                char **in, char **out)
{
    int numins, numouts, i;

    numins = numouts = 0;
    for(i = 0; con[i].name; i++) {
        if(con[i].type & CONTYPE_OUT) {
            if(out)
                out[numouts] = con[i].name;
            numouts ++;
        } else {
            if(in)
                in[numins] = con[i].name;
            numins ++;
        }
    }
    if(ip)
        *ip = numins;
    if(op)
        *op = numouts;
}

GtkWidget *
modbox_new_bydescr(struct descr_mod *descr)
{
    GtkWidget *wid;
    char **in, **out;
    int numins, numouts;

    // build up list of inputs and outputs
    connect_proc_io(descr->connect, &numins, &numouts, 0, 0);
    in = new (char *)[numins];
    out = new (char *)[numouts];
    connect_proc_io(descr->connect, 0, 0, in, out);
    wid = mymodbox_new(descr->name, numins, in, numouts, out);
    delete [] in;
    delete [] out;
    return wid;
}

static char *
gen_name()
{
    static char buf[30];

    g_snprintf(buf, sizeof buf, "gtkmod_%d", nextmodid++);
    return buf;
}

void
check_name(char *name)
{
    int n;

    if(strncmp(name, "gtkmod_", 7) == 0) {
        n = atoi(name + 7);
        if(n >= nextmodid)
            nextmodid = n + 1;
    }
}

static void
moddialogok(GtkWidget *button, struct descr_mod *descr)
{
    module *mod;
    void *input;
    char **arg;
    int i, nargs;

    if(button)
        gtk_widget_hide(GTK_WIDGET(descr->data));

    // build module
    mod = descr->instantiate(gen_name());
    nargs = param_count(descr->param);
    arg = new (char *)[nargs];
    for(i = 0; i < nargs; i++) {
        input = gtk_object_get_data(GTK_OBJECT(descr->data), 
                                    descr->param[i].name);
        arg[i] = gtk_entry_get_text(GTK_ENTRY(input));
    }
    mod->set_params(nargs, arg);
    delete [] arg;
}

static void
moddialogcancel(GtkWidget *widget, struct descr_mod *descr)
{
    gtk_widget_hide(GTK_WIDGET(descr->data));
}

static int
moddialogdelete(GtkWidget *widget, GdkEvent *ev)
{
    gtk_widget_hide(widget);
    return TRUE;
}

static GtkWidget *
mod_prompt(struct descr_mod *descr)
{
    GtkWidget *dw, *x, *b, *fr, *vb;
    GtkDialog *d;
    struct descr_param *par;
    char *buf;
    int i;

    par = descr->param;
    if(!par[0].name)
        return 0;

    // make a dialog box full of prompts
    dw = gtk_dialog_new();
    d = GTK_DIALOG(dw);

    buf = new char[strlen(descr->name) + 15];
    g_snprintf(buf, strlen(descr->name) + 15, "%s parameters", descr->name);
    gtk_window_set_title(GTK_WINDOW(dw), buf);
    gtk_window_set_position(GTK_WINDOW(dw), GTK_WIN_POS_MOUSE);
    fr = gtk_frame_new(buf);
    gtk_widget_show(fr);
    gtk_container_set_border_width(GTK_CONTAINER(fr), 5);
    delete [] buf;

    gtk_box_pack_start(GTK_BOX(d->vbox), fr, TRUE, TRUE, 0);
    vb = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vb);
    gtk_container_add(GTK_CONTAINER(fr), vb);

    for(i = 0; par[i].name; i++) {
        b = gtk_hbox_new(FALSE, 0);
        gtk_widget_show(b);

        x = gtk_label_new(par[i].name);
        gtk_widget_show(x);
        gtk_box_pack_start(GTK_BOX(b), x, TRUE, FALSE, 10);

        // XXX should make diff widget types for diff input types and
        // ranges, ie, radio button, slider/spinner, text entry
        x = gtk_entry_new();
        gtk_widget_show(x);
        gtk_object_set_data(GTK_OBJECT(dw), par[i].name, x);
        gtk_box_pack_start(GTK_BOX(b), x, TRUE, FALSE, 10);

        gtk_box_pack_start(GTK_BOX(vb), b, TRUE, FALSE, 5);
    }

    x = gtk_button_new_with_label("OK");
    gtk_widget_show(x);
    gtk_box_pack_start(GTK_BOX(d->action_area), x, FALSE, FALSE, 0);
    gtk_signal_connect(GTK_OBJECT(x), "clicked",
                      GTK_SIGNAL_FUNC(moddialogok), descr);
    x = gtk_button_new_with_label("Cancel");
    gtk_widget_show(x);
    gtk_box_pack_start(GTK_BOX(d->action_area), x, FALSE, FALSE, 0);
    gtk_signal_connect(GTK_OBJECT(x), "clicked",
                      GTK_SIGNAL_FUNC(moddialogcancel), descr);
    gtk_signal_connect(GTK_OBJECT(dw), "delete_event",
                      GTK_SIGNAL_FUNC(moddialogdelete), descr);

    return dw;
}

static void
newmod(GtkMenuItem *item, struct descr_mod *descr)
{
    if(descr->data)
        gtk_widget_show(GTK_WIDGET(descr->data));
    else
        moddialogok(0, descr);
}

static void
module_menu(GtkWidget *menu)
{
    struct modtype_list *t;
    GtkWidget *item;

    for(t = all_modtypes; t; t = t->next) {
        t->modtype->data = (GtkWidget *)mod_prompt(t->modtype);

        item = gtk_menu_item_new_with_label(t->modtype->name);
        gtk_menu_append(GTK_MENU(menu), item);
        gtk_widget_show(item);
        gtk_signal_connect(GTK_OBJECT(item), "activate",
                           GTK_SIGNAL_FUNC(newmod), t->modtype);
    }
    return;
}

static dragpad *
get_pad(GtkWidget *w)
{
    return (dragpad *)gtk_object_get_data(GTK_OBJECT(w), "pad");
}

static void
mods_cut(gpointer data, guint action, GtkWidget *w)
{
    dragpad *pad = get_pad(w);

    delete_selected_wires(pad);
    delete_selected_mods(pad);
    return;
}

static void
notyet()
{
    g_print("menu item not yet supported\n");
}

#define CB              GtkItemFactoryCallback

static GtkItemFactoryEntry menu_items[] = {
    { "/_New",          0,            0,              0, "<Branch>" },
    { "/_Edit",         0,            0,              0, "<Branch>" },
    { "/Edit/_Cut",     "<Delete>",   (CB)mods_cut,   0, 0 },
    { "/Edit/Copy",     0,            (CB)notyet,  0, 0 },
    { "/Edit/_Paste",   "<Insert>",   (CB)notyet,  0, 0 },
};

#define FACTABSIZE(tab)  (sizeof(tab) / sizeof(tab[0]))

static void
menu_data(GtkItemFactory *ifac, char *path, char *key, gpointer data)
{
    GtkWidget *w;

    w = gtk_item_factory_get_widget(ifac, path);
    if(!w)
        die("menu_data: %s not found\n", path);
    gtk_object_set_data(GTK_OBJECT(w), key, data);
    return;
}

GtkWidget *
new_modwin(char *title, dragpad **retpad)
{
    GtkWidget *win, *vb, *hb, *menu, *new_menu;
    GtkAccelGroup *accel;
    GtkItemFactory *ifac;
    GList **wires;
    dragpad *pad;

    win = gtk_window_new(GTK_WINDOW_DIALOG);
    gtk_widget_set_usize(win, 500, 300);
    gtk_window_set_title(GTK_WINDOW(win), title);
    gtk_window_set_policy(GTK_WINDOW(win), TRUE, TRUE, FALSE);
    gtk_widget_add_events(win, GDK_KEY_RELEASE_MASK);
    pad = dragpad_new();

    accel = gtk_accel_group_new();
    ifac = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<design>", accel); 
    gtk_item_factory_create_items(ifac, FACTABSIZE(menu_items), menu_items, 0);
    gtk_accel_group_attach(accel, GTK_OBJECT(win));
    menu = gtk_item_factory_get_widget(ifac, "<design>");
    new_menu = gtk_item_factory_get_widget(ifac, "<design>/New");
    module_menu(new_menu);
    menu_data(ifac, "<design>/Edit/Cut", "pad", pad);
    menu_data(ifac, "<design>/Edit/Copy", "pad", pad);
    menu_data(ifac, "<design>/Edit/Paste", "pad", pad);
    gtk_widget_show(menu);

    vb = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(win), vb);
    gtk_widget_show(vb);
    gtk_box_pack_start(GTK_BOX(vb), menu, FALSE, TRUE, 0);

    hb = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, TRUE, 0);
    gtk_widget_show(hb);

    gtk_box_pack_start(GTK_BOX(vb), pad->scrollwin, TRUE, TRUE, 0);

    /* allocate a wirelist and attach it to the layout */
    wires = new (GList *);
    *wires = 0;
    gtk_widget_add_events(pad->layout, GDK_BUTTON_PRESS_MASK);
    gtk_object_set_data(GTK_OBJECT(pad->layout), "wirelist", wires);
    gtk_signal_connect(GTK_OBJECT(pad->layout), "expose_event",
                       GTK_SIGNAL_FUNC(exposewires), wires);
    gtk_signal_connect(GTK_OBJECT(pad->layout), "button_press_event",
                       GTK_SIGNAL_FUNC(presswires), pad);

    if(retpad)
        *retpad = pad;
    return win;
}


