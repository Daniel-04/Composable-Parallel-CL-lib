/*
** I suspect there is a bug with my NVIDIA OpenCL drivers.
** Timings returned by clGetEventProfilingInfo for CL_PROFILING_COMMAND_START
** and CL_PROFILING_COMMAND_END seem unrealistic and somewhat random.
** The timings also differ significantly from what is gotten from wall clock
** time with clFinish.
** This only happens when using clBlast, so it could be a library caused
** problem rather than a driver issue?
**
** The code below, compiled with:
** make bench/unreliable_timings_with_nvidia_drivers CFLAGS=-lclblast
** Output the following (which I could not replicate with my CPU OpenCL driver)
** for matrices of size 1024x1024, 1300x1300, (all values tested in between),
** and 4096x4096:
**
** 1024x1024 (wall clock was 5 seconds):
Found 1 platform(s)

Platform 0: NVIDIA CUDA
  Device 0:
    Name               : NVIDIA GeForce GTX 1050 Ti
    Type               : GPU
    Vendor             : NVIDIA Corporation
    Version            : OpenCL 3.0 CUDA
    Driver Version     : 575.64.05
    Device Available   : Yes
    Compiler Available : Yes
    Context Status     : Context created successfully

1000 Iterations with 1024x1024 matrix of float(4 bytes)
  Average time: 4.442992 ms
  Estimated GFLOPS: 483.341801

** 1300x1300 (wall clock was 5 seconds):
Found 1 platform(s)

Platform 0: NVIDIA CUDA
  Device 0:
    Name               : NVIDIA GeForce GTX 1050 Ti
    Type               : GPU
    Vendor             : NVIDIA Corporation
    Version            : OpenCL 3.0 CUDA
    Driver Version     : 575.64.05
    Device Available   : Yes
    Compiler Available : Yes
    Context Status     : Context created successfully

1000 Iterations with 1300x1300 matrix of float(4 bytes)
  Average time: 0.141426 ms
  Estimated GFLOPS: 31069.269339

** 4096x4096 (wall clock was 2 minutes 17 seconds):
Found 1 platform(s)

Platform 0: NVIDIA CUDA
  Device 0:
    Name               : NVIDIA GeForce GTX 1050 Ti
    Type               : GPU
    Vendor             : NVIDIA Corporation
    Version            : OpenCL 3.0 CUDA
    Driver Version     : 575.64.05
    Device Available   : Yes
    Compiler Available : Yes
    Context Status     : Context created successfully

1000 Iterations with 4096x4096 matrix of float(4 bytes)
  Average time: 1.432624 ms
  Estimated GFLOPS: 95935.095114
*/
#include <CL/cl.h>
#include <cl_utils.h>
#include <clblast_c.h>
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

  LOG_DEVICES ();

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

  cl_event mmul_event;
  unsigned long long total = 0;
  for (int i = 0; i < WARMUP_ITERS; i++)
    {
      CLBlastSgemm (CLBlastLayoutRowMajor, CLBlastTransposeNo,
                    CLBlastTransposeNo, n, n, n, 1, A.device, 0, n, B.device,
                    0, n, 0, C.device, 0, n, &queue, &mmul_event);
      clWaitForEvents (1, &mmul_event);
    }
  for (int i = 0; i < ITERS; i++)
    {
      CLBlastSgemm (CLBlastLayoutRowMajor, CLBlastTransposeNo,
                    CLBlastTransposeNo, n, n, n, 1, A.device, 0, n, B.device,
                    0, n, 0, C.device, 0, n, &queue, &mmul_event);
      total += GET_CL_EVENT_TIME (mmul_event);
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
