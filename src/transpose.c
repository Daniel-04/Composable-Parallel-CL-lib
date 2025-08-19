#include "transpose.h"
#include "cl_utils.h"
#include <stdio.h>

/* Format strings:
** 1. A type
** 2. B type
** 3. TILE_SIZE
** 4. A_tile type
*/
const char *_transpose_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global const % s * A,
    const int b1, const int b2, const int b3, __global % s * B) {
  int row = get_global_id (1);
  int col = get_global_id (0);

  int local_row = get_local_id (1);
  int local_col = get_local_id (0);
  const int tile_size = % d;

  __local % s A_tile[tile_size][tile_size + 1];

  if (col < a1 && row < a2)
    {
      A_tile[local_row][local_col] = A[row * a1 + col];
    }
  barrier (CLK_LOCAL_MEM_FENCE);

  int transposed_col = get_group_id (1) * tile_size + local_col;
  int transposed_row = get_group_id (0) * tile_size + local_row;

  if (transposed_col < a2 && transposed_row < a1)
    {
      B[transposed_row * b1 + transposed_col] = A_tile[local_col][local_row];
    }
});

char *
get_transpose (const char *dtype)
{
  int size
      = snprintf (NULL, 0, _transpose_fmt, dtype, dtype, _tile_size, dtype);

  char *kernel = malloc (size + 1);
  if (!kernel)
    {
      handle_error ("Failed to allocate memory for kernel string");
    }

  int count = snprintf (kernel, size + 1, _transpose_fmt, dtype, dtype,
                        _tile_size, dtype);
  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }

  return kernel;
}

unsigned long long
transpose (array A, array B, cl_event *event)
{
  cl_event _event;
  const char *dtype = TYPE_STR_FROM_ENUM (B.type);
  char *src = get_transpose (dtype);
  cl_kernel kernel = TRY_COMPILE_KERNEL (src);
  free (src);
  SET_KERNEL_ARGS (kernel, A, B);

  size_t local_size[] = { _tile_size, _tile_size, _tile_size };
  size_t global_size[]
      = { LOWEST_MULTIPLE_OF_TILE (B.dim1), LOWEST_MULTIPLE_OF_TILE (B.dim2),
          LOWEST_MULTIPLE_OF_TILE (B.dim3) };
  CHECK_CL (clEnqueueNDRangeKernel (_queue, kernel, ARRAY_NUM_DIMS (B), NULL,
                                    global_size, local_size, 0, NULL,
                                    event ? event : &_event));

  unsigned long long time = 0;
  if (!event)
    {
      time = GET_CL_EVENT_TIME (_event);
    }

  return time;
}
