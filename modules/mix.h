
class mixmodule : public module {
public:
    mixmodule(char *);
    ~mixmodule();
    void fasttick(void);
    int proc_params(void);

private:
    int sign;
};

