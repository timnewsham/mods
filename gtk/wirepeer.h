
class wirepeer : public guipeer {
public:
    wirepeer(class module *m1, int idx1, class module *m2, int idx2,
             struct _dragpad *p);
    ~wirepeer();

    void notify(guipeer_notification type);
    void syncattr(void) { }

    void draw();
    void undraw();
    void select(int on);

    struct _dragpad *pad;
    class module *mod1, *mod2;
    GdkGC *color;
    int idx1, idx2;
    int selectcnt;
    int cb1, cb2;
    int x1, y1, x2, y2;
    int xoff1, yoff1, xoff2, yoff2;
    int num1, out1, num2, out2;
};

int exposewires(GtkWidget *widget, GdkEventExpose *exp, GList **wires);
int presswires(GtkWidget *widget, GdkEventButton *event, dragpad *pad);
void make_wire(struct _dragpad *p, GtkWidget *w1, int num1, int out1, GtkWidget *w2, int num2, int out2);
void delete_selected_wires(dragpad *pad);

