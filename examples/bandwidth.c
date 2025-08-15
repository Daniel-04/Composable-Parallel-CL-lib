#include <CL/cl.h>
#include <cl_utils.h>

#define WARMUP_ITERS 100
#define ITERS 1000

const char *src = RAW (
    __kernel void bandwidth_test (int a1, int a2, int a3, __global float *A,
                                  int b1, int b2, int b3, __global float *B) {
      int gid = get_global_id (0);
      B[gid] = A[gid];
    });

void
do_reads_and_writes (int megabytes)
{
  size_t size = 1024 * 1024 * megabytes;
  array A = ALLOC_ARRAY (double, CL_MEM_READ_ONLY, size / sizeof (double));

  for (int i = 0; i < ARRAY_SIZE (A); i++)
    {
      A.doubles[i] = i;
    }

  cl_event write_event, read_event;
  unsigned long long write_nanos, read_nanos;
  unsigned long long total_write = 0, total_read = 0;
  for (int i = 0; i < WARMUP_ITERS; i++)
    {
      SYNC_ARRAY_TO_DEVICE (A);
    }
  for (int i = 0; i < ITERS; i++)
    {
      SYNC_ARRAY_TO_DEVICE (A, &write_event);
      write_nanos = GET_CL_EVENT_TIME (write_event);
      total_write += write_nanos;
    }

  for (int i = 0; i < WARMUP_ITERS; i++)
    {
      SYNC_ARRAY_FROM_DEVICE (A);
    }
  for (int i = 0; i < ITERS; i++)
    {
      SYNC_ARRAY_FROM_DEVICE (A, &read_event);
      read_nanos = GET_CL_EVENT_TIME (read_event);
      total_read += read_nanos;
    }

  printf ("%d Iterations host-to-device with buffer of %d MB\n", ITERS,
          megabytes);
  printf ("  Average write time: %f ms\n",
          (total_write / (double)ITERS) / 1e6);
  printf ("  Average read time: %f ms\n", (total_read / (double)ITERS) / 1e6);

  double write_bandwidth
      = ((double)(size * ITERS) / (total_write / 1e9)) / 1e9;
  double read_bandwidth = ((double)(size * ITERS) / (total_read / 1e9)) / 1e9;
  printf ("  Estimated write bandwidth: %.2f GB/s\n", write_bandwidth);
  printf ("  Estimated read bandwidth: %.2f GB/s\n", read_bandwidth);

  array B = ALLOC_ARRAY (float, CL_MEM_READ_WRITE, size / sizeof (float));
  cl_kernel kernel = TRY_COMPILE_KERNEL (src, "bandwidth_test");
  SET_KERNEL_ARGS (kernel, A, B);
  size_t global_size = A.dim1;

  cl_event kernel_event;
  unsigned long long copy_nanos;
  unsigned long long total_copy = 0;
  for (int i = 0; i < WARMUP_ITERS; i++)
    {
      CHECK_CL (clEnqueueNDRangeKernel (_queue, kernel, 1, NULL, &global_size,
                                        NULL, 0, NULL, &kernel_event));
      clFinish (_queue);
    }
  for (int i = 0; i < ITERS; i++)
    {
      CHECK_CL (clEnqueueNDRangeKernel (_queue, kernel, 1, NULL, &global_size,
                                        NULL, 0, NULL, &kernel_event));
      copy_nanos = GET_CL_EVENT_TIME (kernel_event);
      total_copy += copy_nanos;
    }

  printf ("%d Iterations on-device with buffer of %d MB\n", ITERS, megabytes);
  printf ("  Average copy time: %f ms\n", (total_copy / (double)ITERS) / 1e6);

  double copy_bandwidth = ((double)(size * ITERS) / (total_copy / 1e9)) / 1e9;
  printf ("  Estimated memory bandwidth: %.2f GB/s\n", copy_bandwidth);

  free_array (A);
  free_array (B);
}

int
main ()
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[]
      = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  setup_cl (&platform, &device, &context, &queue, props);

  for (int i = 1; i <= 1024; i *= 2)
    {
      do_reads_and_writes (i);
    }
  return 0;
}
