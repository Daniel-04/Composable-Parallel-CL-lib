/**
 * @file scan.h
 */

#ifndef SCAN_H_
#define SCAN_H_

#include "cl_utils.h"

extern const char *_partial_scan_fmt;
extern const char *_propagate_scan_fmt;
/**
 * @brief Composes partial scan kernel.
 *
 * Constructs the kernel with the specified types and operations, return an
 * allocated null-terminated string containing the kernel.
 *
 * The caller is responsible for freeing the string.
 *
 * Onto the input argument A, the operation is evaluated on pairs of row
 * elements in rightwards within the work group, scanning results for each work
 * group. Variables `a` and `b` hold the current pair.
 *
 * @param atype String for type of first @ref array of kernel: A.
 * @param op1 String for the operation the kernel performs.
 * @return Pointer to null-terminated string.
 */
char *get_partial_scan (const char *dtype, const char *op1);
/**
 * @brief Composes propagation step reduction kernel.
 *
 * Constructs the kernel with the specified types and operations, return an
 * allocated null-terminated string containing the kernel.
 *
 * The caller is responsible for freeing the string.
 *
 * For the input argument A, the partial results across strides determined by
 * the stride argument are propagated forward. Variables `a` and `b` hold the
 * current partial-target pair.
 *
 * @param atype String for type of first @ref array of kernel: A.
 * @param op1 String for the operation the kernel performs.
 * @return Pointer to null-terminated string.
 */
char *get_propagate_scan (const char *dtype, const char *op1);
/**
 * @brief Perform scan operation.
 *
 * Calls partial scan kernel followed by propagation step kernels repeatedly on
 * given input @ref array "arrays". Blocks and attempts to record timing if no
 * cl_event is provided, non blocking otherwise.
 *
 * @param op1 String of operation to perform.
 * @param A First argument @ref array of the kernel.
 * @param event cl_event to be attached to the kernel calls.
 * @return Nanoseconds taken, or 0 if not timing or queue profiling disabled.
 *
 * Example usage:
 * Evaluations of the input operation are performed on pairs of elements and
 * written onto the array until fully scanned, within the scope of which the
 * variables `a` and `b` exist that hold the values of the current pair.
 * @code
 * // Prefix sum of my_array
 * cl_event event;
 * SCAN("a + b", my_array, &event);
 * clWaitForEvents(1, &event);
 * // Prefix product of my_array
 * int nanos = SCAN("a * b", my_array);
 * printf("Kernel took %d ms\n", nanos * 1e-6);
 * @endcode
 */
unsigned long long scan (const char *op1, array A, cl_event *event);
#define _SCAN_ONE(op1, A) scan (op1, A, NULL);
#define _SCAN_TWO(op1, A, event) scan (op1, A, event)
#define SCAN(...)                                                             \
  _GETM_THREE (__VA_ARGS__, _SCAN_TWO,                                        \
               _SCAN_ONE) (__VA_ARGS__) /**< @copydoc scan*/

#endif // SCAN_H_
