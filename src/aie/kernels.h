
#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H

  void fib(input_stream_uint32 * taskIn, output_stream_uint32 * taskOut_argDataOut, input_stream_uint32 * closureIn, output_stream_uint32 * argOut_SpawnNext);

  void fib_cont0(input_stream_uint32 * taskIn, output_stream_uint32 * argDataOut, output_stream_uint32 * argOut);

#endif
