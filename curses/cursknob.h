
class cursknobmodule : public module {
public:
    cursknobmodule(char *);
    ~cursknobmodule();
    void slowtick();
    char *get_state(void);
    int set_state(char *);
    int proc_params();

    void adjust(int);
    void adjusttick(int);
    void draw();
    void set(int);

private:
    char *knobname;
    int min, max;
    int val;
};

void move_curknob(int dir);
void curknob_adjust(int dir);

