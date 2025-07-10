#include "cl_utils.h"
#include "inner_product.h"
#include <CL/cl.h>
#include <stdio.h>

int
main ()
{
  cl_event kernel_event, mem_a_write, mem_b_write, mem_c_read;
  cl_int err;
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[]
      = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };

  setup_cl (&platform, &device, &context, &queue, props);

  int a1, a2, b1, b2, c1, c2;
  a1 = a2 = b1 = b2 = c1 = c2 = 4096;

  array A
      = ALLOC_ARRAY (float, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, a1, a2);
  array B
      = ALLOC_ARRAY (float, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, b1, b2);
  array C = ALLOC_ARRAY (float, CL_MEM_READ_WRITE, c1, c2);

  for (int i = 0; i < ARRAY_SIZE (A); i++)
    ((float *)A.host)[i] = (float)(i % 100) / 10.0f;
  for (int i = 0; i < ARRAY_SIZE (B); i++)
    ((float *)B.host)[i] = (float)(i % 100) / 10.0f;

  char *mmul_src = _get_inner_product ("float", "+", "*");
  printf ("%s\n", mmul_src);
  cl_program program
      = CHECK_CL (clCreateProgramWithSource (
                      context, 1, (const char **)&mmul_src, NULL, &err),
                  err);
  CHECK_CL (clBuildProgram (program, 1, &device, NULL, NULL, NULL));

  cl_kernel kern = CHECK_CL (clCreateKernel (program, "entry", &err), err);

  CHECK_CL (clSetKernelArg (kern, 0, sizeof (int), &a1));
  CHECK_CL (clSetKernelArg (kern, 1, sizeof (int), &a2));
  CHECK_CL (clSetKernelArg (kern, 2, sizeof (cl_mem), &A.device));
  CHECK_CL (clSetKernelArg (kern, 3, sizeof (int), &b1));
  CHECK_CL (clSetKernelArg (kern, 4, sizeof (int), &b2));
  CHECK_CL (clSetKernelArg (kern, 5, sizeof (cl_mem), &B.device));
  CHECK_CL (clSetKernelArg (kern, 6, sizeof (int), &c1));
  CHECK_CL (clSetKernelArg (kern, 7, sizeof (int), &c2));
  CHECK_CL (clSetKernelArg (kern, 8, sizeof (cl_mem), &C.device));

  size_t local_size[2] = { TILE_SIZE, TILE_SIZE };
  size_t global_size[2] = { ((c1 + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE,
                            ((c2 + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE };

  CHECK_CL (clEnqueueNDRangeKernel (queue, kern, 2, NULL, global_size,
                                    local_size, 0, NULL, &kernel_event));

  CHECK_CL (clEnqueueReadBuffer (queue, C.device, CL_TRUE, 0,
                                 sizeof (float) * ARRAY_SIZE (C), C.host, 0,
                                 NULL, &mem_c_read));

  /* LOG_CL_EVENT_TIME(mem_a_write); */
  /* LOG_CL_EVENT_TIME(mem_b_write); */
  LOG_CL_EVENT_TIME (kernel_event);
  LOG_CL_EVENT_TIME (mem_c_read);

  /* free(A); */
  /* free(B); */
  /* free(C); */

  /* CHECK_CL(clReleaseMemObject(A_d)); */
  /* CHECK_CL(clReleaseMemObject(B_d)); */
  /* CHECK_CL(clReleaseMemObject(C_d)); */
  /* CHECK_CL(clReleaseDevice(device)); */
  /* CHECK_CL(clReleaseContext(context)); */
  /* CHECK_CL(clReleaseCommandQueue(queue)); */
  /* free(mmul_src); */

  return 0;
}
