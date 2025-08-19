/**
 * @file transpose.h
 */

#ifndef TRANSPOSE_H_
#define TRANSPOSE_H_

#include "cl_utils.h"

extern const char *_transpose_fmt;
/**
 * @brief Composes transpose kernel.
 *
 * Constructs the kernel with the specified types and operations, return an
 * allocated null-terminated string containing the kernel.
 *
 * The caller is responsible for freeing the string.
 *
 * Onto the second input argument B, the transposition of A is written.
 *
 * @param atype String for type of first @ref array of kernel: A.
 * @param btype String for type of second @ref array of kernel: B.
 * @return Pointer to null-terminated string.
 */
char *get_transpose (const char *dtype);
/**
 * @brief Perform transpose operation.
 *
 * Calls transpose kernel on given input @ref array "arrays". Blocks and
 * attempts to record timing if no cl_event is provided, non blocking
 * otherwise.
 *
 * @param A First argument @ref array of the kernel.
 * @param B Second argument @ref array of the kernel.
 * @param event cl_event to be attached to the kernel call.
 * @return Nanoseconds taken, or 0 if not timing or queue profiling disabled.
 *
 * Example usage:
 * @code
 * cl_event event;
 * TRANSPOSE(A, B, &event);
 * clWaitForEvents(1, &event);
 * @endcode
 */
unsigned long long transpose (array A, array B, cl_event *event);
#define _TRANSPOSE_ONE(A, B) transpose (A, B, NULL);
#define _TRANSPOSE_TWO(A, B, event) transpose (A, B, event)
#define TRANSPOSE(...)                                                        \
  _GETM_THREE (__VA_ARGS__, _TRANSPOSE_TWO,                                   \
               _TRANSPOSE_ONE) (__VA_ARGS__) /**< @copydoc transpose*/

#endif // TRANSPOSE_H_
