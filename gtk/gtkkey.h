
class gtkkeymodule : public module {
public:
    gtkkeymodule(char *);
    ~gtkkeymodule();
    char *get_state(void);
    int set_state(char *);

    void setoctave(int);
    void proc_key(int code, char ch, int press);
    void play_note(int, int);
    void unplay_note(int);

private:
    int snooper;
    struct _GList *keylist;
    int octave;
};

