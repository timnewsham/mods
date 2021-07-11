
class lfomodule : public module {
public:
    lfomodule(char *);
    ~lfomodule();
    void slowtick();

private:
    int theta;
};

