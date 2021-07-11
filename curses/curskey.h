
class curskeymodule : public module {
public:
    curskeymodule(char *);
    ~curskeymodule();
    void slowtick();
    char *get_state(void);
    int set_state(char *);

    void setoctave(int);
    void proc_key(int);

private:
    int onticks;
    int octave;
};

