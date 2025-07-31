#include "outer_product.h"
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

/* const char *_outer_product_fmt = RAW (__kernel void entry ( */
/*     const int a1, const int a2, const int a3, __global const % s * A, */
/*     const int b1, const int b2, const int b3, __global const % s * B, */
/*     const int c1, const int c2, const int c3, __global % s * C) { */
/*   int row = get_global_id (1); */
/*   int col = get_global_id (0); */
/*   int tile_size = % d; */

/*   int tile_row = row * tile_size; */
/*   int tile_col = col * tile_size; */

/*   for (int i = 0; i < tile_size; i++) */
/*     for (int j = 0; j < tile_size; j++) */
/*       { */
/*         { */
/*           int curr_row = tile_row + i; */
/*           int curr_col = tile_col + j; */
/*           C[curr_row * c1 + curr_col] = A[curr_row] % s B[curr_col]; */
/*         } */
/*       } */
/* }); */

char *
_get_outer_product (const char *dtype, const char *op1)
{
  char *kernel = NULL;

  int count = asprintf (&kernel, _outer_product_fmt, dtype, dtype, dtype, op1);

  if (count == -1)
    {
      return NULL; /* ERROR */
    }

  return kernel;
}
