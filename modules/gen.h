
class genmodule : public module {
public:
    genmodule(char *);
    ~genmodule();
    void slowtick();
    void fasttick();

private:
    int adv, theta, dutytheta;
    int (*genfunc)(int, int);
};

