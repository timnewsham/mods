
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mod.h"
#include "wire.h"
#include "gui.h"
#include "misc.h"
#include "list.h"

static struct modlist *allmods = 0, *allmodstail = 0;
static struct wire *allwires = 0, *allwirestail = 0;

void
add_module(module *m, int fast)
{
    struct modlist *n;

    n = new modlist;
    n->mod = m;
    if(fast)
        list_addhead((genlist**)&allmods, (genlist**)&allmodstail, (genlist*)n);
    else
        list_addtail((genlist**)&allmods, (genlist**)&allmodstail, (genlist*)n);
}

module *
find_module(char *name)
{
    struct modlist *t;

    for(t = allmods; t; t = t->next) {
       if(t->mod->name && strcmp(t->mod->name, name) == 0)
           return t->mod;
    }
    return 0; 
}

void
rem_module(module *m)
{
    struct modlist *t;
    struct wire *t2, *next;

    /* XXX not the most efficient, but it'll do for now */
    for(t = allmods; t; t = t->next) {
        if(t->mod == m)
            break;
    }
    if(!t)
        die("rem_module: module wasn't on list!");

    /* remove any wires attached to the module */
    for(t2 = allwires; t2; t2 = next) {
        next = t2->next;

        if(t2->frm == m || t2->to == m)
            rem_wire_common(t2);
    }

    if(list_rem((genlist**)&allmods, (genlist**)&allmodstail, (genlist*)t) == -1)
        die("rem_module: module wasn't on list!");
}

void
rem_all_modules()
{
    struct modlist *t, *next;

    for(t = allmods; t; t = next) {
        next = t->next;

        if(strcmp(t->mod->type, "audio") == 0)
            continue;
        if(strcmp(t->mod->type, "key") == 0)
            continue;
        delete t->mod;
    }
    return;
}

void
write_modules(FILE *fp)
{
    struct modlist *t;
    module *m;
    char *str;
    int i;

    for(t = allmods; t; t = t->next) {
        m = t->mod;
        fprintf(fp, "D %s %s", m->name, m->type);

        for(i = 0; (str = m->get_param_str(i)); i++)
            fprintf(fp, " %s", *str ? str : "\"\"");

        str = m->get_attr_str();
        if(str && *str)
            fprintf(fp, " {%s}", str);
        fprintf(fp, "\n");
    }
}

void
add_wire(module *frm, int frmidx, module *to, int toidx)
{
    struct wire *n = new wire;

    // sanity
    if(frmidx < 0 || frmidx >= connect_count(frm->descr->connect))
        die("wire from %s:%d - invalid index", frm->name, frmidx);
    if(toidx < 0 || toidx >= connect_count(to->descr->connect))
        die("wire to %s:%d - invalid index", to->name, toidx);
    if(!(frm->descr->connect[frmidx].type & CONTYPE_OUT))
        die("wire from %s:%s - not an output", frm->name, 
            frm->descr->connect[frmidx].name);
    if(to->descr->connect[toidx].type & CONTYPE_OUT)
        die("wire to %s:%s - not an input", to->name, 
            to->descr->connect[toidx].name);
    if(to->conused[toidx])
        die("wire to %s:%s - Input already connected", to->name, 
            to->descr->connect[toidx].name);
    to->conused[toidx] = 1;

    n->frm = frm;
    n->to = to;
    n->frmidx = frmidx;
    n->toidx = toidx;
    n->frmp = &frm->connect[frmidx];
    n->top = &to->connect[toidx];
    if((frm->descr->connect[frmidx].type & CONTYPE_FAST) &&
            (to->descr->connect[toidx].type & CONTYPE_FAST)) 
        n->fast = 1;
    else
        n->fast = 0;

    if(n->fast)
        list_addhead((genlist**)&allwires, (genlist**)&allwirestail, 
                     (genlist*)n);
    else
        list_addtail((genlist**)&allwires, (genlist**)&allwirestail, 
                     (genlist*)n);

    if(gui)
        n->peer = gui->new_wire(frm, frmidx, to, toidx);
}

void
rem_wire_common(struct wire *w)
{
    w->to->conused[w->toidx] = 0;
    list_rem((genlist**)&allwires, (genlist**)&allwirestail, (genlist*)w);

    if(w->peer)
        w->peer->notify(DELETED);

    free(w);
}

void
rem_wire(module *frm, int frmidx, module *to, int toidx)
{
    struct wire *t;

    // *sigh* we traverse the list twice, maybe my list primitives need
    // more work (ie. a function for doing equiv)
    for(t = allwires; t; t = t->next) {
        if(t->frm == frm && t->to == to && 
                t->frmidx == frmidx && t->toidx == toidx)
            break;
    }
    if(!t)
        die("rem_wire: wire wasn't on list!");

    rem_wire_common(t);
}

void
rem_all_wires()
{
    struct wire *t, *next;

    for(t = allwires; t; t = next) {
        next = t->next;
        rem_wire_common(t);
    }
}

void
write_wires(FILE *fp)
{
    struct wire *t;

    for(t = allwires; t; t = t->next) {
        fprintf(fp, "W %s:%s %s:%s\n",
                t->frm->name, t->frm->descr->connect[t->frmidx].name,
                t->to->name, t->to->descr->connect[t->toidx].name);
    }
}

void
slowtick()
{
    struct modlist *t;
    struct wire *tw;

    for(tw = allwires; tw; tw = tw->next) {
        if(!tw->fast) 
            *tw->top = *tw->frmp;
    }

    for(t = allmods; t; t = t->next)
        t->mod->slowtick();
}

void
fasttick()
{
    struct wire *tw;
    struct modlist *t;

    for(tw = allwires; tw; tw = tw->next) {
        if(!tw->fast)
            break;
        *tw->top = *tw->frmp;
    }

    for(t = allmods; t; t = t->next) {
        if(!t->mod->fast)
            break;
        t->mod->fasttick();
    }
}

void
drive()
{
    int tick;

    slowtick();
    for(tick = slowdiv; tick; tick--)
        fasttick();
}

int
save_state(char *fname)
{
    struct modlist *t;
    module *mod;
    char *state;
    FILE *fp;

    fp = fopen(fname, "w");
    if(!fp)
        return -1;

    for(t = allmods; t; t = t->next) {
        mod = t->mod;
        state = mod->get_state();
        if(state) 
            fprintf(fp, "%s %s: %s\n", mod->type, mod->name, state);
    }
    fclose(fp);
    return 0;
}

int
load_state(char *fname)
{
    char buf[256], *name, *type, *state, *p;
    module *mod;
    FILE *fp;

    fp = fopen(fname, "r");
    if(!fp)
        return -1;

    while(fgets(buf, sizeof buf - 1, fp)) {
        p = strchr(buf, '\n');
        if(p)
            *p = 0;

        type = buf;
        p = strchr(type, ' ');
        if(!p) {
            fclose(fp);
            return -1;
        }
        *p++ = 0;

        name = p;
        p = strchr(name, ':');
        if(!p) {
            fclose(fp);
            return -1;
        }
        *p++ = 0;

        if(*p == ' ')
            p++;
        state = p;
        
        mod = find_module(name);
        if(!mod || strcmp(mod->type, type) != 0) {
            fclose(fp);
            return -1;
        }

        if(mod->set_state(state) == -1) {
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);

    // XXX issue a general gui refresh?
    return 0;
}

