#ifndef TRANSPOSE_H_
#define TRANSPOSE_H_

#include "cl_utils.h"

extern const char *_transpose_fmt;
char *_get_transpose (const char *dtype);
cl_kernel transpose (array A, array B);

#endif // TRANSPOSE_H_
