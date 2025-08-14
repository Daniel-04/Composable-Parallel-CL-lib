#ifndef REDUCE_H_
#define REDUCE_H_

#include "cl_utils.h"

extern const char *_reduce_1step_fmt;
char *get_reduce_1step (const char *dtype, const char *op1);
void reduce (const char *op1, array A, cl_event *event);
#define _REDUCE_ONE(op1, A) reduce (op1, A, NULL);
#define _REDUCE_TWO(op1, A, event) reduce (op1, A, event)
#define REDUCE(...)                                                           \
  _GETM_THREE (__VA_ARGS__, _REDUCE_TWO, _REDUCE_ONE) (__VA_ARGS__)

#endif // REDUCE_H_
