
class modpeer : public guipeer {
public:
    modpeer(class module *, struct _dragpad *);
    ~modpeer();

    void notify(guipeer_notification type);
    void syncattr(void);

    struct _dragpad *pad;
    GtkWidget *mb, *knob, *knoblabel, *knobbox;
    module *mod;
    int isknob;
};

modpeer *get_modpeer(GtkWidget *wid);
void delete_selected_mods(dragpad *pad);

