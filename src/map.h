/**
 * @file map.h
 */

#ifndef MAP_H_
#define MAP_H_

#include "cl_utils.h"

extern const char *_map_fmt;
/**
 * @brief Composes mapping kernel.
 *
 * Constructs the kernel with the specified types and operations, return an
 * allocated null-terminated string containing the kernel.
 *
 * The caller is responsible for freeing the string.
 *
 * Onto the second input argument B, the result of evaluating the operation is
 * written for each index. Variables `a` and `b` hold the values of A and B at
 * the current index respectively.
 *
 * @param atype String for type of first @ref array of kernel: A.
 * @param btype String for type of second @ref array of kernel: B.
 * @param op1 String for the operation the kernel maps.
 * @return Pointer to null-terminated string.
 */
char *get_map (const char *atype, const char *btype, const char *op1);
/**
 * @brief Perform mapping operation.
 *
 * Calls mapping kernel on given input @ref array "arrays". Blocks and attempts
 * to record timing if no cl_event is provided, non blocking otherwise.
 *
 * @param op1 String of operation to map.
 * @param A First argument @ref array of the kernel.
 * @param B Second argument @ref array of the kernel.
 * @param event cl_event to be attached to the kernel call.
 * @return Nanoseconds taken, or 0 if not timing or queue profiling disabled.
 *
 * Example usage:
 * Onto each index of the second input array is written the evaluation of the
 * input operation, within the scope of which the variables `a` and `b` exist
 * that hold the values of the first and second arrays at the current index
 * respectively.
 * @code
 * // Write the square root of each value of A into B
 * MAP("sqrt(a)", A, B);
 * // Sum element-wise array_C and array_D
 * cl_event event;
 * MAP("a + b", array_C, array_D, &event);
 * clWaitForEvents(1, &event);
 * // Euclidean distance of pairs of values
 * int nanos = MAP("sqrt(pow(a, 2) + pow(b, 2))", A, B);
 * printf("Kernel took %d ms\n", nanos * 1e-6);
 * @endcode
 */
unsigned long long map (const char *op1, array A, array B, cl_event *event);
#define _MAP_ONE(op1, A, B) map (op1, A, B, NULL);
#define _MAP_TWO(op1, A, B, event) map (op1, A, B, event)
#define MAP(...)                                                              \
  _GETM_FOUR (__VA_ARGS__, _MAP_TWO,                                          \
              _MAP_ONE) (__VA_ARGS__) /**< @copydoc map */

#endif // MAP_H_
