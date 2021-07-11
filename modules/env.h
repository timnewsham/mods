
class envmodule : public module {
public:
    envmodule(char *);
    ~envmodule();
    void slowtick(void);

private:
    const static int attack = 0, decay = 1, release = 2;
    int phase;
};

