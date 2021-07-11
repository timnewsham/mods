
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "attr.h"

#define ABUFLEN 256

void
attrlist_add(struct attrlist **headp, char *name, char *val)
{
    struct attrlist *t;

    for(t = *headp; t; t = t->next) {
        if(strcmp(t->name, name) == 0)
            break;
    }

    if(!t) {
        t = new attrlist;
        t->next = *headp;
        *headp = t;
    }

    t->name = strdup(name);
    t->val = strdup(val);
}

char *
attrlist_find(struct attrlist **headp, char *name)
{
    struct attrlist *t;

    for(t = *headp; t; t = t->next) {
        if(strcmp(t->name, name) == 0)
            return t->val;
    }
    return 0;
}

char *
attrlist_string(struct attrlist **headp)
{
    static char buf[ABUFLEN + 1];
    struct attrlist *t;
    int pos, len, x;

    len = ABUFLEN;
    pos = 0;
    buf[0] = 0;
    for(t = *headp; t; t = t->next) {
        if((int)strlen(t->name) + (int)strlen(t->val) + 7 > len)
            die("attribute list is too long!");
        snprintf(buf + pos, len, "%s=%s; ", t->name, t->val);
        x = strlen(buf + pos);
        pos += x;
        len -= x;
    }
    return buf;
}

int
attrlist_parse(struct attrlist **headp, char *str)
{
    char *name, *val, *p;

    while(*str) {
        /* parse <spaces><name>=<val>; */
        while(isascii(*str) && isspace(*str))
            str++;
        if(!*str)
            break;

        name = str;
        p = strchr(name, '=');
        if(!p)
            return -1;
        *p++ = 0;

        val = p;
        p = strchr(val, ';');
        if(!p)
            return -1;
        *p++ = 0;
        str = p;

        attrlist_add(headp, name, val);
    }
    return 0;    
}

#ifdef notneeded
void
attrlist_rem(struct attrlist **headp, char *name)
{
    struct attrlist *t, *prevp;

    prevp = headp;
    for(t = *headp; t; t = t->next) {
        if(strcmp(t->name, name) == 0)
            return t->val;
        prevp = &t->next;
    }

    if(t) {
        *prevp = t->next;
        free(t->name);
        free(t->val);
        delete t;
    }
}
#endif

