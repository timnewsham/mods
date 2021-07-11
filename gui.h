
class module;
struct descr_mod;

typedef enum {
    PARAM_CHANGED, STATE_CHANGED, ATTR_CHANGED, DELETED
} guipeer_notification;

// a guipeer object holds the user interface's state for synth
// objects.  When the underlying object is created, a peer object
// is created.  When the object changes or is deleted, the peer object
// is notified of this change.
class guipeer {
public:
     virtual ~guipeer() {};
     virtual void notify(guipeer_notification type) = 0;
     virtual void syncattr(void) = 0;
};

// a gui object represents a user interface.   It has methods
// for creating peer elements used to represent synth objects
// to the user.
class gui {
public:
    virtual ~gui() {};
    virtual int init(int *argcp, char **argv) = 0;
    virtual void run(void) = 0;
    virtual guipeer *new_wire(class module *m1, int idx1, 
                              class module *m2, int idx2) { return 0; }
    virtual guipeer *new_module(class module *mod)        { return 0; }
    virtual void error(char *msg) = 0;
    virtual void shutdown(void) = 0;
};

extern gui *gui;

void set_gui(class gui *);

