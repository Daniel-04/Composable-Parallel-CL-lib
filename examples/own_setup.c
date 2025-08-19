#include <CL/cl.h>
#include <cl_utils.h>
#include <map.h>

int
main ()
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[]
      = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };

  cl_int err;
  CHECK_CL (clGetPlatformIDs (1, &platform, NULL));
  CHECK_CL (clGetDeviceIDs (platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL));
  context
      = CHECK_CL (clCreateContext (NULL, 1, &device, NULL, NULL, &err), err);
  queue = CHECK_CL (
      clCreateCommandQueueWithProperties (context, device, props, &err), err);

  // Library will use this device
  copy_cl_pipeline (&platform, &device, &context, &queue);

  int n = 1 << 16;
  array A = ALLOC_ARRAY (char, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, n);

  MAP ("93", A, A);

  FREE_ARRAY (A);
  release_cl (&device, &context, &queue);
}
