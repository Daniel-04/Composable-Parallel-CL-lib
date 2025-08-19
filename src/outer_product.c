#include "outer_product.h"
#include "cl_utils.h"
#include <stdio.h>

/* Format strings:
** 1. A type
** 2. B type
** 3. C type
** 4. TILE_SIZE
** 5. A_tile type
** 6. B_tile type
** 7. a type
** 8. b type
** 9. OP1
*/
const char *_outer_product_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global const % s * _A,
    const int b1, const int b2, const int b3, __global const % s * _B,
    const int c1, const int c2, const int c3, __global % s * C) {
  int global_row = get_global_id (1);
  int global_col = get_global_id (0);

  int local_row = get_local_id (1);
  int local_col = get_local_id (0);
  int row = local_row;
  int col = local_col;

  const int tile_size = % d;
  __local % s A[tile_size];
  __local % s B[tile_size];

  A[local_row] = _A[row];
  B[local_col] = _B[col];
  barrier (CLK_LOCAL_MEM_FENCE);

  if (global_col < c1 && global_row < c2)
    {
      % s a = A[local_row];
      % s b = B[local_col];
      C[row * c1 + col] = % s;
    }
});

char *
get_outer_product (const char *atype, const char *btype, const char *ctype,
                   const char *op1)
{
  int size = snprintf (NULL, 0, _outer_product_fmt, atype, btype, ctype,
                       _tile_size, atype, btype, atype, btype, op1);

  char *kernel = malloc (size + 1);
  if (!kernel)
    {
      handle_error ("Failed to allocate memory for kernel string");
    }

  int count = snprintf (kernel, size + 1, _outer_product_fmt, atype, btype,
                        ctype, _tile_size, atype, btype, atype, btype, op1);
  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }

  return kernel;
}

unsigned long long
outer_product (const char *op1, array A, array B, array C, cl_event *event)
{
  cl_event _event;
  char *src = get_outer_product (TYPE_STR_FROM_ENUM (A.type),
                                 TYPE_STR_FROM_ENUM (B.type),
                                 TYPE_STR_FROM_ENUM (C.type), op1);
  cl_kernel kernel = TRY_COMPILE_KERNEL (src);
  free (src);
  SET_KERNEL_ARGS (kernel, A, B, C);

  size_t local_size[] = { _tile_size, _tile_size, _tile_size };
  size_t global_size[]
      = { LOWEST_MULTIPLE_OF_TILE (C.dim1), LOWEST_MULTIPLE_OF_TILE (C.dim2),
          LOWEST_MULTIPLE_OF_TILE (C.dim3) };
  CHECK_CL (clEnqueueNDRangeKernel (_queue, kernel, ARRAY_NUM_DIMS (C), NULL,
                                    global_size, local_size, 0, NULL,
                                    event ? event : &_event));

  unsigned long long time = 0;
  cl_command_queue_properties props = 0;
  CHECK_CL (clGetCommandQueueInfo (_queue, CL_QUEUE_PROPERTIES, sizeof (props),
                                   &props, NULL));
  if (event || !(props & CL_QUEUE_PROFILING_ENABLE))
    return time;

  time = GET_CL_EVENT_TIME (_event);

  return time;
}
