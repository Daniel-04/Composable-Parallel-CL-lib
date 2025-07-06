/*
** TODO: isolate inner product code into standalone string,
** extract function signature into wrapper
*/
#include "inner_product.h"
#include <stdio.h>

/*
 * clEnqueueNDRangeKernel(queue, kern, 2, NULL, global_size, local_size, 0, NULL, &kernel_event);
 *                                     |  |     |            |           |  |     ↳ attach event to operation
 *                                     |  |     |            |           |  ↳ array of events to wait for
 *                                     |  |     |            |           ↳ number of events to wait for
 *                                     |  |     |            ↳ array of size of local dimension
 *                                     |  |     ↳ array of size of each dimension
 *                                     |  ↳ array of offsets for each dimension
 *                                     ↳ number of dimensions
*/

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
const char *_inner_product_fmt = RAW(__kernel void entry(
    const int M1, const int N2, const int N1, __global const %s *A,
    __global const %s *B, __global %s *C) {
    int row = get_global_id(1);
    int col = get_global_id(0);

    int local_row = get_local_id(1);
    int local_col = get_local_id(0);

    int group_row = get_group_id(1);
    int group_col = get_group_id(0);

    __local %s A_tile[%d][%d];
    __local %s B_tile[%d][%d];

    %s acc = 0.0f;

    for (int t = 0; t < (N1 + %d - 1) / %d; t++) {
        int tiled_col_A = t * %d + local_col;
        int tiled_row_B = t * %d + local_row;

        A_tile[local_row][local_col] =
            (row < M1 && tiled_col_A < N1) ? A[row * N1 + tiled_col_A] : 0.0f;
        B_tile[local_row][local_col] =
            (tiled_row_B < N1 && col < N2) ? B[tiled_row_B * N2 + col] : 0.0f;
        barrier(CLK_LOCAL_MEM_FENCE);

        for (int k = 0; k < %d; ++k) {
            acc = acc %s (A_tile[local_row][k] %s B_tile[k][local_col]);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (row < M1 && col < N2) {
        C[row * N2 + col] = acc;
    }
});

char *_get_inner_product(const char *dtype, const char *op1, const char *op2) {
    char *kernel = NULL;

    int count = asprintf(
        &kernel,
        _inner_product_fmt,
        dtype /* A type */,
        dtype /* B type */,
        dtype /* C type */,
        dtype /* A_tile type */,
        TILE_SIZE, TILE_SIZE,
        dtype /* B_tile type */,
        TILE_SIZE, TILE_SIZE,
        dtype /* acc type */,
        TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE, TILE_SIZE,
        op1, op2);

    if (count == -1) {
        return NULL; /* ERROR */
    }

    return kernel;
}
