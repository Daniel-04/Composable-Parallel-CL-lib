#include <cl_utils.h>

int
main ()
{
  LOG_DEVICES ();
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[]
      = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  setup_cl (&platform, &device, &context, &queue, props);

  LOG_MEMORY_LIMITS (device);
  LOG_WORK_LIMITS (device);
}
