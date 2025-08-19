#include <cl_utils.h>
#include <map.h>
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
      fprintf (stderr, "Usage:\n  %s ARRAY_SIZE", argv[0]);
      return 1;
    }

  int n = atoi (argv[1]);
  array A = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, n);
  array B = ALLOC_ARRAY (float, CL_MEM_READ_WRITE, n);
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
      MAP ("a + b", A, B);
    }
  for (int i = 0; i < ITERS; i++)
    {
      total += MAP ("a + b", A, B);
    }
  printf ("%d Iterations with %d array of %s(%zu bytes)\n", ITERS, n,
          TYPE_STR_FROM_ENUM (B.type), SIZE_FROM_ENUM (B.type));
  printf ("  Average time: %lf ms\n", (total / (double)ITERS) / 1e6);
  double avg_time_sec = ((double)total / ITERS) / 1e9;
  double gflops = n / avg_time_sec / 1e9;
  printf ("  Estimated GFLOPS: %lf\n", gflops);

  FREE_ARRAY (A);
  FREE_ARRAY (B);
  release_cl (&device, &context, &queue);

  return 0;
}
