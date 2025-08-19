#include <cl_utils.h>
#include <inner_product.h>

int
main ()
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  setup_cl (&platform, &device, &context, &queue, NULL);

  int m = 3;
  int n = 5;
  int p = 4;
  array A = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, n, m);
  array B = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, p, n);
  array C = ALLOC_ARRAY (float, CL_MEM_READ_WRITE, p, m);

  for (int i = 0; i < ARRAY_SIZE (A); i++)
    A.floats[i] = (float)rand ();

  for (int i = 0; i < ARRAY_SIZE (B); i++)
    B.floats[i] = (float)rand ();

  SYNC_ARRAY_TO_DEVICE (A);
  SYNC_ARRAY_TO_DEVICE (B);
  PRINT_ARRAY (A);
  PRINT_ARRAY (B);

  INNER_PRODUCT ("+", "*", A, B, C);

  SYNC_ARRAY_FROM_DEVICE (C);
  PRINT_ARRAY (C);

  FREE_ARRAY (A);
  FREE_ARRAY (B);
  FREE_ARRAY (C);
  release_cl (&device, &context, &queue);
}
