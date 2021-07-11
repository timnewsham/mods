
#include <stdlib.h>
#include <gtk/gtk.h>

#include "mod.h"
#include "gui.h"
#include "misc.h"

#include "myknob.h"
#include "mymodbox.h"
#include "dragpad.h"
#include "modpeer.h"
#include "wirepeer.h"
#include "mods.h"

#include "gtkknob.h"
#include "gtkoscope.h"

extern struct descr_mod descr_gtkknobmodule, descr_gtkoscopemodule,
    descr_constmodule;
extern dragpad *controlpad;

void
delete_selected_mods(dragpad *pad)
{
    GList *t, *next;
    dragentry *ent;
    modpeer *peer;

    for(t = pad->selected; t; t = next) {
        next = t->next;
        ent = (dragentry *)t->data;

        peer = get_modpeer(ent->widget);
        if(peer && !(strcmp(peer->mod->type, "audio") == 0) &&
                   !(strcmp(peer->mod->type, "key") == 0))
            delete peer->mod;
    }
    return;
}


/* XXX doesnt properly support multiple pads */
static void
termsel(GtkWidget *widget, int num, int out, GdkEventButton *ev, dragpad *p)
{
    static int lastnum = -1, lastout = -1;
    static GtkWidget *lastwidget;

    if(ev->button == 1) {
        if(lastnum == -1) {
            lastnum = num;
            lastout = out;
            lastwidget = widget;
        } else {
            if(lastwidget != widget)
                make_wire(p, widget, num, out, lastwidget, lastnum, lastout);
            lastnum = lastout = -1;
        }
    }
}

modpeer::modpeer(module *m, dragpad *p)
{
    mod = m;
    knob = 0;
    knoblabel = 0;
    knobbox = 0;
    pad = p;
    isknob = (mod->descr == &descr_gtkknobmodule);

    check_name(m->name);

    mb = modbox_new_bydescr(mod->descr);
    gtk_widget_show(mb);
    dragpad_add(pad, mb, -1, -1, DE_FLAG_DRAG | DE_FLAG_SELECT);
    gtk_object_set_data(GTK_OBJECT(mb), "peer", this);
    gtk_signal_connect(GTK_OBJECT(mb), "input_select",
                       GTK_SIGNAL_FUNC(termsel), p);
    gtk_signal_connect(GTK_OBJECT(mb), "output_select",
                       GTK_SIGNAL_FUNC(termsel), p);

    if(isknob) {
        knobbox = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(knobbox);

        knob = myknob_new(0);
        gtk_widget_show(knob);
        gtk_box_pack_start(GTK_BOX(knobbox), knob, FALSE, FALSE, 0);

        knoblabel = gtk_label_new("unnamed");
        gtk_widget_show(knoblabel);
        gtk_box_pack_start(GTK_BOX(knobbox), knoblabel, FALSE, FALSE, 0);

        dragpad_add(controlpad, knobbox, -1, -1, 
                    DE_FLAG_DRAG | DE_FLAG_SELECT);
    }
}

modpeer::~modpeer()
{
    if(isknob && knob) {
        dragpad_remove(controlpad, knobbox);
        //gtk_container_remove(GTK_CONTAINER(knobbox), knob);
        //gtk_widget_destroy(knobbox);
        //gtk_widget_destroy(knob);
    }

    /* XXX it seems like the widget is getting destroyed when it is
     * removed from the dragpad.  neatoh
     */
    dragpad_remove(pad, mb);
    //gtk_widget_destroy(mb);

    // XXX if pad is empty, should it go away?
}

void
modpeer::notify(guipeer_notification type)
{
    gtkknobmodule *knobmod;
    char *xs, *ys;

    if(type == DELETED) {
        delete this;
        return;
    }

    if(type == PARAM_CHANGED) {
        if(mod->descr == &descr_constmodule)
            mymodbox_set_name(MYMODBOX(mb), mod->get_param_str(0));

        if(isknob) {
            knobmod = (gtkknobmodule *)mod;
            mymodbox_set_name(MYMODBOX(mb), knobmod->get_knobname());
            gtk_label_set_text(GTK_LABEL(knoblabel), knobmod->get_knobname());

            /*
             * we dont set_ widget in the constructor above because
             * it is called from the "module" constructor, before
             * the gtkknobmodule constructor has had a chance to run,
             * which would clear out the widget field
             */
            knobmod->set_widget(MYKNOB(knob));
        }

        if(mod->descr == &descr_gtkoscopemodule) {
            gtkoscopemodule *omod = (gtkoscopemodule *)mod;
            mymodbox_set_name(MYMODBOX(mb), omod->name);
        }
    }

    if(type == ATTR_CHANGED) {
        xs = mod->get_attr("x");
        ys = mod->get_attr("y");
        if(xs && ys)
            dragpad_setpos(pad, mb, atoi(xs), atoi(ys));

        xs = mod->get_attr("ctrl_x");
        ys = mod->get_attr("ctrl_y");
        if(isknob && xs && ys)
            dragpad_setpos(controlpad, knobbox, atoi(xs), atoi(ys));

        if(mod->descr == &descr_gtkoscopemodule && xs && ys) {
            gtkoscopemodule *omod = (gtkoscopemodule *)mod;
            dragpad_setpos(controlpad, omod->vb, atoi(xs), atoi(ys));
        }
    }
}

void
modpeer::syncattr(void)
{
    int x, y;

    if(dragpad_getpos(pad, mb, &x, &y) != -1) {
        mod->set_attr("x", num2str(x));
        mod->set_attr("y", num2str(y));
    }

    if(isknob && dragpad_getpos(controlpad, knobbox, &x, &y) != -1) {
        mod->set_attr("ctrl_x", num2str(x));
        mod->set_attr("ctrl_y", num2str(y));
    }

    if(mod->descr == &descr_gtkoscopemodule) {
        gtkoscopemodule *omod = (gtkoscopemodule *)mod;
        if(dragpad_getpos(controlpad, omod->vb, &x, &y) != -1) {
            mod->set_attr("ctrl_x", num2str(x));
            mod->set_attr("ctrl_y", num2str(y));
        }
    }
}

modpeer *
get_modpeer(GtkWidget *wid)
{
    return (modpeer *)gtk_object_get_data(GTK_OBJECT(wid), "peer");
}

