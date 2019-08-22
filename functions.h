#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <math.h>

inline bool checkAdjoin(int x1, int y1, int x2, int y2)
{
    return abs(x1 - x2) + abs(y1 - y2) == 1;
}


#endif // FUNCTIONS_H
