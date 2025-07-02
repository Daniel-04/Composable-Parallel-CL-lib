#include "cl_utils.h"
#include "inner_product.h"
#include <CL/cl.h>
#include <stdio.h>

/*
**      N1
**    A --
** M1 | ..
**    | ..
**
**      N2
**    B --
** M2 | ..
**    | ..
**
**      N2
**    C --
** M1 | ..
**    | ..
**
** M2 = N1
*/
int main() {
  cl_event kernel_event, mem_a_write, mem_b_write, mem_c_read;
  cl_int err;
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  const cl_queue_properties props[] = {CL_QUEUE_PROPERTIES,
                                       CL_QUEUE_PROFILING_ENABLE, 0};

  CHECK_CL(clGetPlatformIDs(1, &platform, NULL));
  CHECK_CL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));
  context = CHECK_CL(clCreateContext(NULL, 1, &device, NULL, NULL, &err), err);
  queue = CHECK_CL(
      clCreateCommandQueueWithProperties(context, device, props, &err), err);

  const int M1 = 4096;
  const int N1 = 4096;
  const int M2 = 4096;
  const int N2 = 4096;
  const int A_s = M1 * N1;
  const int B_s = M2 * N2;
  const int C_s = M1 * N2;
  float *A = malloc(sizeof(float) * A_s);
  float *B = malloc(sizeof(float) * B_s);
  float *C = malloc(sizeof(float) * C_s);
  if (!A || !B || !C) {
    fprintf(stderr, "Host mallocation failed.\n");
    return 1;
  }

  for (int i = 0; i < A_s; i++)
    A[i] = (float)(i % 100) / 10.0f;
  for (int i = 0; i < B_s; i++)
    B[i] = (float)(i % 100) / 10.0f;
  cl_mem A_d = CHECK_CL(clCreateBuffer(context, CL_MEM_READ_ONLY,
                                       sizeof(float) * A_s, NULL, &err),
                        err);
  CHECK_CL(clEnqueueWriteBuffer(queue, A_d, CL_FALSE, 0, sizeof(float) * A_s, A,
                                0, NULL, &mem_a_write));

  cl_mem B_d = CHECK_CL(clCreateBuffer(context, CL_MEM_READ_ONLY,
                                       sizeof(float) * B_s, NULL, &err),
                        err);
  CHECK_CL(clEnqueueWriteBuffer(queue, B_d, CL_FALSE, 0, sizeof(float) * B_s, A,
                                0, NULL, &mem_b_write));

  cl_mem C_d = CHECK_CL(clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                       sizeof(float) * C_s, NULL, &err),
                        err);

  char *mmul_src = _get_inner_product("float", "+", "*");
  printf("%s\n", mmul_src);
  cl_program program = CHECK_CL(clCreateProgramWithSource(
      context, 1, (const char **)&mmul_src, NULL, &err), err);
  CHECK_CL(clBuildProgram(program, 1, &device, NULL, NULL, NULL));

  cl_kernel kern = CHECK_CL(clCreateKernel(program, "inner", &err), err);

  CHECK_CL(clSetKernelArg(kern, 0, sizeof(int), &M1));
  CHECK_CL(clSetKernelArg(kern, 1, sizeof(int), &N2));
  CHECK_CL(clSetKernelArg(kern, 2, sizeof(int), &N1));
  CHECK_CL(clSetKernelArg(kern, 3, sizeof(cl_mem), &A_d));
  CHECK_CL(clSetKernelArg(kern, 4, sizeof(cl_mem), &B_d));
  CHECK_CL(clSetKernelArg(kern, 5, sizeof(cl_mem), &C_d));

  size_t local_size[2] = {TILE_SIZE, TILE_SIZE};
  size_t global_size[2] = {((N2 + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE,
                           ((M1 + TILE_SIZE - 1) / TILE_SIZE) * TILE_SIZE};

  CHECK_CL(clEnqueueNDRangeKernel(queue, kern, 2, NULL, global_size, local_size,
                                  0, NULL, &kernel_event));

  CHECK_CL(clEnqueueReadBuffer(queue, C_d, CL_TRUE, 0, sizeof(float) * C_s, C,
                               0, NULL, &mem_c_read));

  LOG_CL_EVENT_TIME(mem_a_write);
  LOG_CL_EVENT_TIME(mem_b_write);
  LOG_CL_EVENT_TIME(kernel_event);
  LOG_CL_EVENT_TIME(mem_c_read);

  free(A);
  free(B);
  free(C);

  CHECK_CL(clReleaseMemObject(A_d));
  CHECK_CL(clReleaseMemObject(B_d));
  CHECK_CL(clReleaseMemObject(C_d));
  CHECK_CL(clReleaseDevice(device));
  CHECK_CL(clReleaseContext(context));
  CHECK_CL(clReleaseCommandQueue(queue));
  free(mmul_src);

  return 0;
}
