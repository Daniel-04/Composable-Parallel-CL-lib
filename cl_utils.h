#ifndef CL_UTILS_H_
#define CL_UTILS_H_

#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 300
#endif
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

/*
** Used by kernels.
*/
#ifndef TILE_SIZE
#define TILE_SIZE 16
#endif
extern int _tile_size;
void set_tile_size (int size);

#define LOWEST_MULTIPLE_OF_TILE(n)                                            \
  ((((n) + _tile_size - 1) / _tile_size) * _tile_size)

/*
** Copies of user CL pipeline for library use.
*/
extern cl_platform_id _platform;
extern cl_device_id _device;
extern cl_context _context;
extern cl_command_queue _queue;

#ifndef DEFAULT_CL_DEVICE
#define DEFAULT_CL_DEVICE CL_DEVICE_TYPE_GPU
#endif
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

/*
** For multi argument macros to choose N'th argument version
*/
#define _GETM_ONE(_1, NAME, ...) NAME
#define _GETM_TWO(_1, _2, NAME, ...) NAME
#define _GETM_THREE(_1, _2, _3, NAME, ...) NAME

const char *_cl_err_to_str (cl_int err);
void _check_cl (cl_int err, const char *expr, int line, const char *file);
#define _CHECK_CL_ONE(expr) _check_cl ((expr), #expr, __LINE__, __FILE__)
#define _CHECK_CL_TWO(expr, err)                                              \
  expr;                                                                       \
  _check_cl (err, #expr, __LINE__, __FILE__)
#define CHECK_CL(...)                                                         \
  _GETM_TWO (__VA_ARGS__, _CHECK_CL_TWO, _CHECK_CL_ONE) (__VA_ARGS__)

void try_build_program (cl_program program, cl_device_id device);
#define _TRY_BUILD_PROGRAM_ONE(program) try_build_program (program, _device)
#define _TRY_BUILD_PROGRAM_two(program, device)                               \
  try_build_program (program, device)
#define TRY_BUILD_PROGRAM(...)                                                \
  _GETM_TWO (__VA_ARGS__, _TRY_BUILD_PROGRAM_TWO,                             \
             _TRY_BUILD_PROGRAM_ONE) (__VA_ARGS__)

void set_kernel_args (cl_kernel kernel, int num_args, ...);
#define _SET_KERNEL_ARGS_ONE(kernel, A) set_kernel_args (kernel, 1, A)
#define _SET_KERNEL_ARGS_TWO(kernel, A, B) set_kernel_args (kernel, 2, A, B)
#define _SET_KERNEL_ARGS_THREE(kernel, A, B, C)                               \
  set_kernel_args (kernel, 3, A, B, C)
#define SET_KERNEL_ARGS(kernel, ...)                                          \
  _GETM_THREE (__VA_ARGS__, _SET_KERNEL_ARGS_THREE, _SET_KERNEL_ARGS_TWO,     \
               _SET_KERNEL_ARGS_ONE) (kernel, __VA_ARGS__)

void _log_cl_event_time (cl_event event, const char *expr);
#define LOG_CL_EVENT_TIME(event) _log_cl_event_time ((event), #event)

#define _TYPE_LIST                                                            \
  X (int, TYPE_INT)                                                           \
  X (float, TYPE_FLOAT)                                                       \
  X (double, TYPE_DOUBLE)

typedef enum
{
  TYPE_UNKNOWN,
#define X(type, enum_name) enum_name,
  _TYPE_LIST
#undef X
} array_type;

enum
{
#define X(type, enum_name) _TYPE_MAP_##type = enum_name,
  _TYPE_LIST
#undef X
};
#define TYPE_TO_ENUM(type) _TYPE_MAP_##type

#define X(type, enum_name) sizeof (type),
static const size_t _size_of_type_map[] = { 0, _TYPE_LIST };
#undef X
#define SIZE_FROM_ENUM(type) _size_of_type_map[type]

typedef struct
{
  int dim1, dim2, dim3;
  size_t membsize;
  cl_mem device;
  array_type type;
  union
  {
    void *host;
    int *ints;
    float *floats;
    double *doubles;
  };
} array;

/*
** Allocate memory for array of given dimensions, on host and device.
*/
array _alloc_array (array_type type, cl_mem_flags flags, size_t dim1,
                    size_t dim2, size_t dim3);
#define _ALLOC_ARRAY_ONE(type, flags, dim1)                                   \
  _alloc_array (type, flags, dim1, 1, 1)
#define _ALLOC_ARRAY_TWO(type, flags, dim1, dim2)                             \
  _alloc_array (type, flags, dim1, dim2, 1)
#define _ALLOC_ARRAY_THREE(type, flags, dim1, dim2, dim3)                     \
  _alloc_array (type, flags, dim1, dim2, dim3)
#define ALLOC_ARRAY(type, flags, ...)                                         \
  _GETM_THREE (__VA_ARGS__, _ALLOC_ARRAY_THREE, _ALLOC_ARRAY_TWO,             \
               _ALLOC_ARRAY_ONE) ((array_type)TYPE_TO_ENUM (type), flags,     \
                                  __VA_ARGS__)

#define ARRAY_SIZE(arr) (arr.dim1 * arr.dim2 * arr.dim3)

/*
** Write data in host side buffer to device buffer, blocks until finished.
*/
void sync_array_to_device (array arr, cl_event *event);
#define _SYNC_ARRAY_TO_DEVICE_ONE(arr) sync_array_to_device (arr, NULL)
#define _SYNC_ARRAY_TO_DEVICE_TWO(arr, event) sync_array_to_device (arr, event)
#define SYNC_ARRAY_TO_DEVICE(...)                                             \
  _GETM_TWO (__VA_ARGS__, _SYNC_ARRAY_TO_DEVICE_TWO,                          \
             _SYNC_ARRAY_TO_DEVICE_ONE) (__VA_ARGS__)

/*
** Read data into host side buffer from device buffer, blocks until finished.
*/
void sync_array_from_device (array arr, cl_event *event);
#define _SYNC_ARRAY_FROM_DEVICE_ONE(arr) sync_array_from_device (arr, NULL)
#define _SYNC_ARRAY_FROM_DEVICE_TWO(arr, event)                               \
  sync_array_from_device (arr, event)
#define SYNC_ARRAY_FROM_DEVICE(...)                                           \
  _GETM_TWO (__VA_ARGS__, _SYNC_ARRAY_FROM_DEVICE_TWO,                        \
             _SYNC_ARRAY_FROM_DEVICE_ONE) (__VA_ARGS__)

void free_array (array arr);
#define FREE_ARRAY(arr) free_array (arr)

#endif // CL_UTILS_H_
