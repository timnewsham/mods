
class summodule : public module {
public:
    summodule(char *);
    ~summodule();
    void slowtick(void);
    int proc_params(void);

private:
    int sign;
};

