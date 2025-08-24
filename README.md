# Library of utilities and common data parallel operations to ease composition with OpenCL
Collection of C utilities to simplify common tasks in OpenCL programming. It's designed to greatly reduce boilerplate code for OpenCL pipeline management, memory handling, error checking, and performance profiling.

-----

## Building
``` sh
make              # makes a static release target
make shared debug # shared debug target
make static debug # static debug

make examples/    # compiles and links the examples in examples/
make bench/       # compiles and links the benchmarks in bench/

make docs         # makes documentation
```

-----

## Features
  * Simplified OpenCL Pipeline Management
  * Flexible Array Container
  * Customizable Error Handling
  * Performance Profiling
  * Kernel Compilation and Execution

-----

## Examples

The `examples/` and `bench/` directories contain several programs to demonstrate functionality and usage.

-----

## API Reference

### Core Utilities

  * `setup_cl()`: Initializes a default OpenCL pipeline, including the platform, device, context, and command queue.
  * `copy_cl_pipeline()`: Copies user initialized OpenCL pipeline for library use.
  * `release_cl()`: Releases the resources initialized by `setup_cl()`.
  * `CHECK_CL()`: A macro for error checking the return status of OpenCL API calls.
  * `TRY_COMPILE_KERNEL()`: Compiles an OpenCL kernel from a source string.
  * `LOG_DEVICES()`, `LOG_MEMORY_LIMITS()`, `LOG_WORK_LIMITS()`: Functions to log information about the available devices and their capabilities.

### `array` Data Structure
The `array` struct is the main data container.
  * `ALLOC_ARRAY()`: Allocates and initializes an `array` with host and device memory.
  * `FREE_ARRAY()`: Deallocates the memory associated with an `array`.
  * `SYNC_ARRAY_TO_DEVICE()`: Copies data from the host buffer to the device buffer.
  * `SYNC_ARRAY_FROM_DEVICE()`: Copies data from the device buffer to the host buffer.
  * `ARRAY_SIZE()`: A macro that returns the total number of elements in an `array`.

-----

## Animations

`animations.py` contains a number of manim animations demonstrating some
optimisations of present in the included kernels.

after installing manim, these can be run with:
``` sh
manim -pqk animations.py
```
