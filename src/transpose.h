#ifndef TRANSPOSE_H_
#define TRANSPOSE_H_

#include "cl_utils.h"

extern const char *_transpose_fmt;
char *_get_transpose (const char *dtype);
void transpose (array A, array B, cl_event *event);
#define _TRANSPOSE_ONE(A, B) transpose (A, B, NULL);
#define _TRANSPOSE_TWO(A, B, event) transpose (A, B, event)
#define TRANSPOSE(...)                                                        \
  _GETM_THREE (__VA_ARGS__, _TRANSPOSE_TWO, _TRANSPOSE_ONE) (__VA_ARGS__)

#endif // TRANSPOSE_H_
