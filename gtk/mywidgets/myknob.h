
#ifndef __MYKNOB_H
#define __MYKNOB_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#define MYKNOB(obj)		GTK_CHECK_CAST(obj, myknob_get_type(), MyKnob)
#define MYKNOB_CLASS(klass)	GTK_CHECK_CLASS_CAST(klass, myknob_get_type(), MyKnobClass)
#define IS_MYKNOB(obj)		GTK_CHECK_TYPE(obj, myknob_get_type())


typedef struct _MyKnob MyKnob;
typedef struct _MyKnobClass MyKnobClass;

struct _MyKnob {
    GtkWidget widget;

    GtkAdjustment *adjust;
    GdkFont *font;
    GdkGC *col0;
    int pressy, pressval, steps;
};

struct _MyKnobClass {
    GtkWidgetClass parent_class;
};


int myknob_get_type(void);
GtkWidget *myknob_new(int);
void myknob_set(MyKnob *, int);
void myknob_set_steps(MyKnob *, int);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__MYKNOB_H*/
