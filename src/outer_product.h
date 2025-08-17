/**
 * @file outer_product.h
 */

#ifndef OUTER_PRODUCT_H_
#define OUTER_PRODUCT_H_

#include "cl_utils.h"

extern const char *_outer_product_fmt;
/**
 * @brief Composes outer product kernel.
 *
 * Constructs the kernel with the specified types and operations, return an
 * allocated null-terminated string containing the kernel.
 *
 * The caller is responsible for freeing the string.
 *
 * Onto the third input argument C, the result of evaluating the operation is
 * written for each index. Variables `a` and `b` hold the values of A and B at
 * the current row index and column index respectively.
 *
 * @param atype String for type of first @ref array of kernel: A.
 * @param btype String for type of second @ref array of kernel: B.
 * @param ctype String for type of third @ref array of kernel: C.
 * @param op1 String for the operation the kernel performs.
 * @return Pointer to null-terminated string.
 */
char *get_outer_product (const char *atype, const char *btype,
                         const char *ctype, const char *op1);
/**
 * @brief Perform outer product operation.
 *
 * Calls outer product kernel on given input @ref array "arrays". Blocks and
 * attempts to record timing if no cl_event is provided, non blocking
 * otherwise.
 *
 * @param op1 String of operation to perform.
 * @param A First argument @ref array of the kernel.
 * @param B Second argument @ref array of the kernel.
 * @param C Third argument @ref array of the kernel.
 * @param event cl_event to be attached to the kernel call.
 * @return Nanoseconds taken, or 0 if not timing or queue profiling disabled.
 *
 * Example usage:
 * Onto each row-column pair of the third input array is written the evaluation
 * of the input operation, within the scope of which the variables `a` and `b`
 * exist that hold the values of the first and second arrays at the current
 * index respectively.
 * @code
 * // Sum of every pair of elements of array_D and array_E
 * cl_event event;
 * OUTER_PRODUCT("a + b", array_D, array_E, array_F, &event);
 * clWaitForEvents(1, &event);
 * // Euclidean distance of every pair of elements
 * int nanos = OUTER_PRODUCT("sqrt(a*a + b*b)", A, B, C);
 * printf("Kernel took %d ms\n", nanos * 1e-6);
 * @endcode
 */
unsigned long long outer_product (const char *op1, array A, array B, array C,
                                  cl_event *event);
#define _OUTER_PRODUCT_ONE(op1, A, B, C) outer_product (op1, A, B, C, NULL);
#define _OUTER_PRODUCT_TWO(op1, A, B, C, event)                               \
  outer_product (op1, A, B, C, event)
#define OUTER_PRODUCT(...)                                                    \
  _GETM_FIVE (__VA_ARGS__, _OUTER_PRODUCT_TWO,                                \
              _OUTER_PRODUCT_ONE) (__VA_ARGS__) /**< @copydoc outer_product*/

#endif // OUTER_PRODUCT_H_
