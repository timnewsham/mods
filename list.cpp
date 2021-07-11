
// list.cpp
// 	procedural list operations
//
// yes, I know I could use STL, or even classes for this, but no.. I refuse.
// send holy-war fodder to /dev/null

#include "list.h"

// add node at head of list
void
list_addhead(struct genlist **headp, struct genlist **tailp, 
             struct genlist *node)
{
    node->next = *headp;
    *headp = node;

    // init tail if this is the first node on the list
    if(!*tailp)
        *tailp = node;
}

// add node at tail of list
void
list_addtail(struct genlist **headp, struct genlist **tailp, 
             struct genlist *node)
{
    // link after old tail if there is one, init head if this is the first node
    if(*tailp)
        (*tailp)->next = node;
    else
        *headp = node;

    *tailp = node;
    node->next = 0;
}

// unlink single instance of node on the list
int
list_rem(struct genlist **headp, struct genlist **tailp,
         struct genlist *node)
{
    struct genlist *t, *prev, **prevp;

    prevp = headp;
    prev = 0;
    for(t = *headp; t; t = t->next) {
        if(t == node) {
            *prevp = t->next;
            if(t == *tailp)
                *tailp = prev;
            return 0;
        }

        prevp = &t->next;
        prev = t;
    }
    return -1;
}

