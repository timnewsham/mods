
#ifdef __cplusplus
extern "C" {
#endif

GdkGC *alloc_color_gc(GtkWidget *widget, int red, int green, int blue);
void realize_window(GtkWidget *widget, int eventmask);

#ifdef __cplusplus
}
#endif

