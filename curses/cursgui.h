
#define ROW_KNOB	0
#define ROW_PROMPT	6
#define ROW_MAIN	13
#define ROW_OCTAVE	14
#define ROW_KEYBOARD	15

#define KNOBS_WIDTH	50
#define KNOBS_VISIBLE	6

class cursgui : public gui {
public:
    cursgui();
    ~cursgui();

    int init(int *, char **);
    void run(void);
    void error(char *);
    void shutdown(void);
};

void printat(int r, int c, char *fmt, ...);

