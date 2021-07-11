// example.cpp
//	An example module
//
// This module shows the basic module framework.  In itself it does
// very little: copy the inputs to the outputs.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mod.h"
#include "example.h"


// This is the list of all parameters that are used by the module.
// It specifies the parameter name and the type.  These parameters are
// first set externally and then the proc_params method is invoked
// to tell the module to use the values.  The module may not be used
// until it is initialized with a call to proc_params.
static struct descr_param P[] = {
#define P_FOO 0
    {"foo", 's'},
#define P_BAR 1
    {"bar", 'i'},
    {0}
};

// This is the list of all connectors into and out of the module.
// It specifies the connectors name, description, and type.  The
// type specifies if the connector is an input or output and if it
// operates on a slow or fast tick.
static struct descr_connect C[] = {
#define C_SLOWIN 0
    {"slowin", "Slow Input", CONTYPE_IN | CONTYPE_SLOW},
#define C_FASTIN 1
    {"fastin", "Fast Input", CONTYPE_IN | CONTYPE_FAST},
#define C_SLOWOUT 2
    {"slowout", "Slow Output", CONTYPE_OUT | CONTYPE_SLOW},
#define C_FASTOUT 3
    {"fastout", "Fast Output", CONTYPE_OUT | CONTYPE_FAST},
    {0}
};

// We need a function that can instantiate this type of module.
// This is it.
static module *
B(char *name)
{
    return new examplemodule(name);
}

// We also need to supply a description of the module.  This has
// things like the name it can be instantiated by, its connectors and
// parameters and a function that can instantiate it.  This description
// is used to figure out how to build a module and set its parameters
// given its name and some string arguments, and may also be used
// by the user interface to prompt users for parameters and to show
// the module graphically.
struct descr_mod descr_examplemodule = { "example", C, P, SLOW, B };

// The constructor takes a name.  Most of the work is done by the
// super-class' constructor which just needs to know the name and
// the description of the module.  The constructor itself just needs
// to initialize variables.
examplemodule::examplemodule(char *name) :
    module(&descr_examplemodule, name)
{
    // initialize any state, parse any arguments
}

// The destructor just cleans up after anything the module may have
// allocated.
examplemodule::~examplemodule()
{
    // destroy anything we allocated
}

// The proc_params method is called when the module is first initialized
// and any time the user requests the modules parameters should be changed.
// By the time this is called, the parameters have all been parsed and
// filled in to the param[] array.  We just need to use the values in
// whatever way they should be used.
int
examplemodule::proc_params()
{
    static char *str = 0;
    int x;

    // read in the parameters and use them,  note that this method
    // can be called more than once, so if you allocated something
    // last time, you should free it before reallocating it again.
    if(str)
        free(str);
    str = strdup(param[P_FOO].s);
    if(!str)
        return -1;
    x = param[P_BAR].i;
    printf("example module parameters:  foo = %s, bar = %d\n", str, x);

    // return -1 if something went wrong, else zero
    return 0;
}

// The fast tick gets called every time a sample is to be generated as
// output.  It is usually used to create and consume audio data.
// The operations here should be optimized to run efficiently.
void
examplemodule::fasttick()
{
    // fastout = fastin, every fast tick
    connect[C_FASTOUT] = connect[C_FASTIN];
}

// The slow tick gets called much less often than the fast tick operation.
// It is used to generate and consume control information such as volume
// levels and pitches.  Often the slow tick method will compute more
// efficient settings used by the fast tick method.
void
examplemodule::slowtick()
{
    // slowout = slowin, every slow tick
    connect[C_SLOWOUT] = connect[C_SLOWIN];
}


