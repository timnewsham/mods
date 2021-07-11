
#include <stdio.h>
#include <stdlib.h>

#include "mod.h"
#include "wire.h"
#include "misc.h"
#include "conf.h"
#include "gui.h"
#include "version.h"

#include "audio.h" //XXX linux

#ifdef CURSGUI
#include "cursgui.h"
#endif
#ifdef GTKGUI
#include "gtkgui.h"
#endif

char version[] = SYNTH_VERSION;

static class gui *
make_gui(char *name)
{
#ifdef GTKGUI
    if(!name || strcmp(name, "gtk") == 0)
        return new gtkgui();
#endif
#ifdef CURSGUI
    if(!name || strcmp(name, "curses") == 0)
        return new cursgui();
#endif
    return 0;
}

static int
register_modules()
{
    // declare module descriptions as external
    #define MOD(x) x
    extern struct descr_mod
          #include "modlist_generic.h"
        ;

    // make a table of the module descriptions
    #undef MOD
    #define MOD(x) &x
    static struct descr_mod *descrs[] = {
          #include "modlist_generic.h"
        };

    return register_modtypes(descrs, ARRAYCNT(descrs));
}

static void
usage(char *progname)
{
    die("Usage:    %s -v\n"
        "          %s [-g gui] [guiargs] [archargs] [conffile [settings]]\n",
        progname, progname);

    // XXX display gui list, and gui and arch arguments
    exit(1);
}

/* XXX fixme: due to order of things, the menu of all modules is
 * created before arch_init.  so we need to do this first.  Better
 * would be to fix the gui to not build this menu until after the
 * gui is running
 */
static void
xxx()
{
    extern descr_mod descr_midikeymodule;
    static struct descr_mod *linux_mods[] = {
        &descr_midikeymodule
    };

    register_modtypes(linux_mods, ARRAYCNT(linux_mods));
}

// XXX linux initialization
static void
arch_init(int *argcp, char **argv)
{
    module *mod;
    char *snddev;
    int sndfragsize;

    // XXX should be parsed from args somehow
    snddev = "";
    sndfragsize = 6;

    mod = new audiomodule("audio");
    if(mod->set_params_v(0, sndfragsize, snddev) == -1)
        die("Error initializing audio module");
    return;
}

int
main(int argc, char **argv)
{
    class gui *g;
    char *conf = 0, *sfile = 0;
    char *guiname = 0;
    char *progname;

    if(register_modules() == -1)
        die("Error setting up modules!");
    xxx();

    progname = argv[0];
    if(argc > 1 && strcmp(argv[1], "-v") == 0)
        die("version: %s\n", SYNTH_VERSION);
    
    if(argc > 1 && strcmp(argv[1], "-g") == 0) {
        if(!argv[2])
            die("gui name not given in -g option");
        guiname = argv[2];
        argc -= 2;
        argv += 2;
    }
    g = make_gui(guiname);
    if(!g) {
        if(guiname)
            die("Could not find gui %s", guiname);
        else
            die("No default gui!");
    }
    set_gui(g);
    g->init(&argc, argv);

    arch_init(&argc, argv);

    if(argc > 1)
        conf = argv[1];
    if(argc > 2)
        sfile = argv[2];
    if(argc > 3 || (conf && *conf == '-'))
        usage(progname);

    if(conf && readconf(conf) == -1)
        die("Couldn't read conf file");
    if(sfile && load_state(sfile) == -1)
        die("Couldn't read settings file");

    g->run();
    return 0;
}

