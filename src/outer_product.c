#include "outer_product.h"
#include "cl_utils.h"
#include <stdio.h>

/* Format strings:
** 1. A type
** 2. B type
** 3. C type
** 4. OP1
*/
const char *_outer_product_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global const % s * A,
    const int b1, const int b2, const int b3, __global const % s * B,
    const int c1, const int c2, const int c3, __global % s * C) {
  int row = get_global_id (1);
  int col = get_global_id (0);
  if (col < c1 && row < c2)
    {
      C[row * c1 + col] = A[row] % s B[col];
    }
});

char *
_get_outer_product (const char *dtype, const char *op1)
{
  char *kernel = NULL;

  int count = asprintf (&kernel, _outer_product_fmt, dtype, dtype, dtype, op1);

  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }

  return kernel;
}
