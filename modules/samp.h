
class sampmodule : public module {
public:
    sampmodule(char *);
    ~sampmodule();
    void slowtick();
    void fasttick();
    int proc_params(void);

private:
    long long pos;
    short *buf;
    int buflen, origfreq, adv, lasttrig;
};

