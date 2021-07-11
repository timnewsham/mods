
#ifndef __MYMODBOX_H
#define __MYMODBOX_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#define MYMODBOX(obj)		GTK_CHECK_CAST(obj, mymodbox_get_type(), MyModbox)
#define MYMODBOX_CLASS(klass)	GTK_CHECK_CLASS_CAST(klass, mymodbox_get_type(), MyModboxClass)
#define IS_MYMODBOX(obj)	GTK_CHECK_TYPE(obj, mymodbox_get_type())


typedef struct _MyModbox MyModbox;
typedef struct _MyModboxClass MyModboxClass;

struct _mbconnect {
    char *name;
    int ypos;
    int width;
};

struct _MyModbox {
    GtkWidget widget;

    GdkFont *font;
    struct _mbconnect *in, *out;
    char *name;
    int num_in, num_out;
    int namewidth, boxwidth, boxheight, width, height;
};

struct _MyModboxClass {
    GtkWidgetClass parent_class;

    void (*inputsel) (MyModbox *, int num, int out, GdkEvent *ev);
    void (*outputsel) (MyModbox *, int num, int out, GdkEvent *ev);
};

int mymodbox_get_type(void);
GtkWidget *mymodbox_new(char *, int, char **, int, char **);
int mymodbox_gettermpos(MyModbox *mb, int num, int out, int *xp, int *yp);
void mymodbox_set_name(MyModbox *mb, char *name);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__MYMODBOX_H*/
