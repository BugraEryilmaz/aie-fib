/* A simple kernel
 */
#include <adf.h>
#include "fib.h"

void fib_cont0(input_stream_uint32 * taskIn, output_stream_uint32 * argDataOut, output_stream_uint32 * argOut) {
  fib_cont0_task args;
  read_hs(taskIn, args);

  arg_out cont;
  cont.data = args._cont;
  write_hs(argOut, cont);

  uint32_t_arg_out a0;
  a0.addr = args._cont;
  a0.data = (args.f1 + args.f2);
  a0.size = 2;
  a0.allow = 1;
  write_hs(argDataOut, a0);
}
