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
  float *B = (float *)malloc (n * sizeof (float));
  float *C = (float *)malloc (n * n * sizeof (float));

  for (int i = 0; i < n; i++)
    {
      A[i] = (float)(i + 1);
      B[i] = (float)(n + i + 1);
    }

  for (int i = 0; i < n * n; i++)
    {
      C[i] = 0.0f;
    }

  struct timespec start, end;
  unsigned long long total = 0;

#pragma acc data copyin(A[0 : n], B[0 : n]) copy(C[0 : n * n])
  {
    for (int iter = 0; iter < WARMUP_ITERS; iter++)
      {
#pragma acc parallel loop collapse(2)
        for (int i = 0; i < n; i++)
          {
            for (int j = 0; j < n; j++)
              {
                C[i * n + j] += A[i] * B[j];
              }
          }
      }

    for (int iter = 0; iter < ITERS; iter++)
      {
        clock_gettime (CLOCK_MONOTONIC, &start);

#pragma acc parallel loop collapse(2)
        for (int i = 0; i < n; i++)
          {
            for (int j = 0; j < n; j++)
              {
                C[i * n + j] = A[i] * B[j];
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
  double gflops = ((double)n * n) / avg_time_sec / 1e9;

  printf ("%d Iterations with %d array of (%zu bytes)\n", ITERS, n,
          sizeof (float));
  printf ("  Average time: %lf ms\n", avg_time_ms);
  printf ("  Estimated GFLOPS: %lf\n", gflops);

  free (A);
  free (B);
  free (C);

  return 0;
}
