#include "map.h"
#include <stdio.h>

/* Format strings:
 * 1. A type
 * 2. B type
 * 3. OP1
 */

const char *_map_fmt = RAW (__kernel void entry (
    const int a1, const int a2, const int a3, __global const % s * A,
    const int b1, const int b2, const int b3, __global % s * B) {
  int idx = get_global_id (0);
  int total = a1 * a2 * a3;
  if (idx < total)
    {
      B[idx] = % s (A[idx]);
    }
});

char *
_get_map (const char *dtype, const char *op1)
{
  char *kernel = NULL;

  int count = asprintf (&kernel, _map_fmt, dtype, dtype, op1);

  if (count == -1)
    {
      handle_error ("Failed to print to kernel string");
    }
  return kernel;
}
