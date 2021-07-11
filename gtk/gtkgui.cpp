
#include <gtk/gtk.h>

#include "mod.h"
#include "gui.h"
#include "wire.h"
#include "conf.h"
#include "misc.h"

#include "gtkgui.h"
#include "gtkkey.h"
#include "dragpad.h"
#include "mods.h"
#include "modpeer.h"
#include "wirepeer.h"

extern struct descr_mod descr_gtkknobmodule, descr_gtkoscopemodule;

static struct descr_mod *gtk_modules[] = {
    &descr_gtkknobmodule, &descr_gtkoscopemodule
};

dragpad *controlpad, *designpad, *curpad;
GtkWidget *mainwin = 0;
GtkWidget *designwin = 0;
gtkkeymodule *keymod = 0;

enum { SEL_LOAD, SEL_SAVE, SEL_READ, SEL_WRITE };

struct fileseldat {
    GtkFileSelection *filesel;
    gpointer data;
};

static int
keypress(GtkWidget *w, GdkEventKey *ev, gpointer *modp)
{
    gtkkeymodule *mod = (gtkkeymodule *)modp;

    if(ev->type == GDK_KEY_PRESS && ev->length == 1)
        mod->proc_key(ev->keyval, ev->string[0], 1);
    else if(ev->type == GDK_KEY_RELEASE)
        mod->proc_key(ev->keyval, 0, 0);

    return FALSE;
}

static int
leave(GtkWidget *w, GdkEventCrossing *ev, gpointer *modp)
{
    gtkkeymodule *mod = (gtkkeymodule *)modp;

    mod->proc_key(-1, -1, 0);
    return FALSE;
}

static void
gtkkey_register_window(GtkWidget *win)
{
    gtk_widget_add_events(win, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
    gtk_signal_connect(GTK_OBJECT(win), "key_press_event",
                       GTK_SIGNAL_FUNC(keypress), keymod);
    gtk_signal_connect(GTK_OBJECT(win), "key_release_event",
                       GTK_SIGNAL_FUNC(keypress), keymod);
    gtk_signal_connect(GTK_OBJECT(win), "leave_notify_event",
                       GTK_SIGNAL_FUNC(leave), keymod);
    return;
}

static void
quitaction()
{
    gtk_main_quit();
}

static void
newaction()
{
    rem_all_modules();
    rem_all_wires();
}

void
filesel_ok(GtkWidget *w, struct fileseldat *dat)
{
    char *fname;

    // XXX check for errors
    fname = gtk_file_selection_get_filename(dat->filesel);
    switch((int)dat->data) {
    case SEL_LOAD:
        newaction();
        readconf(fname);
        break;
    case SEL_SAVE:
        writeconf(fname);
        break;
    case SEL_READ:
        load_state(fname);
        break;
    case SEL_WRITE:
        save_state(fname);
        break;
    }
    gtk_widget_destroy(GTK_WIDGET(dat->filesel));
    delete dat;
}

static void
filesel(char *title, GtkSignalFunc func, gpointer func_data)
{
    GtkWidget *w;
    GtkFileSelection *fs;
    struct fileseldat *dat;

    w = gtk_file_selection_new(title);
    fs = GTK_FILE_SELECTION(w);
    dat = new fileseldat;
    dat->filesel = fs;
    dat->data = func_data;
    gtk_signal_connect(GTK_OBJECT(fs->ok_button), "clicked", func, dat);

    gtk_signal_connect_object(GTK_OBJECT(fs->cancel_button), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(w));
#if nope
    gtk_signal_connect_object(GTK_OBJECT(fs), "destroy",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(w));
#endif
    gtk_widget_show(w);
    return;
}

static void
fileaction(gpointer data, guint action, GtkWidget *w)
{
    char *title;

    switch(action) {
    case SEL_LOAD: title = "Load Configuration"; break;
    case SEL_SAVE: title = "Save Configuration"; break;
    case SEL_READ: title = "Read Settings"; break;
    case SEL_WRITE: title = "Write Settings"; break;
    default: title = "???"; break;
    }
    filesel(title, GTK_SIGNAL_FUNC(filesel_ok), (void *)action);
}

static gboolean
destroy(GtkWidget *widget, GdkEvent *ev, gpointer unused)
{
    quitaction();
    return FALSE;
}

#define CB		GtkItemFactoryCallback

static GtkItemFactoryEntry menu_items[] = {
    { "/_File",         0,            0,              0, "<Branch>" },
    { "/File/_New",     0,            (CB)newaction,  0, 0 },
    { "/File/_Load",    0,            (CB)fileaction, SEL_LOAD, 0 },
    { "/File/_Save",    0,            (CB)fileaction, SEL_SAVE, 0 },
    { "/File/_Read Settings",  0,     (CB)fileaction, SEL_READ, 0 },
    { "/File/_Write Settings", 0,     (CB)fileaction, SEL_WRITE, 0 },
    { "/File/_Quit",    "<control>Q", (CB)quitaction, 0, 0 },
    { "/_Windows",      0,            0,              0, "<Branch>" }
};

#define FACTABSIZE(tab)  (sizeof(tab) / sizeof(tab[0]))

static void
togglewindow(GtkWidget *widget, GtkWidget *window)
{
    /* hide or unhide the window */
    if(GTK_CHECK_MENU_ITEM(widget)->active)
        gtk_widget_show(window);
    else
        gtk_widget_hide(window);
}

static gboolean
hidewindow(GtkWidget *widget, GdkEvent *ev, GtkWidget *item)
{
    /* uncheck the window's entry, that will force it to hide */
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), 0);
    return TRUE;
}

static void
hideable_window(GtkWidget *menu, char *name, GtkWidget *window)
{
    GtkWidget *item;

    item = gtk_check_menu_item_new_with_label(name);
    gtk_widget_show(item);
    gtk_menu_append(GTK_MENU(menu), item);

    gtk_signal_connect(GTK_OBJECT(item), "toggled",
                       GTK_SIGNAL_FUNC(togglewindow), window);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event",
                       GTK_SIGNAL_FUNC(hidewindow), item);

    /* unhide now */
    //gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), 1);
}

static void
guisetup()
{
    GtkWidget *vb, *menus, *winmenu;
    GtkAccelGroup *accel;
    GtkItemFactory *ifac;

    mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(mainwin, 450, 300);
    gtk_window_set_title(GTK_WINDOW(mainwin), "Synth");
    gtk_window_set_policy(GTK_WINDOW(mainwin), TRUE, TRUE, FALSE);
    gtk_signal_connect(GTK_OBJECT(mainwin), "delete_event",
                       GTK_SIGNAL_FUNC(destroy), 0);

    accel = gtk_accel_group_new();
    ifac = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel); 
    gtk_item_factory_create_items(ifac, FACTABSIZE(menu_items), menu_items, 0);
    gtk_accel_group_attach(accel, GTK_OBJECT(mainwin));
    menus = gtk_item_factory_get_widget(ifac, "<main>");
    winmenu = gtk_item_factory_get_widget(ifac,"<main>/Windows");
    gtk_widget_show(menus);

    vb = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(mainwin), vb);
    gtk_widget_show(vb);

    controlpad = dragpad_new();
    gtk_box_pack_start(GTK_BOX(vb), menus, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vb), controlpad->scrollwin, TRUE, TRUE, 0);
    gtk_widget_show(mainwin);

    /* make the design window */
    designwin = new_modwin("Synth Design", &designpad);
    hideable_window(winmenu, "Design", designwin);
    curpad = designpad;
}


static int
idlefunc(gpointer unused)
{
    drive();
    return TRUE;
}

gtkgui::gtkgui()
{
}

gtkgui::~gtkgui()
{
}

int
gtkgui::init(int *argcp, char **argv)
{
    register_modtypes(gtk_modules, ARRAYCNT(gtk_modules));

    gtk_init(argcp, &argv);
    gtk_idle_add(idlefunc, 0);
    guisetup();

    keymod = new gtkkeymodule("key");
    if(keymod->set_params_v(0) == -1)
        die("Error initializing gtkkey module");

    gtkkey_register_window(mainwin);
    gtkkey_register_window(designwin);
    
    return 0;
}

void
gtkgui::run()
{
    gtk_main();
}

guipeer *
gtkgui::new_wire(class module *m1, int idx1, class module *m2, int idx2)
{
    return new wirepeer(m1, idx1, m2, idx2, curpad);
}

guipeer *
gtkgui::new_module(module *mod)
{
    return new modpeer(mod, curpad);
}

void
gtkgui::error(char *mesg)
{
    // XXX show some dialog box at least
    g_print("%s\n", mesg);
    exit(1);
}

void
gtkgui::shutdown()
{
    // XXX
    exit(0);
}

