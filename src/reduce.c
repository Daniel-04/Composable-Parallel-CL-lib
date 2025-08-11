#include "reduce.h"
#include <stdio.h>

/* Format strings:
 * 1. A type
 * 2. TILE_SIZE
 * 3. A_tile type
 * 4. OP1
 */
const char *_reduce_1step_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global % s * A) {
  int idx = get_global_id (0);
  int local_idx = get_local_id (0);
  int group_idx = get_group_id (0);
  int total = a1 * a2 * a3;
  const int tile_size = % d;
  __local % s A_tile[tile_size];

  if (idx < total)
    {
      A_tile[local_idx] = A[idx];
      barrier (CLK_LOCAL_MEM_FENCE);

      for (int offset = tile_size / 2; offset > 0; offset >>= 1)
        {
          if (local_idx < offset && (idx + offset) < total)
            {
              A_tile[local_idx]
                  = A_tile[local_idx] % s A_tile[local_idx + offset];
            }
          barrier (CLK_LOCAL_MEM_FENCE);
        }

      if (local_idx == 0)
        {
          A[group_idx] = A_tile[0];
        }
    }
});

char *
_get_reduce_1step (const char *dtype, const char *op1)
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

cl_kernel
reduce_1step (const char *op1, array A)
{
  cl_kernel kernel = NULL;

  cl_int err;
  const char *dtype = TYPE_STR_FROM_ENUM (A.type);
  char *src = _get_reduce_1step (dtype, op1);
  cl_program program = CHECK_CL (
      clCreateProgramWithSource (_context, 1, (const char **)&src, NULL, &err),
      err);
  TRY_BUILD_PROGRAM (program);
  free (src);

  kernel = CHECK_CL (clCreateKernel (program, "entry", &err), err);
  clReleaseProgram (program);

  SET_KERNEL_ARGS (kernel, A);

  return kernel;
}
