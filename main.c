#include <cl_utils.h>
#include <inner_product.h>
#include <map.h>
#include <outer_product.h>
#include <reduce.h>
#include <scan.h>
#include <transpose.h>

#include <CL/cl.h>

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

  int a1 = 93;
  array A = ALLOC_ARRAY (int, CL_MEM_READ_ONLY, a1, 2);

  int i = 0;
  for (; i < ARRAY_SIZE (A); i++)
    {
      A.ints[i] = i;
    }

  SYNC_ARRAY_TO_DEVICE (A);

  cl_event kernel_time;
  SCAN ("+", A, &kernel_time);
  LOG_CL_EVENT_TIME (kernel_time);

  print_array (A);

  SYNC_ARRAY_FROM_DEVICE (A);

  print_array (A);

  free_array (A);
  CHECK_CL (clReleaseDevice (device));
  CHECK_CL (clReleaseContext (context));
  CHECK_CL (clReleaseCommandQueue (queue));

  return 0;
}
