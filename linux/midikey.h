
struct midinotes {
    struct midinotes *next;
    char note;
    char vel;
};

class midikeymodule : public module {
public:
    midikeymodule(char *);
    ~midikeymodule();
    void slowtick(void);

    void proc_cmd(int cmd, int note, int vel);

private:
    int midifd, mcmd, note;
    struct midinotes *notelist;
};

