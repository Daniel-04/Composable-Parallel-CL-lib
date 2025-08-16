#include "map.h"
#include "cl_utils.h"
#include <stdio.h>

/* Format strings:
 * 1. A type
 * 2. B type
 * 3. OP1
 */

const char *_map_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global const % s * A,
    const int b1, const int b2, const int b3, __global % s * B) {
  int idx = get_global_id (0);
  int total = a1 * a2 * a3;
  if (idx < total)
    {
      B[idx] = % s (A[idx]);
    }
});

char *
get_map (const char *dtype, const char *op1)
{
  int size = snprintf (NULL, 0, _map_fmt, dtype, dtype, op1);

  char *kernel = malloc (size + 1);
  if (!kernel)
    {
      handle_error ("Failed to allocate memory for kernel string");
    }

  int count = snprintf (kernel, size + 1, _map_fmt, dtype, dtype, op1);
  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }

  return kernel;
}

unsigned long long
map (const char *op1, array A, array B, cl_event *event)
{
  cl_event _event;
  const char *dtype = TYPE_STR_FROM_ENUM (B.type);
  char *src = get_map (dtype, op1);
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
