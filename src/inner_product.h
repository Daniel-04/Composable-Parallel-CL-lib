/**
 * @file inner_product.h
 */

#ifndef INNER_PRODUCT_H_
#define INNER_PRODUCT_H_

#include "cl_utils.h"

extern const char *_inner_product_fmt;
/**
 * @brief Composes inner product kernel.
 *
 * Constructs the kernel with the specified types and operations, return an
 * allocated null-terminated string containing the kernel.
 *
 * The caller is responsible for freeing the string.
 *
 * Onto the third input argument C, the result of evaluating the op2 is
 * written for each row-column pair of A and B, reduced by op1. op1 and op2
 * must be binary operators.
 *
 * @param atype String for type of first @ref array of kernel: A.
 * @param btype String for type of second @ref array of kernel: B.
 * @param ctype String for type of third @ref array of kernel: C.
 * @param op1 String for the reducing operation the kernel performs.
 * @param op2 String for the pairwise operation the kernel performs.
 * @return Pointer to null-terminated string.
 */
char *get_inner_product (const char *atype, const char *btype,
                         const char *ctype, const char *op1, const char *op2);
/**
 * @brief Perform inner product operation.
 *
 * Calls inner product kernel on given input @ref array "arrays". Blocks and
 * attempts to record timing if no cl_event is provided, non blocking
 * otherwise.
 *
 * @param op1 String of reducing operation to perform.
 * @param op2 String of pairwise operation to perform.
 * @param A First argument @ref array of the kernel.
 * @param B Second argument @ref array of the kernel.
 * @param C Third argument @ref array of the kernel.
 * @param event cl_event to be attached to the kernel call.
 * @return Nanoseconds taken, or 0 if not timing or queue profiling disabled.
 *
 * Example usage:
 * @code
 * // Matrix multiplication
 * cl_event event;
 * INNER_PRODUCT("+", "*", A, B, C, &event);
 * clWaitForEvents(1, &event);
 * @endcode
 */
unsigned long long inner_product (const char *op1, const char *op2, array A,
                                  array B, array C, cl_event *event);
#define _INNER_PRODUCT_ONE(op1, op2, A, B, C)                                 \
  inner_product (op1, op2, A, B, C, NULL);
#define _INNER_PRODUCT_TWO(op1, op2, A, B, C, event)                          \
  inner_product (op1, op2, A, B, C, event)
#define INNER_PRODUCT(...)                                                    \
  _GETM_SIX (__VA_ARGS__, _INNER_PRODUCT_TWO,                                 \
             _INNER_PRODUCT_ONE) (__VA_ARGS__) /**< @copydoc inner_product*/

#endif // INNER_PRODUCT_H_
