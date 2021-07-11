
#define O_WIDTH         100     /* geometry */
#define O_HEIGHT        50
#define STICKS          10      /* how often we draw */

class gtkoscopemodule : public module {
public:
    gtkoscopemodule(char *);
    ~gtkoscopemodule();
    void slowtick(void);
    void fasttick(void);
    int proc_params(void);

public:
    int buf[O_WIDTH];
    GtkWidget *draw, *vb, *label;
    char *name;
    int max;

private:
    int skip, trig;
    int pos, stick, ftick, ttick;
};

