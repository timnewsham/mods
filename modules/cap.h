
class capmodule : public module {
public:
    capmodule(char *);
    ~capmodule();
    void slowtick(void);
    int proc_params(void);

private:
    FILE *out;
    int thresh, len;
    int curlen;
};

