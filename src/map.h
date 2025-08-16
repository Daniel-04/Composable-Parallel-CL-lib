#ifndef MAP_H_
#define MAP_H_

#include "cl_utils.h"

extern const char *_map_fmt;
char *get_map (const char *dtype, const char *op1);
unsigned long long map (const char *op1, array A, array B, cl_event *event);
#define _MAP_ONE(op1, A, B) map (op1, A, B, NULL);
#define _MAP_TWO(op1, A, B, event) map (op1, A, B, event)
#define MAP(...) _GETM_FOUR (__VA_ARGS__, _MAP_TWO, _MAP_ONE) (__VA_ARGS__)

#endif // MAP_H_
