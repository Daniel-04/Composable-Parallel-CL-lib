#include <cl_utils.h>
#include <inner_product.h>
#include <map.h>
#include <outer_product.h>
#include <reduce.h>
#include <scan.h>
#include <transpose.h>

#include <CL/cl.h>
#include <stdio.h>

int
main ()
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[]
      = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  setup_cl (&platform, &device, &context, &queue, props);

  int a1 = 2;
  int a2 = 3;
  int b1 = 3;
  int b2 = a1;
  int c1 = b1;
  int c2 = a2;
  array A = ALLOC_ARRAY (int, CL_MEM_READ_ONLY, a1, a2);
  array B = ALLOC_ARRAY (int, CL_MEM_READ_ONLY, b1, b2);
  array C = ALLOC_ARRAY (int, CL_MEM_READ_WRITE, c1, c2);

  int i = 0;
  for (; i < ARRAY_SIZE (A); i++)
    {
      A.ints[i] = i + 1;
    }
  for (int j = 0; j < ARRAY_SIZE (B); j++)
    {
      B.ints[j] = i + 1;
      i++;
    }

  SYNC_ARRAY_TO_DEVICE (A);
  SYNC_ARRAY_TO_DEVICE (B);

  cl_kernel kern = inner_product ("+", "*", A, B, C);

  cl_event kernel_time;
  size_t local_size[] = { _tile_size, _tile_size };
  size_t global_size[]
      = { LOWEST_MULTIPLE_OF_TILE (C.dim1), LOWEST_MULTIPLE_OF_TILE (C.dim2) };
  CHECK_CL (clEnqueueNDRangeKernel (
      queue, kern, sizeof (global_size) / sizeof (*global_size), NULL,
      global_size, local_size, 0, NULL, &kernel_time));
  LOG_CL_EVENT_TIME (kernel_time);

  SYNC_ARRAY_FROM_DEVICE (C);

  print_array (A);
  print_array (B);
  print_array (C);

  free_array (A);
  free_array (B);
  free_array (C);
  CHECK_CL (clReleaseDevice (device));
  CHECK_CL (clReleaseContext (context));
  CHECK_CL (clReleaseCommandQueue (queue));

  return 0;
}
