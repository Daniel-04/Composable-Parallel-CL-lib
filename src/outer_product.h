#ifndef OUTER_PRODUCT_H_
#define OUTER_PRODUCT_H_

#include "cl_utils.h"

extern const char *_outer_product_fmt;
char *get_outer_product (const char *dtype, const char *op1);
void outer_product (const char *op1, array A, array B, array C,
                    cl_event *event);
#define _OUTER_PRODUCT_ONE(op1, A, B, C) outer_product (op1, A, B, C, NULL);
#define _OUTER_PRODUCT_TWO(op1, A, B, C, event)                               \
  outer_product (op1, A, B, C, event)
#define OUTER_PRODUCT(...)                                                    \
  _GETM_FIVE (__VA_ARGS__, _OUTER_PRODUCT_TWO,                                \
              _OUTER_PRODUCT_ONE) (__VA_ARGS__)

#endif // OUTER_PRODUCT_H_
