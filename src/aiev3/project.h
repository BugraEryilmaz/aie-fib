
#include "kernels.h"
#include "kernels/fib.h"
#include <adf.h>

using namespace adf;

class simpleGraph : public adf::graph {
private:
  kernel fib_kernel;
  kernel fib_cont0_kernel;

public:
  input_plio fib_taskIn; // 256 bits
  output_plio fib_taskOut_argDataOut;
  input_plio fib_closureIn;
  output_plio fib_argOut_SpawnNext;
  input_plio fib_cont0_taskIn;
  output_plio fib_cont0_argDataOut;
  output_plio fib_cont0_argOut;
  simpleGraph() {

    fib_taskIn = input_plio::create("PLIO_fib_taskIn", adf::plio_128_bits,
                                    "data/fib_taskIn.txt");
    fib_taskOut_argDataOut =
        output_plio::create("PLIO_fib_taskOut_argDataOut", adf::plio_128_bits,
                            "data/fib_taskOut_argDataOut.txt");
    fib_closureIn = input_plio::create("PLIO_fib_closureIn", adf::plio_64_bits,
                                       "data/fib_closureIn.txt");
    fib_argOut_SpawnNext =
        output_plio::create("PLIO_fib_argOut_SpawnNext", adf::plio_128_bits,
                            "data/fib_argOut_SpawnNext.txt");

    fib_cont0_taskIn =
        input_plio::create("PLIO_fib_cont0_taskIn", adf::plio_128_bits,
                           "data/fib_cont0_taskIn.txt");
    fib_cont0_argDataOut =
        output_plio::create("PLIO_fib_cont0_argDataOut", adf::plio_128_bits,
                            "data/fib_cont0_argDataOut.txt");
    fib_cont0_argOut =
        output_plio::create("PLIO_fib_cont0_argOut", adf::plio_64_bits,
                            "data/fib_cont0_argOut.txt");

    fib_kernel = kernel::create(fib);
    connect<window<sizeof(fib_task)>> net0(fib_taskIn.out[0], fib_kernel.in[0]);
    connect<stream> net1(fib_kernel.out[0], fib_taskOut_argDataOut.in[0]);
    connect<window<sizeof(closure_in)>> net2(fib_closureIn.out[0],
                                             fib_kernel.in[1]);
    connect<window<sizeof(stream_union_2<arg_out, fib_cont0_spawn_next>)>> net3(
        fib_kernel.out[1], fib_argOut_SpawnNext.in[0]);

    fib_cont0_kernel = kernel::create(fib_cont0);
    connect<window<sizeof(fib_cont0_task)>> net4(fib_cont0_taskIn.out[0],
                                                 fib_cont0_kernel.in[0]);
    connect<window<sizeof(uint32_t_arg_out)>> net5(fib_cont0_kernel.out[0],
                                                   fib_cont0_argDataOut.in[0]);
    connect<window<sizeof(arg_out)>> net6(fib_cont0_kernel.out[1],
                                          fib_cont0_argOut.in[0]);
    source(fib_kernel) = "kernels/fib.cc";
    source(fib_cont0_kernel) = "kernels/fib_cont0.cc";

    runtime<ratio>(fib_kernel) = 1;
    runtime<ratio>(fib_cont0_kernel) = 1;
  }
};
