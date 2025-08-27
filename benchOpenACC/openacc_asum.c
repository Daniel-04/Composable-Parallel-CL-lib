#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WARMUP_ITERS 100
#define ITERS 1000

int
main (int argc, char **argv)
{
  if (argc != 2)
    {
      fprintf (stderr, "Usage:\n  %s ARRAY_SIZE\n", argv[0]);
      return 1;
    }

  int n = atoi (argv[1]);
  float *A = (float *)malloc (n * sizeof (float));

  for (int i = 0; i < n; i++)
    {
      A[i] = (float)(i + 1);
    }

  float result = 0.0f;

  struct timespec start, end;
  unsigned long long total = 0;
#pragma acc data copyin(A[0 : n])
  {
    for (int i = 0; i < WARMUP_ITERS; i++)
      {
        float sum = 0.0f;

#pragma acc parallel loop reduction(+ : sum)
        for (int j = 0; j < n; j++)
          {
            sum += fabsf (A[j]);
          }

        result = sum;
      }

    for (int i = 0; i < ITERS; i++)
      {
        float sum = 0.0f;

        clock_gettime (CLOCK_MONOTONIC, &start);

#pragma acc parallel loop reduction(+ : sum)
        for (int j = 0; j < n; j++)
          {
            sum += fabsf (A[j]);
          }

        clock_gettime (CLOCK_MONOTONIC, &end);

        result = sum;

        long seconds = end.tv_sec - start.tv_sec;
        long nanoseconds = end.tv_nsec - start.tv_nsec;
        long elapsed = seconds * 1000000000L + nanoseconds;
        total += elapsed;
      }
  }

  double avg_time_ms = (double)total / ITERS / 1e6;
  double avg_time_sec = (double)total / ITERS / 1e9;
  double gflops = (n - 1) / avg_time_sec / 1e9;

  printf ("%d Iterations with %d array of (%zu bytes)\n", ITERS, n,
          n * sizeof (float));
  printf ("  Average time: %lf ms\n", avg_time_ms);
  printf ("  Estimated GFLOPS: %lf\n", gflops);
  printf ("  Final result (sum): %f\n", result);

  free (A);

  return 0;
}
