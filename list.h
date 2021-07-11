
// generic linked list - any list with a next pointer as the first element
struct genlist {
    struct genlist *next;
};

void list_addhead(struct genlist **, struct genlist **, struct genlist *);
void list_addtail(struct genlist **, struct genlist **, struct genlist *);
int list_rem(struct genlist **, struct genlist **, struct genlist *);

