
#include "gui.h"
#include "misc.h"

class gui *gui = 0;

void
set_gui(class gui *g)
{
    if(gui)
        die("set_gui: gui is already set!");

    gui = g;
}

