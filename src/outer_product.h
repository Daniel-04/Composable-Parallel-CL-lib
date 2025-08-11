#ifndef OUTER_PRODUCT_H_
#define OUTER_PRODUCT_H_

#include "cl_utils.h"

extern const char *_outer_product_fmt;
char *_get_outer_product (const char *dtype, const char *op1);
cl_kernel outer_product (const char *op1, array A, array B, array C);

#endif // OUTER_PRODUCT_H_
