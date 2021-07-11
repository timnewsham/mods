
class splitmodule : public module {
public:
    splitmodule(char *);
    ~splitmodule();
    void slowtick(void);
    int proc_params(void);

private:
    int break1, break2, break3;
};

