#ifndef CL_UTILS_H_
#define CL_UTILS_H_

#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 300
#endif
#ifndef DEVICE_TYPE
#define DEVICE_TYPE CL_DEVICE_TYPE_GPU
#endif
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define _STR(x) #x
#define _EXPAND(x) _STR(x)
/*
** Coverts contents into a string after expanding macros, does not break syntax
** highlight, eases writing OpenCL kernels in c source files.
*/
#define RAW(...) _EXPAND(__VA_ARGS__)

const char *_cl_err_to_str(cl_int err);
void _check_cl(cl_int err, const char *expr);
#define _GET_CHECK_CL(_1, _2, NAME, ...) NAME
#define _CHECK_CL_ONE(expr) _check_cl((expr), #expr)
#define _CHECK_CL_TWO(expr, err) expr; _check_cl(err, #expr)
#define CHECK_CL(...) _GET_CHECK_CL(__VA_ARGS__, _CHECK_CL_TWO, _CHECK_CL_ONE)(__VA_ARGS__)

void _log_cl_event_time(cl_event event, const char *expr);
#define LOG_CL_EVENT_TIME(event) _log_cl_event_time((event), #event)

/*
** Pointer to beggining of contiguous array of up to 3 dimensions, preceded by
** related data. Allows for direct indexing of host array, while carrying
** information needed by library helper functions (i.e ((int*) array)[93] = 42).
** [cl_mem, dim3, dim2, dim1, ...host data...]
**      -4    -3    -2    -1| 0 ^ pointer returned to user
**      ^ metadata accessed with helper macros
*/
typedef void*  array;

#define ARRAY_HEADER_SIZE (sizeof(cl_mem) + sizeof(size_t)*3)
#define ARRAY_BASE_PTR(arr) ((unsigned char *)(arr) - ARRAY_HEADER_SIZE)
#define ARRAY_MEM(arr)    (*((cl_mem *)ARRAY_BASE_PTR(arr)))
#define ARRAY_DIM3(arr)   (*((size_t *)((unsigned char *) arr) + sizeof(cl_mem)) - 3)
#define ARRAY_DIM2(arr)   (*((size_t *)((unsigned char *) arr) + sizeof(cl_mem)) - 2)
#define ARRAY_DIM1(arr)   (*((size_t *)((unsigned char *) arr) + sizeof(cl_mem)) - 1)

/*
** Allocate host memory for array of given dimensions. Can return NULL on
** failure to allocate memory. Does not allocate device memory.
*/
array _alloc_array(size_t membsize, size_t dim1, size_t dim2, size_t dim3);
void free_array(array arr);

#define _GET_ALLOC_ARRAY(_1, _2, _3, NAME, ...) NAME
#define _ALLOC_ARRAY_ONE(membsize, dim1)                                    \
    _alloc_array(membsize, dim1, 1, 1)
#define _ALLOC_ARRAY_TWO(membsize, dim1, dim2)                              \
    _alloc_array(membsize, dim1, dim2, 1)
#define _ALLOC_ARRAY_THREE(membsize, dim1, dim2, dim3)                      \
    _alloc_array(membsize, dim1, dim2, dim3)
#define ALLOC_ARRAY(...)                                                    \
    _GET_ALLOC_ARRAY(__VA_ARGS__, _ALLOC_ARRAY_THREE,                    \
                        _ALLOC_ARRAY_TWO,                                   \
                        _ALLOC_ARRAY_ONE)(sizeof(unsigned char), __VA_ARGS__)
#define ALLOC_ARRAY_T(type, ...)                                            \
    _GET_ALLOC_ARRAY(__VA_ARGS__, _ALLOC_ARRAY_THREE,                    \
                        _ALLOC_ARRAY_TWO,                                   \
                        _ALLOC_ARRAY_ONE)(sizeof(type), __VA_ARGS__)

#define FREE_ARRAY(arr) free_array(arr)


static cl_platform_id platform;
static cl_device_id device;
static cl_context context;
static cl_command_queue queue;
void init_cl_pipeline();

#endif // CL_UTILS_H_
