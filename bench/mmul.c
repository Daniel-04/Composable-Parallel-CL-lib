#include <cl_utils.h>
#include <inner_product.h>
#include <stdio.h>
#include <stdlib.h>

#define WARMUP_ITERS 100
#define ITERS 1000

int
main (int argc, const char **argv)
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[]
      = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  setup_cl (&platform, &device, &context, &queue, props);

  if (argc != 2)
    {
      fprintf (stderr, "Usage:\n  %s MATRIX_SIZE", argv[0]);
      return 1;
    }

  int n = atoi (argv[1]);
  array A = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, n, n);
  array B = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, n, n);
  array C = ALLOC_ARRAY (float, CL_MEM_READ_WRITE, n, n);
  int i = 0;
  for (; i < ARRAY_SIZE (A); i++)
    {
      A.floats[i] = (float)i + 1;
    }
  for (int j = 0; j < ARRAY_SIZE (A); j++)
    {
      B.floats[j] = (float)++i;
    }
  SYNC_ARRAY_TO_DEVICE (A);
  SYNC_ARRAY_TO_DEVICE (B);

  unsigned long long total = 0;
  for (int i = 0; i < WARMUP_ITERS; i++)
    {
      INNER_PRODUCT ("+", "*", A, B, C);
    }
  for (int i = 0; i < ITERS; i++)
    {
      total += INNER_PRODUCT ("+", "*", A, B, C);
    }
  printf ("%d Iterations with %dx%d matrix of %s(%zu bytes)\n", ITERS, n, n,
          TYPE_STR_FROM_ENUM (C.type), SIZE_FROM_ENUM (C.type));
  printf ("  Average time: %lf ms\n", (total / (double)ITERS) / 1e6);
  double avg_time_sec = ((double)total / ITERS) / 1e9;
  double gflops = (2.0 * n * n * n) / avg_time_sec / 1e9;
  printf ("  Estimated GFLOPS: %lf\n", gflops);

  FREE_ARRAY (A);
  FREE_ARRAY (B);
  FREE_ARRAY (C);
  release_cl (&device, &context, &queue);

  return 0;
}
