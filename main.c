#include "cl_utils.h"
#include "inner_product.h"
#include "map.h"
#include "outer_product.h"
#include <CL/cl.h>
#include <stdio.h>

int
main ()
{
  cl_int err;
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[]
      = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  setup_cl (&platform, &device, &context, &queue, props);

  int n = 4096;
  int a1 = n;
  int a2 = n;
  int b1 = n;
  int b2 = n;

  int c1 = n;
  int c2 = n;
  array A = ALLOC_ARRAY (int, CL_MEM_READ_ONLY, a1, a2);
  array B = ALLOC_ARRAY (int, CL_MEM_READ_ONLY, b1, b2);
  array C = ALLOC_ARRAY (int, CL_MEM_READ_WRITE, c1, c2);

  int i = 0;
  for (; i < ARRAY_SIZE ((A)); i++)
    {
      ((int *)A.host)[i] = i + 1;
    }

  for (int j = 0; j < ARRAY_SIZE ((B)); j++)
    {
      i++;
      ((int *)B.host)[j] = i;
    }

  SYNC_ARRAY_TO_DEVICE (A);
  SYNC_ARRAY_TO_DEVICE (B);

  char *src = _get_inner_product ("int", "+", "*");
  printf ("%s\n", src);
  cl_program program = CHECK_CL (
      clCreateProgramWithSource (context, 1, (const char **)&src, NULL, &err),
      err);
  TRY_BUILD_PROGRAM (program);
  cl_kernel kern = CHECK_CL (clCreateKernel (program, "entry", &err), err);
  SET_KERNEL_ARGS (kern, A, B, C);

  cl_event kernel_time;
  size_t local_size[] = { _tile_size, _tile_size };
  size_t global_size[]
      = { LOWEST_MULTIPLE_OF_TILE (C.dim1), LOWEST_MULTIPLE_OF_TILE (C.dim2) };
  CHECK_CL (clEnqueueNDRangeKernel (
      queue, kern, sizeof (global_size) / sizeof (*global_size), NULL,
      global_size, local_size, 0, NULL, &kernel_time));

  SYNC_ARRAY_FROM_DEVICE (C);

  LOG_CL_EVENT_TIME (kernel_time);

  free_array (A);
  free_array (B);
  free_array (C);
  CHECK_CL (clReleaseDevice (device));
  CHECK_CL (clReleaseContext (context));
  CHECK_CL (clReleaseCommandQueue (queue));
  free (src);

  return 0;
}
