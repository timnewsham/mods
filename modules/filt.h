
class filtmodule : public module {
public:
    filtmodule(char *);
    ~filtmodule();
    void fasttick(void);
    void slowtick(void);
    int proc_params(void);

private:
    //static const int bandpass = 0, lowpass = 1, hipass = 2, notch = 3;
    int type;
    float last_fc, last_res;

    // the actual filter state and coefficients
    float store1, store2;
    float a1, a2;
    float b0, b1, b2;
};

