#include "cl_utils.h"
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

void copy_cl_pipeline(cl_platform_id *platform, cl_device_id *device, cl_context *context, cl_command_queue *queue) {
    _platform = *platform;
    _device = *device;
    _context = *context;
    _queue = *queue;
}

void setup_cl(cl_platform_id *platform, cl_device_id *device, cl_context *context, cl_command_queue *queue, cl_queue_properties *properties) {
    cl_int err;
    CHECK_CL(clGetPlatformIDs(1, platform, NULL));
    CHECK_CL(clGetDeviceIDs(*platform, DEFAULT_CL_DEVICE, 1, device, NULL));
    *context = CHECK_CL(clCreateContext(NULL, 1, device, NULL, NULL, &err), err);
    *queue = CHECK_CL(
        clCreateCommandQueueWithProperties(*context, *device, properties, &err), err);

    copy_cl_pipeline(platform, device, context, queue); // For library use
}

const char *_cl_err_to_str(cl_int err) {
    switch (err) {
    case 0:
        return "CL_SUCCESS";
    case -1:
        return "CL_DEVICE_NOT_FOUND";
    case -2:
        return "CL_DEVICE_NOT_AVAILABLE";
    case -3:
        return "CL_COMPILER_NOT_AVAILABLE";
    case -4:
        return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5:
        return "CL_OUT_OF_RESOURCES";
    case -6:
        return "CL_OUT_OF_HOST_MEMORY";
    case -7:
        return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8:
        return "CL_MEM_COPY_OVERLAP";
    case -9:
        return "CL_IMAGE_FORMAT_MISMATCH";
    case -10:
        return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11:
        return "CL_BUILD_PROGRAM_FAILURE";
    case -12:
        return "CL_MAP_FAILURE";
    case -13:
        return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14:
        return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15:
        return "CL_COMPILE_PROGRAM_FAILURE";
    case -16:
        return "CL_LINKER_NOT_AVAILABLE";
    case -17:
        return "CL_LINK_PROGRAM_FAILURE";
    case -18:
        return "CL_DEVICE_PARTITION_FAILED";
    case -19:
        return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
    case -30:
        return "CL_INVALID_VALUE";
    case -31:
        return "CL_INVALID_DEVICE_TYPE";
    case -32:
        return "CL_INVALID_PLATFORM";
    case -33:
        return "CL_INVALID_DEVICE";
    case -34:
        return "CL_INVALID_CONTEXT";
    case -35:
        return "CL_INVALID_QUEUE_PROPERTIES";
    case -36:
        return "CL_INVALID_COMMAND_QUEUE";
    case -37:
        return "CL_INVALID_HOST_PTR";
    case -38:
        return "CL_INVALID_MEM_OBJECT";
    case -39:
        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40:
        return "CL_INVALID_IMAGE_SIZE";
    case -41:
        return "CL_INVALID_SAMPLER";
    case -42:
        return "CL_INVALID_BINARY";
    case -43:
        return "CL_INVALID_BUILD_OPTIONS";
    case -44:
        return "CL_INVALID_PROGRAM";
    case -45:
        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46:
        return "CL_INVALID_KERNEL_NAME";
    case -47:
        return "CL_INVALID_KERNEL_DEFINITION";
    case -48:
        return "CL_INVALID_KERNEL";
    case -49:
        return "CL_INVALID_ARG_INDEX";
    case -50:
        return "CL_INVALID_ARG_VALUE";
    case -51:
        return "CL_INVALID_ARG_SIZE";
    case -52:
        return "CL_INVALID_KERNEL_ARGS";
    case -53:
        return "CL_INVALID_WORK_DIMENSION";
    case -54:
        return "CL_INVALID_WORK_GROUP_SIZE";
    case -55:
        return "CL_INVALID_WORK_ITEM_SIZE";
    case -56:
        return "CL_INVALID_GLOBAL_OFFSET";
    case -57:
        return "CL_INVALID_EVENT_WAIT_LIST";
    case -58:
        return "CL_INVALID_EVENT";
    case -59:
        return "CL_INVALID_OPERATION";
    case -60:
        return "CL_INVALID_GL_OBJECT";
    case -61:
        return "CL_INVALID_BUFFER_SIZE";
    case -62:
        return "CL_INVALID_MIP_LEVEL";
    case -63:
        return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64:
        return "CL_INVALID_PROPERTY";
    case -65:
        return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66:
        return "CL_INVALID_COMPILER_OPTIONS";
    case -67:
        return "CL_INVALID_LINKER_OPTIONS";
    case -68:
        return "CL_INVALID_DEVICE_PARTITION_COUNT";
    case -69:
        return "CL_INVALID_PIPE_SIZE";
    case -70:
        return "CL_INVALID_DEVICE_QUEUE";
    case -71:
        return "CL_INVALID_SPEC_ID";
    case -72:
        return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
    default:
        return "Unknown OpenCL Error";
    }
}

void _check_cl(cl_int err, const char *expr, int line, const char *file) {
    if (err != CL_SUCCESS) {
        fprintf(stderr,
                "OpenCL error %d (%s) in \"%s\" at line %d in file %s\n", err,
                _cl_err_to_str(err), expr, line, file);
        abort();
    }
}

void _log_cl_event_time(cl_event event, const char *expr) {
    cl_ulong _start, _end;
    CHECK_CL(clWaitForEvents(1, &event));
    CHECK_CL(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                                     sizeof(cl_ulong), &_start, NULL));
    CHECK_CL(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong), &_end, NULL));
    printf("EVENT: %s took %f ms\n", expr, (_end - _start) * 1e-6);
    CHECK_CL(clReleaseEvent(event));
}

array _alloc_array(size_t membsize, cl_mem_flags flags, size_t dim1, size_t dim2, size_t dim3) {
    array arr;
    size_t total_size = dim1 * dim2 * dim3;

    arr.host = malloc(total_size * membsize);
    if (!arr.host) {
        fprintf(stderr, "Memory allocation error\n");
        abort();
    }

    arr.dim1 = dim1;
    arr.dim2 = dim2;
    arr.dim3 = dim3;

    void *host_ptr = NULL;
    if ((flags & CL_MEM_COPY_HOST_PTR) || (flags & CL_MEM_USE_HOST_PTR)) {
        host_ptr = arr.host;
    }
    cl_int err;
    arr.device = CHECK_CL(clCreateBuffer(_context, flags, membsize * dim1 * dim2 * dim3, host_ptr, &err), err);

    return arr;
}

void free_array(array arr) {
    free(arr.host);
    // TODO: free device mem, zero fields
}
