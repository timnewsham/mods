
#include <stdio.h>
#include <stdarg.h>

#include "mod.h"
#include "gui.h"

void
die(char *fmt, ...)
{
    char buf[200];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf - 1, fmt, ap);
    va_end(ap);

    if(gui)
        gui->error(buf);
    else
        fprintf(stderr, "\n%s\n", buf);
    exit(1);
}

char *
num2str(int num)
{
    static char buf[14];

    sprintf(buf, "%d", num);
    return buf;
}

