#ifndef REDUCE_H_
#define REDUCE_H_

#include "cl_utils.h"

extern const char *_reduce_1step_fmt;
char *_get_reduce_1step (const char *dtype, const char *op1);

#endif // REDUCE_H_
