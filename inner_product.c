#include "inner_product.h"
#include <stdio.h>

/* Format strings:
** 1. A type
** 2. B type
** 3. C type
** 4. A_tile type
** 5. TILE_SIZE
** 6. TILE_SIZE
** 7. B_tile type
** 8. TILE_SIZE
** 9. TILE_SIZE
** 10. acc type
** 11. TILE_SIZE
** 12. TILE_SIZE
** 13. TILE_SIZE
** 14. TILE_SIZE
** 15. TILE_SIZE
** 16. OP1
** 17. OP2
*/
const char *_inner_product_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global const % s * A,
    const int b1, const int b2, const int b3, __global const % s * B,
    const int c1, const int c2, const int c3, __global % s * C) {
  int row = get_global_id (1);
  int col = get_global_id (0);

  int local_row = get_local_id (1);
  int local_col = get_local_id (0);

  __local % s A_tile[% d][% d];
  __local % s B_tile[% d][% d];

  % s acc = 0;

  for (int t = 0; t < (a1 + % d - 1) / % d; t++)
    {
      int tiled_col_A = t * % d + local_col;
      int tiled_row_B = t * % d + local_row;

      A_tile[local_row][local_col]
          = (row < a2 && tiled_col_A < a1) ? A[row * a1 + tiled_col_A] : 0;
      B_tile[local_row][local_col]
          = (tiled_row_B < b2 && col < b1) ? B[tiled_row_B * b1 + col] : 0;
      barrier (CLK_LOCAL_MEM_FENCE);

      for (int k = 0; k < % d; ++k)
        {
          acc = acc % s (A_tile[local_row][k] % s B_tile[k][local_col]);
        }
      barrier (CLK_LOCAL_MEM_FENCE);
    }

  if (row < c2 && col < c1)
    {
      C[row * c1 + col] = acc;
    }
});

/* const char *_inner_product_fmt = RAW (__kernel void entry ( */
/*     const int a1, const int a2, const int a3, __global const % s * A, */
/*     const int b1, const int b2, const int b3, __global const % s * B, */
/*     const int c1, const int c2, const int c3, __global % s * C) { */
/*   int row = get_global_id (1); */
/*   int col = get_global_id (0); */

/*   int local_row = get_local_id (1); */
/*   int local_col = get_local_id (0); */

/*   __local % s A_tile[% d][% d]; */
/*   __local % s B_tile[% d][% d]; */

/*   % s acc = 0; */

/*   for (int t = 0; t < (a1 + % d - 1) / % d; t++) */
/*     { */
/*       int tiled_col_A = t * % d + local_col; */
/*       int tiled_row_B = t * % d + local_row; */

/*       A_tile[local_row][local_col] */
/*           = (row < a2 && tiled_col_A < a1) ? A[row * a1 + tiled_col_A] : 0;
 */
/*       B_tile[local_row][local_col] */
/*           = (tiled_row_B < b2 && col < b1) ? B[tiled_row_B * b1 + col] : 0;
 */
/*       barrier (CLK_LOCAL_MEM_FENCE); */

/*       for (int k = 0; k < % d; ++k) */
/*         { */
/*           acc = acc % s (A_tile[local_row][k] % s B_tile[k][local_col]); */
/*         } */
/*       barrier (CLK_LOCAL_MEM_FENCE); */
/*     } */

/*   if (row < c2 && col < c1) */
/*     { */
/*       C[row * c1 + col] = acc; */
/*     } */
/* }); */

char *
_get_inner_product (const char *dtype, const char *op1, const char *op2)
{
  char *kernel = NULL;

  int count = asprintf (
      &kernel, _inner_product_fmt, dtype /* A type */, dtype /* B type */,
      dtype /* C type */, dtype /* A_tile type */, TILE_SIZE, TILE_SIZE,
      dtype /* B_tile type */, TILE_SIZE, TILE_SIZE, dtype /* acc type */,
      TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE, op1, op2);

  if (count == -1)
    {
      return NULL; /* ERROR */
    }

  return kernel;
}
