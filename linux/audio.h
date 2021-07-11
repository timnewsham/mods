
#define DEF_DEVICE "/dev/dsp"

class audiomodule : public module {
public:
    audiomodule(char *);
    ~audiomodule();
    void fasttick();
    int proc_params();

private:
    short *sbuf, *spos, *end_sbuf;
    int sound_fd, sound_sbufsize;
};

