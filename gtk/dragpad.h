
typedef enum {
    MOVED, 
    REMOVED,
    SELECTED,
    UNSELECTED,
} dragent_cbtype;

typedef struct _dragpad dragpad;
typedef struct _dragentry dragentry;
typedef void (*dragent_callback)(dragpad *, dragentry *, dragent_cbtype, void *);

struct _dragpad {
    GtkWidget *layout;
    GtkWidget *scrollwin;
    guint timeout;
    int width, height;

    GList *widgets;
    GList *selected;

    GtkWidget *drag_widget;
    int drag_x, drag_y;
    int drag_xslide, drag_yslide;
};

struct _dragentry {
    GtkWidget *widget;
    GList *callbacks;
    int x, y;
    int flags;
};

/* dragentry flags */
#define DE_FLAG_DRAG		0x0001		/* can be dragged */
#define DE_FLAG_OVERLAP		0x0002		/* can overlap */
#define DE_FLAG_SELECT		0x0004		/* can be selected */
#define DE_FLAG_SELECTED	0x0008		/* is currently selected */

#define FLAG_SET(x, f)		x |= (f)
#define FLAG_CLR(x, f)		x &= ~(f)
#define FLAG_ISSET(x, f)	(((x) & (f)) == (f))

dragpad * dragpad_new(void);
int dragpad_overlap(dragpad *x, dragentry *ent, int xpos, int ypos, int skipsel, int strict);
void dragpad_add(dragpad *x, GtkWidget *widget, int xpos, int ypos, int flags);
int dragpad_remove(dragpad *x, GtkWidget *widget);
int dragpad_getpos(dragpad *pad, GtkWidget *widget, int *xp, int *yp);
int dragpad_setpos(dragpad *pad, GtkWidget *widget, int x, int y);
dragentry *dragpad_getent(dragpad *, GtkWidget *);
int dragpad_addcallback(dragpad *pad, GtkWidget *widget, dragent_callback cb, void *ctx);
int dragpad_remcallback(dragpad *pad, GtkWidget *widget, int id);
void dragpad_refresh(dragpad *pad, int x1, int y1, int x2, int y2);

