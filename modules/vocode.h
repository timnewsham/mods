
// with 7*128 samples we get a window of 20msec size
#define SAMPLES		(slowdiv * 7)
#define OVERLAP		150

class vocodemodule : public module {
public:
    vocodemodule(char *);
    ~vocodemodule();
    void fasttick(void);
    void slowtick(void);
    int proc_params(void);

private:
    float sample[SAMPLES];
    float *store, *k, *autocorr;
    int order, valid, pos, voiced;
};

