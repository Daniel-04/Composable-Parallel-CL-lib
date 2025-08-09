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
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Library error handler signature.
 */
typedef void (*error_handler_fn) (const char *err, ...);

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
  X (int, TYPE_INT)                                                           \
  X (float, TYPE_FLOAT)                                                       \
  X (double, TYPE_DOUBLE)

/**
 * @brief Enum representing @ref array element types.
 */
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
#define SIZE_FROM_ENUM(enum) _size_of_type_map[enum]

// I hope this is slightly more sane/easier to debug than setting unknown type
// to an empty string?
/* #define X(type, enum_name) #type, */
/* static const char *_str_of_type_map[] = { "void", _TYPE_LIST }; */
/* #undef X */
/* #define TYPE_STR_FROM_ENUM(enum) _str_of_type_map[enum] */

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
    void *host;
    int *ints;
    float *floats;
    double *doubles;
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
