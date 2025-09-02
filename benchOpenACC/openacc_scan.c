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

  struct timespec start, end;
  unsigned long long total = 0;
#pragma acc data copy(A[0 : n])
  {
    for (int i = 0; i < WARMUP_ITERS; i++)
      {
#pragma acc parallel loop
        for (int j = 1; j < n; j++)
          {
            A[j] = A[j - 1] + A[j];
#pragma acc update self(A[j - 1]) // Ensure A[j-1] is visible
          }
      }

    for (int i = 0; i < ITERS; i++)
      {
#pragma acc parallel loop
        for (int j = 0; j < n; j++)
          A[j] = (float)(j + 1);

        clock_gettime (CLOCK_MONOTONIC, &start);

        for (int offset = 1; offset < n; offset *= 2)
          {
#pragma acc parallel loop
            for (int j = n - 1; j >= offset; j--)
              {
                A[j] += A[j - offset];
              }
          }

        clock_gettime (CLOCK_MONOTONIC, &end);

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

  free (A);

  return 0;
}
