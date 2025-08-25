#include <cl_utils.h>
#include <map.h>
#include <math.h>
#include <reduce.h>
#include <stdio.h>

int
main ()
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  setup_cl (&platform, &device, &context, &queue, NULL);

  int n = 12;
  array A = ALLOC_ARRAY (float, CL_MEM_READ_WRITE, n);
  int i = 0;
  for (; i < ARRAY_SIZE (A); i++)
    {
      A.floats[i] = (float)i + 1;
    }

  SYNC_ARRAY_TO_DEVICE (A);
  PRINT_ARRAY (A);

  MAP ("a * a", A, A);

  SYNC_ARRAY_FROM_DEVICE (A);
  PRINT_ARRAY (A);

  REDUCE ("a + b", A);

  SYNC_ARRAY_FROM_DEVICE (A);
  PRINT_ARRAY (A);

  printf ("%f\n", sqrtf (A.floats[0]));

  FREE_ARRAY (A);
  release_cl (&device, &context, &queue);
}
