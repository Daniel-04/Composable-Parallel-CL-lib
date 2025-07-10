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
void _check_cl(cl_int err, const char *expr, int line, const char *file);
#define _GET_CHECK_CL(_1, _2, NAME, ...) NAME
#define _CHECK_CL_ONE(expr) _check_cl((expr), #expr, __LINE__, __FILE__)
#define _CHECK_CL_TWO(expr, err) expr; _check_cl(err, #expr, __LINE__, __FILE__)
#define CHECK_CL(...) _GET_CHECK_CL(__VA_ARGS__, _CHECK_CL_TWO, _CHECK_CL_ONE)(__VA_ARGS__)

void _log_cl_event_time(cl_event event, const char *expr);
#define LOG_CL_EVENT_TIME(event) _log_cl_event_time((event), #event)

typedef struct {
    size_t dim1, dim2, dim3;
    size_t membsize;
    cl_mem device;
    void *host;
} array;

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

#endif // CL_UTILS_H_
