/**
 * @file cl_utils.h
 * @brief Utilities to ease OpenCL usage.
 */

#ifndef CL_UTILS_H_
#define CL_UTILS_H_

#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 300
#endif
#include <CL/cl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef BUFSIZE
#define BUFSIZE 256
#endif

/**
 * @brief Library error handler signature.
 */
typedef void (*error_handler_fn) (const char *err, va_list args);

/**
 * @brief Sets the error handler used by the library.
 *
 * This function sets the error handler function by the library.
 *
 * @param handler The @ref error_handler_fn "error handler function" to set.
 */
void set_error_handler (error_handler_fn handler);

/**
 * @brief Error handler used by the library.
 *
 * By default this error handler prints an error message and aborts the
 * program, generating a coredump. @ref set_error_handler "Error handler" can
 * be set by the user.
 *
 * @param err Error message to output.
 */
void handle_error (const char *err, ...);

/**
 * @brief Default tile size used by the library, can be overriden.
 */
#ifndef TILE_SIZE
#define TILE_SIZE 16
#endif
extern int _tile_size;
/**
 * @brief Sets the tile size used by the library.
 *
 * This function sets the global value of tile_size, used in tile-based
 * operations.
 *
 * @param size The tile size to set.
 */
void set_tile_size (int size);

/**
 * @brief Rounds up a value to the nearest multiple of tile size.
 *
 * @param n Input value.
 */
#define LOWEST_MULTIPLE_OF_TILE(n)                                            \
  ((((n) + _tile_size - 1) / _tile_size) * _tile_size)

/*
** Copies of user CL pipeline for library use.
*/
extern cl_platform_id _platform;
extern cl_device_id _device;
extern cl_context _context;
extern cl_command_queue _queue;

/**
 * @brief Default OpenCL device to search for, can be overriden.
 */
#ifndef DEFAULT_CL_DEVICE
#define DEFAULT_CL_DEVICE CL_DEVICE_TYPE_GPU
#endif
void copy_cl_pipeline (cl_platform_id *platform, cl_device_id *device,
                       cl_context *context, cl_command_queue *queue);
/**
 * @brief Initializes OpenCL platform, device, context, and command queue.
 *
 * Sets up the OpenCL environment by selecting a default platform and
 * device, creating a context, and creating a command queue with the specified
 * properties. It also copies the cl pipeline for library use.
 *
 * @param[out] platform Pointer to a cl_platform_id that will receive the
 * selected platform.
 * @param[out] device Pointer to a cl_device_id that will receive the selected
 * @ref DEFAULT_CL_DEVICE "default" device.
 * @param[out] context Pointer to a cl_context that will receive the created
 * context.
 * @param[out] queue Pointer to a cl_command_queue that will receive the
 * created command queue.
 * @param properties Optional queue properties passed to
 * clCreateCommandQueueWithProperties.
 */
void setup_cl (cl_platform_id *platform, cl_device_id *device,
               cl_context *context, cl_command_queue *queue,
               const cl_queue_properties *properties);

#define _STR(x) #x
#define _EXPAND(x) _STR (x)
/**
 * @brief Stringify contents.
 *
 * Converts contents into a string after expanding macros.
 * Helpful to preserve syntax highlight when writing OpenCL kernels.
 *
 * @param ... Contents to stringify.
 */
#define RAW(...) _EXPAND (__VA_ARGS__)

/*
** For multi argument macros to choose N'th argument version
*/
#define _GETM_ONE(_1, NAME, ...) NAME
#define _GETM_TWO(_1, _2, NAME, ...) NAME
#define _GETM_THREE(_1, _2, _3, NAME, ...) NAME
#define _GETM_FOUR(_1, _2, _3, _4, NAME, ...) NAME
#define _GETM_FIVE(_1, _2, _3, _4, _5, NAME, ...) NAME
#define _GETM_SIX(_1, _2, _3, _4, _5, _6, NAME, ...) NAME

/**
 * @brief Log information of devices.
 */
void log_devices ();
#define LOG_DEVICES() log_devices ()

/**
 * @brief Log device memory limit information.
 *
 * @param device The cl_device to log for.
 */
void log_memory_limits (cl_device_id device);
#define _LOG_MEMORY_LIMITS_ZERO(...) log_memory_limits (_device)
#define _LOG_MEMORY_LIMITS_ONE(device) log_memory_limits (device)
#define LOG_MEMORY_LIMITS(...)                                                \
  _GETM_ONE (__VA_OPT__ (, ) _LOG_MEMORY_LIMITS_ONE,                          \
             _LOG_MEMORY_LIMITS_ZERO) (__VA_ARGS__)

/**
 * @brief Log device work limit information.
 *
 * @param device The cl_device to log for.
 */
void log_work_limits (cl_device_id device);
#define _LOG_WORK_LIMITS_ZERO(...) log_work_limits (_device)
#define _LOG_WORK_LIMITS_ONE(device) log_work_limits (device)
#define LOG_WORK_LIMITS(...)                                                  \
  _GETM_ONE (__VA_OPT__ (, ) _LOG_WORK_LIMITS_ONE,                            \
             _LOG_WORK_LIMITS_ZERO) (__VA_ARGS__)

const char *_cl_err_to_str (cl_int err);
void _check_cl (cl_int err, const char *expr, int line, const char *file);
#define _CHECK_CL_ONE(expr) _check_cl ((expr), #expr, __LINE__, __FILE__)
#define _CHECK_CL_TWO(expr, err)                                              \
  expr;                                                                       \
  _check_cl (err, #expr, __LINE__, __FILE__)
/**
 * @brief Check and log errors in argument OpenCL library expression.
 *
 * @param expr Expression to evaluate.
 * @param err Optiona. Separate error value to error check instead.
 *
 * Example usage:
 * @code
 * // Check function that returns cl_int
 * CHECK_CL(clReleaseMemObject(arr.device));
 * // Check function that writes to a cl_int
 * cl_int err;
 * context = CHECK_CL (clCreateContext (NULL, 1, device, NULL, NULL, &err),
 *                     err);
 * @endcode
 */
#define CHECK_CL(...)                                                         \
  _GETM_TWO (__VA_ARGS__, _CHECK_CL_TWO, _CHECK_CL_ONE) (__VA_ARGS__)

/**
 * @brief Attempt to build cl_program and log errors.
 *
 * @param program cl_program to build.
 * @param device cl_device_id of device to build program for.
 */
void try_build_program (cl_program program, cl_device_id device);
#define _TRY_BUILD_PROGRAM_ONE(program) try_build_program (program, _device)
#define _TRY_BUILD_PROGRAM_TWO(program, device)                               \
  try_build_program (program, device)
#define TRY_BUILD_PROGRAM(...)                                                \
  _GETM_TWO (__VA_ARGS__, _TRY_BUILD_PROGRAM_TWO,                             \
             _TRY_BUILD_PROGRAM_ONE) (__VA_ARGS__)

/**
 * @brief Set @ref array "arrays" as arguments to an OpenCL kernel.
 *
 * For each @ref array argument passed, sets its dimensions and device-side
 * buffer as a kernel argument.
 *
 * @param kernel OpenCL kernel to set arguments for.
 * @param num_args Number of arguments.
 * @param ... List of @ref array "arrays" to set as arguments.
 */
void set_kernel_args (cl_kernel kernel, int num_args, ...);
#define _SET_KERNEL_ARGS_ONE(kernel, A) set_kernel_args (kernel, 1, A)
#define _SET_KERNEL_ARGS_TWO(kernel, A, B) set_kernel_args (kernel, 2, A, B)
#define _SET_KERNEL_ARGS_THREE(kernel, A, B, C)                               \
  set_kernel_args (kernel, 3, A, B, C)
/**
 * @brief Set @ref array "arrays" as arguments to an OpenCL kernel.
 *
 * For each @ref array argument passed, sets its dimensions and device-side
 * buffer as a kernel argument.
 *
 * @param kernel OpenCL kernel to set arguments for.
 * @param ... List of up to three @ref array "arrays" to set as arguments.
 */
#define SET_KERNEL_ARGS(kernel, ...)                                          \
  _GETM_THREE (__VA_ARGS__, _SET_KERNEL_ARGS_THREE, _SET_KERNEL_ARGS_TWO,     \
               _SET_KERNEL_ARGS_ONE) (kernel, __VA_ARGS__)

void _log_cl_event_time (cl_event event, const char *expr);
/**
 * @brief Logs time taken by a cl_event.
 *
 * Requires that cl_command_queue be created with profiling enabled.
 *
 * @param event The cl_event to log.
 */
#define LOG_CL_EVENT_TIME(event) _log_cl_event_time ((event), #event)

#define _TYPE_LIST                                                            \
  X (void, TYPE_UNKNOWN, host)                                                \
  X (float, TYPE_FLOAT, floats)                                               \
  X (double, TYPE_DOUBLE, doubles)                                            \
  X (char, TYPE_CHAR, chars)                                                  \
  X (short, TYPE_SHORT, shorts)                                               \
  X (int, TYPE_INT, ints)                                                     \
  X (long, TYPE_LONG, longs)                                                  \
  X (bool, TYPE_BOOL, bools)

// TODO: either use or remove
#define _VECTOR_TYPE_LIST                                                     \
  X (char2, TYPE_CHAR2, char2s)                                               \
  X (char4, TYPE_CHAR4, char4s)                                               \
  X (char8, TYPE_CHAR8, char8s)                                               \
  X (char16, TYPE_CHAR16, char16s)                                            \
  X (uchar2, TYPE_UCHAR2, uchar2s)                                            \
  X (uchar4, TYPE_UCHAR4, uchar4s)                                            \
  X (uchar8, TYPE_UCHAR8, uchar8s)                                            \
  X (uchar16, TYPE_UCHAR16, uchar16s)                                         \
  X (short2, TYPE_SHORT2, short2s)                                            \
  X (short4, TYPE_SHORT4, short4s)                                            \
  X (short8, TYPE_SHORT8, short8s)                                            \
  X (short16, TYPE_SHORT16, short16s)                                         \
  X (ushort2, TYPE_USHORT2, ushort2s)                                         \
  X (ushort4, TYPE_USHORT4, ushort4s)                                         \
  X (ushort8, TYPE_USHORT8, ushort8s)                                         \
  X (ushort16, TYPE_USHORT16, ushort16s)                                      \
  X (int2, TYPE_INT2, int2s)                                                  \
  X (int4, TYPE_INT4, int4s)                                                  \
  X (int8, TYPE_INT8, int8s)                                                  \
  X (int16, TYPE_INT16, int16s)                                               \
  X (uint2, TYPE_UINT2, uint2s)                                               \
  X (uint4, TYPE_UINT4, uint4s)                                               \
  X (uint8, TYPE_UINT8, uint8s)                                               \
  X (uint16, TYPE_UINT16, uint16s)                                            \
  X (long2, TYPE_LONG2, long2s)                                               \
  X (long4, TYPE_LONG4, long4s)                                               \
  X (long8, TYPE_LONG8, long8s)                                               \
  X (long16, TYPE_LONG16, long16s)                                            \
  X (ulong2, TYPE_ULONG2, ulong2s)                                            \
  X (ulong4, TYPE_ULONG4, ulong4s)                                            \
  X (ulong8, TYPE_ULONG8, ulong8s)                                            \
  X (ulong16, TYPE_ULONG16, ulong16s)                                         \
  X (float2, TYPE_FLOAT2, float2s)                                            \
  X (float4, TYPE_FLOAT4, float4s)                                            \
  X (float8, TYPE_FLOAT8, float8s)                                            \
  X (float16, TYPE_FLOAT16, float16s)                                         \
  X (double2, TYPE_DOUBLE2, double2s)                                         \
  X (double4, TYPE_DOUBLE4, double4s)                                         \
  X (double8, TYPE_DOUBLE8, double8s)                                         \
  X (double16, TYPE_DOUBLE16, double16s)

/**
 * @brief Enum representing @ref array element types.
 */
typedef enum
{
#define X(_, enum_name, __) enum_name,
  _TYPE_LIST
#undef X
} array_type;

enum
{
#define X(type, enum_name, _) _TYPE_MAP_##type = enum_name,
  _TYPE_LIST
#undef X
};
#define TYPE_TO_ENUM(type) _TYPE_MAP_##type

#define X(type, _, __) sizeof (type),
static const size_t _size_of_type_map[] = { _TYPE_LIST };
#undef X
#define SIZE_FROM_ENUM(enum) _size_of_type_map[enum]

#define X(type, _, __) #type,
static const char *_str_of_type_map[] = { _TYPE_LIST };
#undef X
#define TYPE_STR_FROM_ENUM(enum) _str_of_type_map[enum]

/**
 * @struct array
 * @brief Struct array container used by most library functions.
 *
 * Contains host-side and device-side buffer pointers, array element type and
 * dimensions.
 *
 * Has an union of pointers named for every array @ref array_type "type".
 */
typedef struct
{
  int dim1, dim2, dim3;
  size_t membsize;
  cl_mem device;
  array_type type;
  union
  {
#define X(type, _, name) type *name;
    _TYPE_LIST
#undef X
  };
} array;

/**
 * @brief Allocate new @ref array.
 *
 * Allocates @ref array with device-side and host-side buffers for
 * given type, flags, and dimensions.
 *
 * @param type @ref array_type "Type" of array elements.
 * @param flags Flags for @ref array device-side buffer.
 * @param dim1 Size of first dimension.
 * @param dim2 Size of second dimension.
 * @param dim3 Size of third dimension.
 * @return New @ref array.
 */
array alloc_array (array_type type, cl_mem_flags flags, size_t dim1,
                   size_t dim2, size_t dim3);
#define _ALLOC_ARRAY_ONE(type, flags, dim1)                                   \
  alloc_array (type, flags, dim1, 1, 1)
#define _ALLOC_ARRAY_TWO(type, flags, dim1, dim2)                             \
  alloc_array (type, flags, dim1, dim2, 1)
#define _ALLOC_ARRAY_THREE(type, flags, dim1, dim2, dim3)                     \
  alloc_array (type, flags, dim1, dim2, dim3)
#define ALLOC_ARRAY(type, flags, ...)                                         \
  _GETM_THREE (__VA_ARGS__, _ALLOC_ARRAY_THREE, _ALLOC_ARRAY_TWO,             \
               _ALLOC_ARRAY_ONE) ((array_type)TYPE_TO_ENUM (type), flags,     \
                                  __VA_ARGS__) /**< @copydoc alloc_array */

/**
 * @brief The size of an @ref array.
 */
#define ARRAY_SIZE(arr) (arr.dim1 * arr.dim2 * arr.dim3)

/**
 * @brief Number of dimensions greater than 1 of an @ref array.
 */
#define ARRAY_NUM_DIMS(arr) ((arr.dim1 > 1) + (arr.dim2 > 1) + (arr.dim3 > 1))

/**
 * @brief Synchronizes device-side buffer of an @ref array.
 *
 * Writes host-side buffer contents of @ref array into device-side
 * buffer. Write does not block unless cl_event argument is NULL.
 *
 * @param arr @ref array "Array" to sync.
 * @param event The event to attach to the buffer write operation.
 */
void sync_array_to_device (array arr, cl_event *event);
#define _SYNC_ARRAY_TO_DEVICE_ONE(arr) sync_array_to_device (arr, NULL)
#define _SYNC_ARRAY_TO_DEVICE_TWO(arr, event) sync_array_to_device (arr, event)
#define SYNC_ARRAY_TO_DEVICE(...)                                             \
  _GETM_TWO (__VA_ARGS__, _SYNC_ARRAY_TO_DEVICE_TWO,                          \
             _SYNC_ARRAY_TO_DEVICE_ONE) (                                     \
      __VA_ARGS__) /**< @copydoc sync_array_to_device */

/**
 * @brief Synchronizes host-side buffer of an @ref array.
 *
 * Reads device-side buffer contents of @ref array into host-side
 * buffer. Read does not block unless cl_event argument is NULL.
 *
 * @param arr @ref array "Array" to sync.
 * @param event The event to attach to the buffer read operation.
 */
void sync_array_from_device (array arr, cl_event *event);
#define _SYNC_ARRAY_FROM_DEVICE_ONE(arr) sync_array_from_device (arr, NULL)
#define _SYNC_ARRAY_FROM_DEVICE_TWO(arr, event)                               \
  sync_array_from_device (arr, event)
#define SYNC_ARRAY_FROM_DEVICE(...)                                           \
  _GETM_TWO (__VA_ARGS__, _SYNC_ARRAY_FROM_DEVICE_TWO,                        \
             _SYNC_ARRAY_FROM_DEVICE_ONE) (                                   \
      __VA_ARGS__) /**< @copydoc sync_array_from_device */

/**
 * @brief Make a clone of an @ref array.
 *
 * Clones an @ref array, making a copy with different memory flags, but
 * equal dimensions and type. Allocates new host-side and device-side buffers,
 * with copy of contents.
 *
 * @param arr @ref array "Array" to be cloned.
 * @param flags The memory flags for the device-side buffer of the new @ref
 * array.
 */
array clone_array (array arr, cl_mem_flags flags);
#define CLONE_ARRAY(arr, flags)                                               \
  clone_array (arr, flags) /**< @copydoc clone_array */

/**
 * @brief Free an @ref array.
 *
 * Releases the device-side buffer of the @ref array and the host-side
 * allocated memory.
 *
 * @param arr @ref array "Array" to free.
 */
void free_array (array arr);
#define FREE_ARRAY(arr) free_array (arr) /**< @copydoc free_array */

/**
 * @brief Print an @ref array.
 *
 * Prints the contents of an @ref array in human readable format.
 *
 * @param arr @ref array "Array" to print.
 */
void print_array (array arr);
#define PRINT_ARRAY(arr) print_array (arr) /**< @copydoc print_array */

#endif // CL_UTILS_H_
