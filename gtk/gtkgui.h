
class gtkgui : public gui {
public:
    gtkgui();
    ~gtkgui();

    int init(int *argcp, char **argv);
    void run(void);
    guipeer *new_wire(class module *m1, int idx1, class module *m2, int idx2);
    guipeer *new_module(module *mod);
    void error(char *msg);
    void shutdown(void);
};

