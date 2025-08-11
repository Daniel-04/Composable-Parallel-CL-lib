#ifndef MAP_H_
#define MAP_H_

#include "cl_utils.h"

extern const char *_map_fmt;
char *_get_map (const char *dtype, const char *op1);
cl_kernel map (const char *op1, array A, array B);

#endif // MAP_H_
