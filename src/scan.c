#include "scan.h"
#include "cl_utils.h"
#include <stdio.h>

/* Format strings:
** 1. A type
** 2. TILE_SIZE
** 3. A_tile type
** 4. a type
** 5. b type
** 6. OP1
*/
const char *_partial_scan_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global % s * A) {
  int row = get_global_id (1);
  int col = get_global_id (0);

  int local_row = get_local_id (1);
  int local_col = get_local_id (0);
  const int tile_size = % d;
  __local % s A_tile[tile_size][tile_size];

  if (col < a1)
    {
      A_tile[local_row][local_col] = A[col + a1 * row];
      barrier (CLK_LOCAL_MEM_FENCE);

      for (int offset = 1; offset < tile_size; offset *= 2)
        {
          if (local_col >= offset)
            {
              % s a = A_tile[local_row][local_col];
              % s b = A_tile[local_row][local_col - offset];
              A_tile[local_row][local_col] = % s;
            }
          barrier (CLK_LOCAL_MEM_FENCE);
        }
    }

  if (col < a1 && row < a2)
    {
      A[col + a1 * row] = A_tile[local_row][local_col];
    }
});

/* Format strings:
 * 1. A type
 * 2. b type
 * 3. a type
 * 4. OP1
 */
const char *_propagate_scan_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global % s * A, int stride) {
  int row = get_global_id (1);
  int col = get_global_id (0);

  int local_row = get_local_id (1);
  int local_col = get_local_id (0);

  int group_row = get_group_id (1);
  int group_col = get_group_id (0);
  __local % s b;

  int chunk = col / stride;
  if (col > stride - 1 && col < a1 && row < a2 && chunk & 1 != 0)
    {
      % s a = A[col + a1 * row];
      b = A[(col / stride) * stride - 1 + (a1 * row)];
      A[col + a1 * row] = % s;
    }
});

char *
get_partial_scan (const char *dtype, const char *op1)
{
  int size = snprintf (NULL, 0, _partial_scan_fmt, dtype, _tile_size, dtype,
                       dtype, dtype, op1);

  char *kernel = malloc (size + 1);
  if (!kernel)
    {
      handle_error ("Failed to allocate memory for kernel string");
    }

  int count = snprintf (kernel, size + 1, _partial_scan_fmt, dtype, _tile_size,
                        dtype, dtype, dtype, op1);
  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }

  return kernel;
}

char *
get_propagate_scan (const char *dtype, const char *op1)
{
  int size = snprintf (NULL, 0, _propagate_scan_fmt, dtype, dtype, dtype, op1);

  char *kernel = malloc (size + 1);
  if (!kernel)
    {
      handle_error ("Failed to allocate memory for kernel string");
    }

  int count = snprintf (kernel, size + 1, _propagate_scan_fmt, dtype, dtype,
                        dtype, op1);
  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }

  return kernel;
}

unsigned long long
scan (const char *op1, array A, cl_event *event)
{
  const char *dtype = TYPE_STR_FROM_ENUM (A.type);
  char *src_partials = get_partial_scan (dtype, op1);
  cl_kernel kernel_partials = TRY_COMPILE_KERNEL (src_partials);
  free (src_partials);
  SET_KERNEL_ARGS (kernel_partials, A);

  size_t local_size[] = { _tile_size, _tile_size, _tile_size };
  size_t global_size[]
      = { LOWEST_MULTIPLE_OF_TILE (A.dim1), LOWEST_MULTIPLE_OF_TILE (A.dim2),
          LOWEST_MULTIPLE_OF_TILE (A.dim3) };

  cl_event partials[BUFSIZE]; // Input > 2^256 is unlikely.
  int event_count = 0;
  CHECK_CL (clEnqueueNDRangeKernel (
      _queue, kernel_partials, ARRAY_NUM_DIMS (A), NULL, global_size,
      local_size, 0, NULL, &partials[event_count++]));

  char *src_propagate = get_propagate_scan (dtype, op1);
  cl_kernel kernel_propagate = TRY_COMPILE_KERNEL (src_propagate);
  free (src_propagate);
  int idx = SET_KERNEL_ARGS (kernel_propagate, A);

  for (int stride = _tile_size; stride < A.dim1; stride *= 2)
    {
      CHECK_CL (clSetKernelArg (kernel_propagate, idx, sizeof (int), &stride));

      CHECK_CL (clEnqueueNDRangeKernel (
          _queue, kernel_propagate, ARRAY_NUM_DIMS (A), NULL, global_size,
          local_size, 0, NULL, &partials[event_count++]));
    }

  unsigned long long time = 0;
  cl_command_queue_properties props = 0;
  CHECK_CL (clGetCommandQueueInfo (_queue, CL_QUEUE_PROPERTIES, sizeof (props),
                                   &props, NULL));
  if (event || !(props & CL_QUEUE_PROFILING_ENABLE))
    {
      CHECK_CL (
          clEnqueueMarkerWithWaitList (_queue, event_count, partials, event));
      return time;
    }

  for (int i = 0; i < event_count; i++)
    {
      time += GET_CL_EVENT_TIME (partials[i]);
    }

  return time;
}
