
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "mod.h"
#include "gui.h"
#include "misc.h"
#include "wire.h"
#include "attr.h"

module::module(struct descr_mod *d, char *myname)
{
    descr = d;
    type = strdup(descr->name);
    name = strdup(myname);
    fast = descr->isfast;
    peer = 0;
    attrs = 0;

    num_connects = connect_count(d->connect);
    num_params = param_count(d->param);
    connect = new int[num_connects];
    memset(connect, 0, num_connects * sizeof(int));
    conused = new char[num_connects];
    memset(conused, 0, num_connects * sizeof(char));
    param = new paramval[num_params];
    memset(param, 0, num_params * sizeof(paramval));

    // XXX isnt memset implied by new?

    add_module(this, fast);

    if(gui)
        peer = gui->new_module(this);
}

module::~module() 
{
    rem_module(this);
    if(peer)
        peer->notify(DELETED);

    free(name);
    free(type);
    delete [] connect;
    delete [] conused;
    delete [] param;
}

void
module::slowtick()
{
    // do nothing
}

void
module::fasttick()
{
    // do nothing
}


char *
module::get_state()
{
    return 0;
}

int
module::set_state(char *state)
{
    return -1;
}

int
module::proc_params()
{
    return 0;
}

char *
module::get_param_str(int i)
{
    if(i >= num_params)
        return 0;

    switch(descr->param[i].type) {
    case 'i':
        return num2str(param[i].i);
    case 's':
        return param[i].s;
    default:
        die("invalid type for get_param_str");
    }
    return 0;
}

int
module::set_params(int nargs, char *a[])
{
    char *end;
    int i;

    if(nargs != num_params)
        return -1;

    for(i = 0; i < nargs; i++) {
        if(!a[i])
            return -1;
        switch(descr->param[i].type) {
        case 'i':
            param[i].i = strtoul(a[i], &end, 10);
            if(*end)
                return -1;
            break;
        case 's':
            if(param[i].s)
                free(param[i].s);
            param[i].s = strdup(a[i]);
            break;
        default:
            die("bad parameter type in module::set_params");
        }
    }

    if(proc_params() == -1)
        return -1;

    if(peer)
        peer->notify(PARAM_CHANGED);
    return 0;
}

// arg..  stdarg requires at least 1 fixed argument
int
module::set_params_v(int dummy, ...)
{
    va_list ap;
    int i;

    // count arguments
    va_start(ap, dummy);
    for(i = 0; i < num_params; i++) {
        switch(descr->param[i].type) {
        case 'i':
            param[i].i = va_arg(ap, int);
            break;
        case 's':
            if(param[i].s)
                free(param[i].s);
            param[i].s = strdup(va_arg(ap, char *));
            break;
        default:
            die("bad parameter type in module::set_params_v");
        }
    }
    va_end(ap);

    if(proc_params() == -1)
        return -1;

    if(peer)
        peer->notify(PARAM_CHANGED);
    return 0;
}

void
module::set_attr(char *name, char *val)
{
    attrlist_add(&attrs, name, val);
}

char *
module::get_attr(char *name)
{
    return attrlist_find(&attrs, name);
}

int
module::set_attr_str(char *str)
{
    int ret;

    ret = attrlist_parse(&attrs, str);
    if(ret != -1 && peer)
        peer->notify(ATTR_CHANGED);

    return ret;
}

char *
module::get_attr_str(void)
{
    if(peer)
        peer->syncattr();
    return attrlist_string(&attrs);
}

int
connect_count(struct descr_connect *con)
{
    int n;

    for(n = 0; con[n].name; n++)
        continue;
    return n;
}

int
param_count(struct descr_param *par)
{
    int n;

    for(n = 0; par[n].name; n++)
        continue;
    return n;
}

int
find_connect(struct descr_mod *d, char *conname)
{
    int i;

    for(i = 0; d->connect[i].name; i++) {
        if(strcmp(d->connect[i].name, conname) == 0)
            return i;
    }
    return -1;
}

