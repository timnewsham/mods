#include "func.h"
#include "mod.h"

/* 0x200 samples of a sin function scaled to signed 16bit number */
static int sintab[CYCLE] = {
    0, 402, 804, 1206, 1607, 2009, 2410, 2811, 
    3211, 3611, 4011, 4409, 4807, 5205, 5601, 5997, 
    6392, 6786, 7179, 7571, 7961, 8351, 8739, 9126, 
    9511, 9895, 10278, 10659, 11038, 11416, 11792, 12166, 
    12539, 12909, 13278, 13645, 14009, 14372, 14732, 15090, 
    15446, 15799, 16150, 16499, 16845, 17189, 17530, 17868, 
    18204, 18537, 18867, 19194, 19519, 19840, 20159, 20474, 
    20787, 21096, 21402, 21705, 22004, 22301, 22594, 22883, 
    23169, 23452, 23731, 24006, 24278, 24546, 24811, 25072, 
    25329, 25582, 25831, 26077, 26318, 26556, 26789, 27019, 
    27244, 27466, 27683, 27896, 28105, 28309, 28510, 28706, 
    28897, 29085, 29268, 29446, 29621, 29790, 29955, 30116, 
    30272, 30424, 30571, 30713, 30851, 30984, 31113, 31236, 
    31356, 31470, 31580, 31684, 31785, 31880, 31970, 32056, 
    32137, 32213, 32284, 32350, 32412, 32468, 32520, 32567, 
    32609, 32646, 32678, 32705, 32727, 32744, 32757, 32764, 
    32767, 32764, 32757, 32744, 32727, 32705, 32678, 32646, 
    32609, 32567, 32520, 32468, 32412, 32350, 32284, 32213, 
    32137, 32056, 31970, 31880, 31785, 31684, 31580, 31470, 
    31356, 31236, 31113, 30984, 30851, 30713, 30571, 30424, 
    30272, 30116, 29955, 29790, 29621, 29446, 29268, 29085, 
    28897, 28706, 28510, 28309, 28105, 27896, 27683, 27466, 
    27244, 27019, 26789, 26556, 26318, 26077, 25831, 25582, 
    25329, 25072, 24811, 24546, 24278, 24006, 23731, 23452, 
    23169, 22883, 22594, 22301, 22004, 21705, 21402, 21096, 
    20787, 20474, 20159, 19840, 19519, 19194, 18867, 18537, 
    18204, 17868, 17530, 17189, 16845, 16499, 16150, 15799, 
    15446, 15090, 14732, 14372, 14009, 13645, 13278, 12909, 
    12539, 12166, 11792, 11416, 11038, 10659, 10278, 9895, 
    9511, 9126, 8739, 8351, 7961, 7571, 7179, 6786, 
    6392, 5997, 5601, 5205, 4807, 4409, 4011, 3611, 
    3211, 2811, 2410, 2009, 1607, 1206, 804, 402, 
};

/*
 * return 16 bit signed value for a 9 bit input value 
 */
int
intsin(int theta, int dutytheta)
{
    int val;

    val = sintab[theta & 0x00ff];
    if(theta & 0x0100)
        val = -val;
    return val;
}

int
intsquare(int theta, int dutytheta)
{
    /* square output = max until duty point, then min */
    if((theta & 0x1ff) < dutytheta)
        return SAMPMAX;
    return SAMPMIN;
}

int
intsaw(int theta, int dutytheta)
{
    /* sawtooth output = angle */

    /* 
     * probably not really necessary, but we cast to short
     * before returning int in order to properly sign-extend it
     */
    return (short) (theta << (16 - 9));
}

// NOTE: if you use this function at full scale and store it in
// a short, then there will be an occasional -32768 value where there
// should be a 32767 value, due to negate of -32768 being -32768 in 16 bits.
// this will never happen if the value is scaled down at all before being
// used (ie. never happen in practice)
int
inttri(int theta, int dutytheta)
{
    int x;

    /* 
     * triangle output = sawtooth of twice the freq, positive
     * for first half cycle, negative for second half 
     */

    /* short cast does sign extension */
    x = (short) ((theta + 0x80) << (16 - 9 + 1));
    if (theta & 0x100)
        return -x;
    return x;
}

