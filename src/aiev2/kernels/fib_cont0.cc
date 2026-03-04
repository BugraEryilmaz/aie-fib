/* A simple kernel
 */
#include "fib.h"
#include <adf.h>

void fib_cont0(input_window_uint64 *taskIn, output_window_uint64 *argDataOut,
               output_window_uint64 *argOut) {
  fib_cont0_task *args = (fib_cont0_task *)taskIn->ptr;
  window_incr(taskIn, sizeof(fib_cont0_task) / sizeof(uint64_t));

  arg_out *cont = (arg_out *)argOut->ptr;
  cont->data = args->_cont;
  window_incr(argOut, sizeof(arg_out) / sizeof(uint64_t));

  uint32_t_arg_out *a0 = (uint32_t_arg_out *)argDataOut->ptr;
  a0->addr = args->_cont;
  a0->data = (args->f1 + args->f2);
  a0->size = 2;
  a0->allow = 1;
  window_incr(argDataOut, sizeof(uint32_t_arg_out) / sizeof(uint64_t));
}
