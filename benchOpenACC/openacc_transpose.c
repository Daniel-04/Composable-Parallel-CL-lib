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
      fprintf (stderr, "Usage:\n  %s MATRIX_SIZE\n", argv[0]);
      return 1;
    }

  int n = atoi (argv[1]);

  float *restrict A = (float *)malloc (n * n * sizeof (float));
  float *restrict B = (float *)malloc (n * n * sizeof (float));

  for (int i = 0; i < n * n; i++)
    {
      A[i] = (float)(i + 1);
    }

  struct timespec start, end;
  unsigned long long total = 0;
#pragma acc data copyin(A[0 : n * n]) copy(B[0 : n * n])
  {
    for (int iter = 0; iter < WARMUP_ITERS; iter++)
      {
#pragma acc parallel loop collapse(2)
        for (int i = 0; i < n; i++)
          {
            for (int j = 0; j < n; j++)
              {
                B[i * n + j] = A[j * n + i];
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
                B[i * n + j] = A[j * n + i];
              }
          }

        clock_gettime (CLOCK_MONOTONIC, &end);

        long seconds = end.tv_sec - start.tv_sec;
        long nanoseconds = end.tv_nsec - start.tv_nsec;
        total += seconds * 1000000000L + nanoseconds;
      }
  }

  double avg_time_ms = (double)total / ITERS / 1e6;
  double avg_time_s = (double)total / ITERS / 1e9;
  size_t bytes = 2 * n * n * sizeof (float);
  double bandwidth = bytes / avg_time_s / (1024.0 * 1024.0 * 1024.0);

  printf ("%d Iterations with %dx%d matrix of (%zu bytes)\n", ITERS, n, n,
          sizeof (float));
  printf ("  Average time: %lf ms\n", avg_time_ms);
  printf ("  Estimated bandwidth: %lf GB/s\n", bandwidth);

  free (A);
  free (B);

  return 0;
}
