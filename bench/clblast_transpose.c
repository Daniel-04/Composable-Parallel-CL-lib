/*
** Reffer to bench/unreliable_timings_with_nvidia_drivers.c as to why I am not
** using queue profiling in these benchmarks.
**
** Reffer to bench/mmul.c and bench/mmul_wall.c for convincing that clFinish +
** wall clock timings and queue profiling are accurate enough to be compared
** somewhat fairly.
*/
#include <CL/cl.h>
#include <cl_utils.h>
#include <clblast_c.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WARMUP_ITERS 100
#define ITERS 1000

int
main (int argc, const char **argv)
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  setup_cl (&platform, &device, &context, &queue, NULL);

  if (argc != 2)
    {
      fprintf (stderr, "Usage:\n  %s MATRIX_SIZE", argv[0]);
      return 1;
    }

  int n = atoi (argv[1]);
  array A = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, n, n);
  array B = ALLOC_ARRAY (float, CL_MEM_READ_WRITE, n, n);
  int i = 0;
  for (; i < ARRAY_SIZE (A); i++)
    {
      A.floats[i] = (float)i + 1;
    }
  SYNC_ARRAY_TO_DEVICE (A);
  SYNC_ARRAY_TO_DEVICE (B);

  clFinish (queue);
  unsigned long long total = 0;
  struct timespec start, end;
  for (int i = 0; i < WARMUP_ITERS; i++)
    {
      CLBlastSomatcopy (CLBlastLayoutRowMajor, CLBlastTransposeYes, n, n, 1,
                        A.device, 0, n, B.device, 0, n, &queue, NULL);
      clFinish (queue);
    }
  for (int i = 0; i < ITERS; i++)
    {
      clock_gettime (CLOCK_MONOTONIC, &start);
      CLBlastSomatcopy (CLBlastLayoutRowMajor, CLBlastTransposeYes, n, n, 1,
                        A.device, 0, n, B.device, 0, n, &queue, NULL);
      clFinish (queue);
      clock_gettime (CLOCK_MONOTONIC, &end);
      long seconds = end.tv_sec - start.tv_sec;
      long nanoseconds = end.tv_nsec - start.tv_nsec;
      long elapsed = seconds * 1000000000L + nanoseconds;
      total += elapsed;
    }
  printf ("%d Iterations with %dx%d matrix of %s(%zu bytes)\n", ITERS, n, n,
          TYPE_STR_FROM_ENUM (B.type), SIZE_FROM_ENUM (B.type));
  printf ("  Average time: %lf ms\n", (total / (double)ITERS) / 1e6);
  size_t bytes_transferred = 2 * n * n * SIZE_FROM_ENUM (B.type);
  double avg_time_s = (total / (double)ITERS) / 1e9;
  double bandwidth
      = bytes_transferred / avg_time_s / (1024.0 * 1024.0 * 1024.0);
  printf ("  Estimated bandwidth: %lf GB/s\n", bandwidth);

  FREE_ARRAY (A);
  FREE_ARRAY (B);
  release_cl (&device, &context, &queue);

  return 0;
}
