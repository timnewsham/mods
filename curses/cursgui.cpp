
#include <stdio.h>
#include <stdarg.h>
#include <curses.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>

#include "gui.h"
#include "mod.h"
#include "wire.h"
#include "misc.h"
#include "conf.h"

#include "cursgui.h"
#include "curskey.h"
#include "cursknob.h"

extern struct descr_mod descr_cursknobmodule;

static struct descr_mod *curs_modules[] = {
    &descr_cursknobmodule,
};

static int curs_init = 0;
static WINDOW *win = 0;
static curskeymodule *keymod = 0;

static void
curs_end(void)
{
    if(curs_init) {
        echo();
        nocbreak();
        endwin();
    }
    curs_init = 0;
    return;
}

static void
done(int sig)
{
    curs_end();
    if(sig)
        printf("dying on signal %d\n", sig);
    exit(0);
}

/* NOTE: limited buffer size */
void
printat(int r, int c, char *fmt, ...)
{
    char buf[200];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf - 1, fmt, ap);
    va_end(ap);
    mvaddstr(r, c, buf);
    refresh();
    return;
}

static char *
prompt_filename(char *msg)
{
    static char buf[50];
    int ch, pos;

    move(ROW_PROMPT, 3);
    clrtoeol();
    addstr(msg);
    refresh();

    nocbreak();
    echo();
    pos = 0;
    while((ch = getch()) != EOF && ch != '\r' && ch != '\n') {
        buf[pos++] = ch;
        if(pos >= (int)sizeof buf - 2)
            break;
    }
    buf[pos++] = 0;
    noecho();
    cbreak();

    move(ROW_PROMPT, 3);
    clrtoeol();
    refresh();
    return buf;
}

static void
proc_funckey(int ch)
{
    char *fname;

    if(ch == KEY_UP)
        move_curknob(-1);
    if(ch == KEY_DOWN)
        move_curknob(+1);
    if(ch == KEY_LEFT)
        curknob_adjust(-1);
    if(ch == KEY_RIGHT)
        curknob_adjust(+1);

    if(ch == KEY_F(1)) {
        fname = prompt_filename("Load configuration file: ");
        if(!fname || !*fname)
            return;
        rem_all_modules();
        if(readconf(fname) == -1) {
            printat(ROW_PROMPT, 3, "Load configuration failed...");
        } else {
            printat(ROW_PROMPT, 3, "Loaded configuration...");
            move_curknob(-1);
        }
    }
    if(ch == KEY_F(2)) {
        fname = prompt_filename("Load state file: ");
        if(!fname || !*fname)
            return;
        if(load_state(fname) == -1)
            printat(ROW_PROMPT, 3, "Load state failed...");
        else
            printat(ROW_PROMPT, 3, "Loaded state...");
    }
    if(ch == KEY_F(3)) {
        fname = prompt_filename("Save state as: ");
        if(!fname || !*fname)
            return;
        if(save_state(fname) == -1)
            printat(ROW_PROMPT, 3, "Save state failed...");
        else
            printat(ROW_PROMPT, 3, "Saved state...");
    }
}

static void
poll_keybd(void)
{
    fd_set rd;
    struct timeval timeo;
    int n, ch;

    FD_ZERO(&rd);
    FD_SET(0, &rd);
    timeo.tv_sec = 0;
    timeo.tv_usec = 0;
    n = select(0 + 1, &rd, 0, 0, &timeo);
    if(n == 1 && FD_ISSET(0, &rd)) {
        ch = getch();
        if(ch > KEY_CODE_YES)
            proc_funckey(ch);
        else
            keymod->proc_key(ch);
    }

    return;
}

static void
drawkeyboard()
{
    static char *l[] = { 
          " sd ghj l;", "zxcvbnm,./",
          " 23 567 90 =", "qwertyuiop[]" };
    int i, j, row, col;

    attron(A_BOLD | A_REVERSE);
    for(j = 0; j < 4; j++) {
        for(i = 0; l[j][i]; i++) {
            if(l[j][i] == ' ')
                continue;
            col = i * 2 + 35;
            row = ROW_KEYBOARD + j * 2;

            if(j & 1) {
                col++;
            } else {
                attron(A_REVERSE);
            }
            if(j >= 2) {   
                row++;
                col += 7 * 2;
            }
            attron(A_BOLD);
            mvaddch(row, col, ' ');
            mvaddch(row + 1, col, l[j][i]);
            attroff(A_REVERSE | A_BOLD);
            mvaddch(row, col - 1, '|');
            mvaddch(row + 1, col - 1, '|');
            mvaddch(row, col + 1, '|');
            mvaddch(row + 1, col + 1, '|');
        }
    }
    attroff(A_REVERSE);

    mvaddstr(ROW_KEYBOARD + 0, 4, "KEYS:");
    mvaddstr(ROW_KEYBOARD + 1, 4, "F1     - Load Configuration");
    mvaddstr(ROW_KEYBOARD + 2, 4, "F2     - Load State");
    mvaddstr(ROW_KEYBOARD + 3, 4, "F3     - Save State");
    mvaddstr(ROW_KEYBOARD + 4, 4, "Arrows - Adjust Knobs");
    mvaddstr(ROW_KEYBOARD + 5, 4, "+/-    - Adjust Octave");
    refresh();
}

cursgui::cursgui()
{
}

cursgui::~cursgui()
{
}

int
cursgui::init(int *argcp, char **argv)
{
    register_modtypes(curs_modules, ARRAYCNT(curs_modules));

    signal(SIGINT, done);
    signal(SIGBUS, done);
    signal(SIGQUIT, done);

    win = initscr();
    keypad(win, 1);
    cbreak();
    noecho();
    curs_init = 1;

    drawkeyboard();
    keymod = new curskeymodule("key");
    if(keymod->set_params_v(0) == -1)
        die("Error initializing curses key module");
    move_curknob(-1);
    
    return 0;
}

void
cursgui::run()
{
    for(;;) {
        poll_keybd();
        drive();
    }
    return;
}

void
cursgui::error(char *mesg)
{
    curs_end();
    printf("%s\n", mesg);
    exit(1);
}

void
cursgui::shutdown()
{
    curs_end();
    exit(0);
}

