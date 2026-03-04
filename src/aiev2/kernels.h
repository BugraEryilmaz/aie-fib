
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

void fib(input_window_uint64 *taskIn, output_window_uint64 *taskOut_argDataOut,
         input_window_uint64 *closureIn,
         output_window_uint64 *argOut_SpawnNext);

void fib_cont0(input_window_uint64 *taskIn, output_window_uint64 *argDataOut,
               output_window_uint64 *argOut);

#endif
