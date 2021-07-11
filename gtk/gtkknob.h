
class gtkknobmodule : public module {
public:
    gtkknobmodule(char *);
    ~gtkknobmodule();
    void slowtick(void);
    void set(int);
    char *get_state(void);
    int set_state(char *);
    int proc_params();

    /* extensions */
    void set_widget(MyKnob *p);
    char *get_knobname() { return knobname; }
    void set_steps(void);

private:
    MyKnob *widget;
    char *knobname;
    int min, max;
    int val;
};

