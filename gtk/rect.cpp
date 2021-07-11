
#include "rect.h"

int
rect_overlap(int r1x1, int r1y1, int r1x2, int r1y2,
             int r2x1, int r2y1, int r2x2, int r2y2)
{
    if(r1x1 > r2x2 || r1y1 > r2y2 ||
       r2x1 > r1x2 || r2y1 > r1y2)
        return 0;
    return 1;
}

void
bounding_box(int x1, int y1, int x2, int y2,
             int *minxp, int *minyp, int *maxxp, int *maxyp)
{
    if(x1 < x2) {
        *minxp = x1;
        *maxxp = x2;
    } else {
        *minxp = x2;
        *maxxp = x1;
    }

    if(y1 < y2) {
        *minyp = y1;
        *maxyp = y2;
    } else {
        *minyp = y2;
        *maxyp = y1;
    }
}

