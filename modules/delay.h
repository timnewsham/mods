
class delaymodule : public module {
public:
    delaymodule(char *);
    ~delaymodule();
    void fasttick(void);
    void slowtick(void);
    int proc_params(void);

private:
    short *buf;
    int bufsize;
    int pos, delaypos, interp;
    int fbfactor, ftfactor, mixdry, mixwet;
};

