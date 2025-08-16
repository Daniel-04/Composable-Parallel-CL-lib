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

  int a1 = 4096;
  array A = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, a1, a1);

  int i = 0;
  for (; i < ARRAY_SIZE (A); i++)
    {
      A.floats[i] = (float)i;
    }

  SYNC_ARRAY_TO_DEVICE (A);

  int time = SCAN ("+", A);

  printf ("Time taken %f ms\n", 1e-6 * time);

  SYNC_ARRAY_FROM_DEVICE (A);

  free_array (A);
  CHECK_CL (clReleaseDevice (device));
  CHECK_CL (clReleaseContext (context));
  CHECK_CL (clReleaseCommandQueue (queue));

  return 0;
}
