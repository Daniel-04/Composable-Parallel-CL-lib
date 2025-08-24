/*
** This example is very verbose on purpose.
*/

#include <CL/cl.h>
#include <cl_utils.h>
#include <map.h>
#include <setjmp.h>
#include <stdio.h>

jmp_buf jump_buff;

void
jmp_handler (const char *err, va_list args)
{
  vfprintf (stderr, err, args);
  fprintf (stderr, "\n");

  longjmp (jump_buff, 1);
}

int
main ()
{
  set_error_handler (jmp_handler);

  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  if (setjmp (jump_buff) == 0)
    {
      setup_cl (&platform, &device, &context, &queue, NULL);
    }
  else
    {
      fprintf (stderr, "Failed to setup CL pipeline, cannot recover.");
      return 1;
    }

  volatile size_t a1 = 1 << 31;
  array A;
  while (a1 > 0)
    {
      if (setjmp (jump_buff) != 0)
        {
          fprintf (stderr,
                   "Failed to allocate array of length %zu, retrying with "
                   "length %zu.\n",
                   a1, a1 / 1024);
          a1 /= 1024;
          continue;
        }

      A = ALLOC_ARRAY (float, CL_MEM_READ_ONLY, a1);
      break;
    }
  if (a1 == 0)
    {
      fprintf (stderr, "Failed to allocate array repeatedly, cannot recover.");
      return -1;
    }

  for (int i = 0; i < ARRAY_SIZE (A); i++)
    {
      A.floats[i] = (float)i + 1;
    }

  if (setjmp (jump_buff) == 0)
    {
      SYNC_ARRAY_TO_DEVICE (A);
    }
  else
    {
      fprintf (stderr, "Failed to write array to device, cannot recover.");
      return 1;
    }

  if (setjmp (jump_buff) == 0)
    {
      MAP ("a + b", A, A);
    }
  else
    {
      fprintf (stderr, "Kernel execution failed, cannot recover.");
      return 1;
    }

  if (setjmp (jump_buff) == 0)
    {
      SYNC_ARRAY_FROM_DEVICE (A);
    }
  else
    {
      fprintf (stderr, "Failed to read array from device, cannot recover.");
      return 1;
    }

  free_array (A);
  clReleaseDevice (device);
  clReleaseContext (context);
  clReleaseCommandQueue (queue);

  return 0;
}
