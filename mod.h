

// GENERIC definitions, this probably belongs elsewhere

typedef unsigned long long u64_t;	// I dont like using "long long"

const int sampfreq = 44100;
const int slowdiv = 128;	// must be a power of 2

// XXX this names should be scoped or changed to avoid conflicts
const int FAST = 1;
const int SLOW = 0;

// NOTE: some places in the code assume these values
// (for example, shifts of 16 places).  don't change these
// without adjusting the code elsewhere
const short SAMPMAX = 0x7fff;
const short SAMPMIN = 0x8000;

#define ARRAYCNT(arr)		(sizeof(arr) / sizeof((arr)[0]))


class module;
class guipeer;
struct descr_connect;
struct descr_param;

// describes a module type and how to make instances
struct descr_mod {
    char *name;
    struct descr_connect *connect;
    struct descr_param *param;
    int isfast;
    module *(*instantiate)(char *name);

    // for use by the gui
    void *data;
};

// a connection is either an input or an output
struct descr_connect {
    char *name;
    char *descr;
    int type;
};

#define CONTYPE_IN	0
#define CONTYPE_OUT	1
#define CONTYPE_SLOW	0
#define CONTYPE_FAST	2

typedef union {
    char *s;		// string
    int i;		// integer
} paramval;

// a parameter is something that only the user can tweak
struct descr_param {
    char *name;
    char type;
};

class module {
public:
    module(struct descr_mod *, char *);
    virtual ~module();

    void set_peer(guipeer *p) { peer = p; }
    int set_params(int nargs, char *args[]);
    int set_params_v(int dummy, ...);
    char *get_param_str(int);
    void set_attr(char *name, char *val);
    char *get_attr(char *name);
    int set_attr_str(char *str);
    char *get_attr_str(void);
    virtual void slowtick(void);
    virtual void fasttick(void);
    virtual char *get_state(void);
    virtual int set_state(char *);

protected:
    virtual int proc_params(void);

public:
    struct descr_mod *descr;
    guipeer *peer;
    char *name, *type;
    int fast;

protected:
    struct attrlist *attrs;
    int num_params;
    int num_connects;
    paramval *param;
    int *connect;
    char *conused;

    friend void add_wire(module *, int, module *, int);
    friend void rem_wire_common(struct wire *);
};

int connect_count(struct descr_connect *);
int param_count(struct descr_param *);
int find_connect(struct descr_mod *d, char *conname);

