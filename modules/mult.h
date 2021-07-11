
class multmodule : public module {
public:
    multmodule(char *);
    ~multmodule();
    void slowtick(void);
    int proc_params(void);

private:
    int scalefactor;
};

