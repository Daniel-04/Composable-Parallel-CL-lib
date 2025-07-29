#ifndef CL_UTILS_H_
#define CL_UTILS_H_

#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 300
#endif
#ifndef DEFAULT_CL_DEVICE
#define DEFAULT_CL_DEVICE CL_DEVICE_TYPE_GPU
#endif
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

/*
** Copies of user CL pipeline for library use.
*/
extern cl_platform_id _platform;
extern cl_device_id _device;
extern cl_context _context;
extern cl_command_queue _queue;

void copy_cl_pipeline (cl_platform_id *platform, cl_device_id *device,
                       cl_context *context, cl_command_queue *queue);
void setup_cl (cl_platform_id *platform, cl_device_id *device,
               cl_context *context, cl_command_queue *queue,
               const cl_queue_properties *properties);

#define _STR(x) #x
#define _EXPAND(x) _STR (x)
/*
** Converts contents into a string after expanding macros, does not break
** syntax highlight, eases writing OpenCL kernels in c source files.
*/
#define RAW(...) _EXPAND (__VA_ARGS__)

const char *_cl_err_to_str (cl_int err);
void _check_cl (cl_int err, const char *expr, int line, const char *file);
#define _GET_CHECK_CL(_1, _2, NAME, ...) NAME
#define _CHECK_CL_ONE(expr) _check_cl ((expr), #expr, __LINE__, __FILE__)
#define _CHECK_CL_TWO(expr, err)                                              \
  expr;                                                                       \
  _check_cl (err, #expr, __LINE__, __FILE__)
#define CHECK_CL(...)                                                         \
  _GET_CHECK_CL (__VA_ARGS__, _CHECK_CL_TWO, _CHECK_CL_ONE) (__VA_ARGS__)

void _log_cl_event_time (cl_event event, const char *expr);
#define LOG_CL_EVENT_TIME(event) _log_cl_event_time ((event), #event)

typedef struct
{
  size_t dim1, dim2, dim3;
  size_t membsize;
  cl_mem device;
  void *host;
} array;

/*
** Allocate memory for array of given dimensions, on host and device.
*/
array _alloc_array (size_t membsize, cl_mem_flags flags, size_t dim1,
                    size_t dim2, size_t dim3);
#define _GET_ALLOC_ARRAY(_1, _2, _3, NAME, ...) NAME
#define _ALLOC_ARRAY_ONE(membsize, flags, dim1)                               \
  _alloc_array (membsize, flags, dim1, 1, 1)
#define _ALLOC_ARRAY_TWO(membsize, flags, dim1, dim2)                         \
  _alloc_array (membsize, flags, dim1, dim2, 1)
#define _ALLOC_ARRAY_THREE(membsize, flags, dim1, dim2, dim3)                 \
  _alloc_array (membsize, flags, dim1, dim2, dim3)
#define ALLOC_ARRAY(type, flags, ...)                                         \
  _GET_ALLOC_ARRAY (__VA_ARGS__, _ALLOC_ARRAY_THREE, _ALLOC_ARRAY_TWO,        \
                    _ALLOC_ARRAY_ONE) (sizeof (type), flags, __VA_ARGS__)

#define ARRAY_SIZE(arr) (arr.dim1 * arr.dim2 * arr.dim3)

/*
** Write data in host side buffer to device buffer, blocks until finished.
*/
void sync_array_to_device (array arr, cl_event *event);
#define _GET_SYNC_ARRAY_TO_DEVICE(_1, _2, NAME, ...) NAME
#define _SYNC_ARRAY_TO_DEVICE_ONE(arr) sync_array_to_device (arr, NULL)
#define _SYNC_ARRAY_TO_DEVICE_TWO(arr, event) sync_array_to_device (arr, event)
#define SYNC_ARRAY_TO_DEVICE(...)                                             \
  _GET_SYNC_ARRAY_TO_DEVICE (__VA_ARGS__, _SYNC_ARRAY_TO_DEVICE_TWO,          \
                             _SYNC_ARRAY_TO_DEVICE_ONE) (__VA_ARGS__)

/*
** Read data into host side buffer from device buffer, blocks until finished.
*/
void sync_array_from_device (array arr, cl_event *event);
#define _GET_SYNC_ARRAY_FROM_DEVICE(_1, _2, NAME, ...) NAME
#define _SYNC_ARRAY_FROM_DEVICE_ONE(arr) sync_array_from_device (arr, NULL)
#define _SYNC_ARRAY_FROM_DEVICE_TWO(arr, event)                               \
  sync_array_from_device (arr, event)
#define SYNC_ARRAY_FROM_DEVICE(...)                                           \
  _GET_SYNC_ARRAY_FROM_DEVICE (__VA_ARGS__, _SYNC_ARRAY_FROM_DEVICE_TWO,      \
                               _SYNC_ARRAY_FROM_DEVICE_ONE) (__VA_ARGS__)

void free_array (array arr);
#define FREE_ARRAY(arr) free_array (arr)

#endif // CL_UTILS_H_
