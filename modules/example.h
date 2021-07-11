
// The interface definition of the module.  It always inherits from
// the module class. 
class examplemodule : public module {
public:
    // A constructor and destructor should be defined.
    examplemodule(char *);
    ~examplemodule();

    // this method is called to initialize the module and whenever
    // the module parameters have changed.
    int proc_params(void);

    // The fasttick and slowtick methods should be overridden if
    // the module needs them, otherwise they may be left out.
    void slowtick(void);
    void fasttick(void);

private:
    // any private module state
};

