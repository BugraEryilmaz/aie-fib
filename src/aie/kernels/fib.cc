/* A simple kernel
 */
#include <adf.h>
#include "fib.h"

void fib(input_stream_uint32 * taskIn, output_stream_uint32 * taskOut_argDataOut, input_stream_uint32 * closureIn, output_stream_uint32 * argOut_SpawnNext) {
  uint32_t f1;
  uint32_t f2;
  fib_task args;
  read_hs(taskIn, args);
  if ((args.n < 2)) {
    stream_union_2<arg_out, fib_cont0_spawn_next> cont;
    cont.tag = 0;
    cont.t1.data = args._cont;
    write_hs(argOut_SpawnNext, cont);
    stream_union_2<fib_task, uint32_t_arg_out> result;
    result.tag = 1;
    result.t2.addr = args._cont;
    result.t2.data = args.n;
    result.t2.size = 2;
    result.t2.allow = 1;
    write_hs(taskOut_argDataOut, result);
  } else {
    uint32_t SN_fib_cont0c_cnt = 2;
    fib_cont0_task SN_fib_cont0c;
    SN_fib_cont0c._cont = args._cont;
    SN_fib_cont0c._counter = SN_fib_cont0c_cnt;
    closure_in SN_fib_cont0c_k;
    read_hs(closureIn, SN_fib_cont0c_k);
    
    stream_union_2<fib_task, uint32_t_arg_out> fib_args0;
    fib_args0.tag = 0;
    fib_args0.t1._cont = SN_fib_cont0c_k.data + offsetof(fib_cont0_task, f1);
    fib_args0.t1.n = (args.n - 1);
    write_hs(taskOut_argDataOut, fib_args0);
    
    stream_union_2<fib_task, uint32_t_arg_out> fib_args1;
    fib_args1.tag = 0;
    fib_args1.t1._cont = SN_fib_cont0c_k.data + offsetof(fib_cont0_task, f2);
    fib_args1.t1.n = (args.n - 2);
    write_hs(taskOut_argDataOut, fib_args1);

    stream_union_2<arg_out, fib_cont0_spawn_next> SN_fib_cont0;
    SN_fib_cont0.tag = 1;
    SN_fib_cont0.t2.addr = SN_fib_cont0c_k.data;
    SN_fib_cont0.t2.data = SN_fib_cont0c;
    SN_fib_cont0.t2.size = 5;
    SN_fib_cont0.t2.allow = SN_fib_cont0c_cnt;
    write_hs(argOut_SpawnNext, SN_fib_cont0);
  }
}
