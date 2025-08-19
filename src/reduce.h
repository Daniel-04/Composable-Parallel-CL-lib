/**
 * @file reduce.h
 */

#ifndef REDUCE_H_
#define REDUCE_H_

#include "cl_utils.h"

extern const char *_reduce_1step_fmt;
/**
 * @brief Composes one step partial reduction kernel.
 *
 * Constructs the kernel with the specified types and operations, return an
 * allocated null-terminated string containing the kernel.
 *
 * The caller is responsible for freeing the string.
 *
 * Onto the input argument A, the operation is evaluated on pairs of row
 * elements in undefined order, reducing results into the first column of each
 * row. Variables `a` and `b` hold the current pair.
 *
 * @param atype String for type of first @ref array of kernel: A.
 * @param op1 String for the operation the kernel performs.
 * @return Pointer to null-terminated string.
 */
char *get_reduce_1step (const char *dtype, const char *op1);
/**
 * @brief Perform reduction operation.
 *
 * Calls reduction step kernels repeatedly on given input @ref array "arrays".
 * Blocks and attempts to record timing if no cl_event is provided, non
 * blocking otherwise.
 *
 * @param op1 String of operation to perform.
 * @param A First argument @ref array of the kernel.
 * @param event cl_event to be attached to the kernel calls.
 * @return Nanoseconds taken, or 0 if not timing or queue profiling disabled.
 *
 * Example usage:
 * Evaluations of the input operation are performed on pairs of elements and
 * written onto the array until fully reduced, within the scope of which the
 * variables `a` and `b` exist that hold the values of the current pair.
 * @code
 * // Sum of my_array
 * cl_event event;
 * REDUCE("a + b", my_array, &event);
 * clWaitForEvents(1, &event);
 * // Product of my_array
 * int nanos = REDUCE("a * b", my_array);
 * printf("Kernel took %d ms\n", nanos * 1e-6);
 * @endcode
 */
unsigned long long reduce (const char *op1, array A, cl_event *event);
#define _REDUCE_ONE(op1, A) reduce (op1, A, NULL);
#define _REDUCE_TWO(op1, A, event) reduce (op1, A, event)
#define REDUCE(...)                                                           \
  _GETM_THREE (__VA_ARGS__, _REDUCE_TWO,                                      \
               _REDUCE_ONE) (__VA_ARGS__) /**< @copydoc reduce*/

#endif // REDUCE_H_
