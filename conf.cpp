
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mod.h"
#include "conf.h"
#include "misc.h"
#include "wire.h"

#define MAXWORDS 10

static int lineno = 0;

struct modtype_list *all_modtypes = 0;

int
register_modtypes(struct descr_mod *mods[], int cnt)
{
    struct modtype_list *n, *t;
    int i;

    for(i = 0; i < cnt; i++) {
        // sanity.. no duplicate names
        for(t = all_modtypes; t; t = t->next) {
            if(strcmp(mods[i]->name, t->modtype->name) == 0)
                return -1;
        }

        // XXX sanity, check out param types

        // add it to the list
        n = new modtype_list;
        n->modtype = mods[i];
        n->next = all_modtypes;
        all_modtypes = n;
    }
    return 0;
}

module *
build_modtype(char *tname, char *name)
{
    struct modtype_list *t;
    for(t = all_modtypes; t; t = t->next) {
        if(strcmp(t->modtype->name, tname) == 0)
            break;
    }
    if(!t)
        return 0;

    return t->modtype->instantiate(name);
}


static int
readline(FILE *in, char *argv[], int maxargs)
{
    static char buf[200];
    int argc;
    char *p, *start, quote;

    if(fgets(buf, sizeof buf - 1, in) == 0)
        return -1;
    lineno++;

    p = strchr(buf, '#');
    if(p)
        *p = 0;

    p = strchr(buf, '\n');
    if(p)
        *p = 0;

    // parse out words into a vector of words
    argc = 0;
    p = buf;
    while(*p) {
        // skip spaces
        while(*p && isascii(*p) && isspace(*p))
            p++;
        if(!*p)
            break;

        if(argc == maxargs - 1)
            die("line %d: too many words\n", lineno);

        // determine quote mode
        if(*p == '{')
            quote = '}';
        else if(*p == '"')
            quote = '"';
        else
            quote = 0;

        // seperate out word
        if(quote) {
            if(quote == '"')
                p++;
            start = p;
            while(*p && *p != quote)
                p++;
            if(!*p)
                die("line %d: missing end quote\n", lineno);
            if(quote == '}')
                p++;

            *p++ = 0;
        } else {
            start = p;
            while(*p && !(isascii(*p) && isspace(*p)))
                p++;
            if(*p)
                *p++ = 0;
        }

        argv[argc++] = start;
    }

    return argc;
}

static int
defcmd(char *name, char *type, int nargs, char *args[])
{
    module *mod;
    char *attr;
    int x;

    // XXX check for duplicate name before making new module

    attr = 0;
    if(nargs >= 1 && args[nargs - 1][0] == '{') {
        attr = args[nargs - 1];
        nargs --;

        x = strlen(attr);
        if(attr[0] != '{' || attr[x - 1] != '}')
            die("line %d: bad attributes format\n", lineno);
        attr[x - 1] = 0;
        attr++;
    }

    mod = find_module(name);
    if(mod && strcmp(mod->type, type) != 0)
        die("line %d: module %s already exists with different type\n",
            lineno, name);

    if(!mod) {
        mod = build_modtype(type, name);
        if(!mod)
            die("line %d: unknown type %s\n", lineno, type);
    }

    if(attr && mod->set_attr_str(attr) == -1)
        die("line %d: bad attributes\n", lineno);

    if(mod->set_params(nargs, args) == -1)
        die("line %d: bad arguments\n", lineno);
    return 0;
}

static int
parsecon(char *con, module **modp, int *portp)
{
    module *mod;
    char *p, *endp;
    int port;

    p = strchr(con, ':');
    if(!p || p == con || !*(p + 1)) {
        die("line %d: bad connector name %s\n", lineno, con);
    }
    *p = 0;
    p ++;

    mod = find_module(con);
    if(!mod) {
        die("line %d: module %s not found\n", lineno, con);
    }

    port = strtoul(p, &endp, 10);
    if(*endp) {
        port = find_connect(mod->descr, p);
        if(port == -1) {
            die("line %d: connector %s not found\n", lineno, p);
        }
    }

    *modp = mod;
    *portp = port;
    return 0;
}

static int
wirecmd(char *frm, char *to)
{
    module *frmmod, *tomod;
    int frmport, toport;

    if(parsecon(frm, &frmmod, &frmport) == -1 ||
            parsecon(to, &tomod, &toport) == -1)
        return -1;

    // XXX we should check for bad wires here, and complain
    add_wire(frmmod, frmport, tomod, toport);
    return 0;
}

static int
filecmd(char *file)
{
    return load_state(file);
}

int
writeconf(char *fname)
{
    FILE *fp;

    fp = fopen(fname, "w");
    if(!fp)
        die("writing %s: %s", fname, strerror(errno));

    fprintf(fp, "\n\n");
    fprintf(fp, "# Definitions of all modules\n");
    write_modules(fp);

    fprintf(fp, "\n\n");
    fprintf(fp, "# Wiring between modules\n");
    write_wires(fp);

    fclose(fp);
    return 0;
}

int
readconf(char *fname)
{
    char *argv[MAXWORDS];
    FILE *fp;
    int argc;

    fp = fopen(fname, "r");
    if(!fp) {
        die("opening %s: %s", fname, strerror(errno));
    }

    lineno = 0;
    while((argc = readline(fp, argv, MAXWORDS)) >= 0) {
        if(argc == 0)
            continue;

        if(strcmp(argv[0], "D") == 0) {
            if(argc < 3)
                die("line %d: bad define command\n", lineno);
            if(defcmd(argv[1], argv[2], argc - 3, argv + 3) == -1)
                return -1;
        } else if(strcmp(argv[0], "W") == 0) {
            if(argc != 3)
                die("line %d: bad wire command\n", lineno);
            if(wirecmd(argv[1], argv[2]) == -1)
                return -1;
        } else if(strcmp(argv[0], "F") == 0) {
            if(argc != 2)
                die("line %d: bad file command\n", lineno);
             if(filecmd(argv[1]) == -1)
                 return -1;
        } else {
            die("line %d: Unknown command %s\n", 
                    lineno, argv[0]);
        }
    }

    fclose(fp);
    return 0;
}


