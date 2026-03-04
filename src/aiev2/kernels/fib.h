#ifndef FIB_HELPERS
#define FIB_HELPERS
#define __AIENGINE__

#include "adf.h"

struct fib_cont0_task {
  uint32_t _counter;
  uint32_t f1;
  uint64_t _cont;
  uint32_t f2;
  uint32_t _padding[3];
};

struct fib_cont0_spawn_next {
  uint64_t addr;
  fib_cont0_task data;
  uint32_t size;
  uint32_t allow;
  uint32_t _padding[4];
};

struct uint32_t_arg_out {
  uint64_t addr;
  uint32_t data;
  uint32_t size;
  uint32_t allow;
  uint32_t _padding[3];
};

struct fib_task {
  uint64_t _cont;
  uint32_t n;
  uint32_t _padding[5];
};

struct arg_out {
  uint64 data;
};

struct closure_in {
  uint64 data;
};

// void read_hs(input_window_uint64 *in, arg_out &ret) {
//   window_readincr(in, &ret.data);
// }

// void write_hs(output_window_uint64 *out, arg_out &value) {
//   window_writeincr(out, value.data);
// }

// void read_hs(input_window_uint64 *in, closure_in &ret) {
//   window_readincr(in, &ret.data);
// }

// void write_hs(output_window_uint64 *out, closure_in &value) {
//   window_writeincr(out, value.data);
// }

// void read_hs(input_window_uint64 *in, fib_cont0_task &ret) {
//   // Reads both _counter and f1
//   window_readincr(in, &ret._counter);
//   window_readincr(in, &ret._cont);
//   // Reads f2 and 32bit padding
//   window_readincr(in, &ret.f2);
//   window_readincr(in, &ret._padding[1]);
// }

// void write_hs(output_window_uint64 *out, fib_cont0_task &value) {
//   // writes _counter and f1
//   window_writeincr(out, value._counter);
//   window_writeincr(out, value._cont);
//   // writes f2 and 32bit padding
//   window_writeincr(out, value.f2);
//   // padding
//   window_writeincr(out, 0);
// }

// void read_hs(input_window_uint64 *in, fib_cont0_spawn_next &ret) {
//   window_readincr(in, &ret.addr);
//   read_hs(in, ret.data);
//   //   Reads both size and allow
//   window_readincr(in, &ret.size);
//   // padding
//   for (int i = 0; i < 2; i++) {
//     window_readincr(in, &ret._padding[i * 2]);
//   }
// }

// void write_hs(output_window_uint64 *out, fib_cont0_spawn_next &value) {
//   window_writeincr(out, value.addr);
//   write_hs(out, value.data);
//   //   writes both size and allow
//   window_writeincr(out, value.size | ((uint64_t)(value.allow) << 32));
//   // padding
//   for (int i = 0; i < 2; i++) {
//     window_writeincr(out, 0);
//   }
// }

// void read_hs(input_window_uint64 *in, uint32_t_arg_out &ret) {
//   window_readincr(in, &ret.addr);
//   // Reads data and size
//   window_readincr(in, &ret.data);
//   // Reads allow and 32bit padding
//   window_readincr(in, &ret.allow);
//   // padding
//   window_readincr(in, &ret._padding[1]);
// }

// void write_hs(output_window_uint64 *out, uint32_t_arg_out &value) {
//   window_writeincr(out, value.addr);
//   //   writes data and size
//   window_writeincr(out, value.data | ((uint64_t)(value.size) << 32));
//   //   writes allow and 32bit padding
//   window_writeincr(out, value.allow);
//   // padding
//   window_writeincr(out, 0);
// }

// void read_hs(input_window_uint64 *in, fib_task &ret) {
//   window_readincr(in, &ret._cont);
//   window_readincr(in, &ret.n);
//   // padding
//   for (int i = 0; i < 2; i++) {
//     window_readincr(in, &ret._padding[i]);
//   }
// }

// void write_hs(output_window_uint64 *out, fib_task &value) {
//   window_writeincr(out, value._cont);
//   window_writeincr(out, value.n);
//   // padding
//   for (int i = 0; i < 2; i++) {
//     window_writeincr(out, 0);
//   }
// }

template <typename T1, typename T2> struct stream_union_2 {
  union {
    T1 t1;
    T2 t2;
  };
  uint32 _padding[3];
  uint32 tag;
};

// template <typename T1, typename T2>
// void read_hs(input_window_uint64 *in, stream_union_2<T1, T2> &ret) {
//   for (int i = 0; i < sizeof(ret) / 8; i++) {
//     window_readincr(in, ((uint64_t *)&ret) + i);
//   }
// }

// template <typename T1, typename T2>
// void write_hs(output_window_uint64 *out, stream_union_2<T1, T2> &value) {
//   for (int i = 0; i < sizeof(value) / 8; i++) {
//     window_writeincr(out, *((uint64_t *)&value) + i);
//   }
// }
#endif