#ifndef INNER_PRODUCT_H_
#define INNER_PRODUCT_H_

#include "cl_utils.h"

extern const char *_inner_product_fmt;
char *_get_inner_product (const char *dtype, const char *op1, const char *op2);
cl_kernel inner_product (const char *op1, const char *op2, array A, array B,
                         array C);

#endif // INNER_PRODUCT_H_
