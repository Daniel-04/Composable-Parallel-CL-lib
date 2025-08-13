#ifndef INNER_PRODUCT_H_
#define INNER_PRODUCT_H_

#include "cl_utils.h"

extern const char *_inner_product_fmt;
char *get_inner_product (const char *dtype, const char *op1, const char *op2);
void inner_product (const char *op1, const char *op2, array A, array B,
                    array C, cl_event *event);
#define _INNER_PRODUCT_ONE(op1, op2, A, B, C)                                 \
  inner_product (op1, op2, A, B, C, NULL);
#define _INNER_PRODUCT_TWO(op1, op2, A, B, C, event)                          \
  inner_product (op1, op2, A, B, C, event)
#define INNER_PRODUCT(...)                                                    \
  _GETM_SIX (__VA_ARGS__, _INNER_PRODUCT_TWO, _INNER_PRODUCT_ONE) (__VA_ARGS__)

#endif // INNER_PRODUCT_H_
