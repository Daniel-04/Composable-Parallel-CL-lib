#include "reduce.h"
#include "cl_utils.h"
#include <stdio.h>

/* Format strings:
 * 1. A type
 * 2. TILE_SIZE
 * 3. A_tile type
 * 4. OP1
 */
const char *_reduce_1step_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global % s * A) {
  int row = get_global_id (1);
  int col = get_global_id (0);

  int local_row = get_local_id (1);
  int local_col = get_local_id (0);

  int group_row = get_group_id (1);
  int group_col = get_group_id (0);
  const int tile_size = % d;
  __local % s A_tile[tile_size][tile_size];

  if (col < a1)
    {
      A_tile[local_row][local_col] = A[col + a1 * row];
      barrier (CLK_LOCAL_MEM_FENCE);

      for (int offset = get_local_size (0) / 2; offset > 0; offset >>= 1)
        {
          if (local_col < offset && (col + offset) < a1)
            {
              A_tile[local_row][local_col]
                  = A_tile[local_row][local_col]
                    % s A_tile[local_row][local_col + offset];
            }

          barrier (CLK_LOCAL_MEM_FENCE);
        }

      if (local_col == 0 && row < a2)
        {
          A[group_col + a1 * row] = A_tile[local_row][0];
        }
    }
});

char *
get_reduce_1step (const char *dtype, const char *op1)
{
  char *kernel = NULL;

  int count
      = asprintf (&kernel, _reduce_1step_fmt, dtype, _tile_size, dtype, op1);

  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }
  return kernel;
}

void
reduce (const char *op1, array A, cl_event *event)
{
  cl_int err;
  const char *dtype = TYPE_STR_FROM_ENUM (A.type);
  char *src = get_reduce_1step (dtype, op1);
  cl_program program = CHECK_CL (
      clCreateProgramWithSource (_context, 1, (const char **)&src, NULL, &err),
      err);
  TRY_BUILD_PROGRAM (program);
  free (src);

  cl_kernel kernel = CHECK_CL (clCreateKernel (program, "entry", &err), err);
  clReleaseProgram (program);

  SET_KERNEL_ARGS (kernel, A);

  size_t local_size[] = { _tile_size, _tile_size, _tile_size };
  size_t global_size[]
      = { LOWEST_MULTIPLE_OF_TILE (A.dim1), LOWEST_MULTIPLE_OF_TILE (A.dim2),
          LOWEST_MULTIPLE_OF_TILE (A.dim3) };

  for (; global_size[0] > _tile_size;
       global_size[0] = (global_size[0] + _tile_size - 1) / _tile_size)
    {
      CHECK_CL (clEnqueueNDRangeKernel (_queue, kernel, ARRAY_NUM_DIMS (A),
                                        NULL, global_size, local_size, 0, NULL,
                                        event));
    }
  if (global_size[0] > 1)
    {
      CHECK_CL (clEnqueueNDRangeKernel (_queue, kernel, ARRAY_NUM_DIMS (A),
                                        NULL, global_size, NULL, 0, NULL,
                                        event));
    }
}
