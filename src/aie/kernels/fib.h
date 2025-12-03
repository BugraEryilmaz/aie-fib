#ifndef FIB_HELPERS
#define FIB_HELPERS

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

void read_hs(input_stream_uint32 * in, arg_out& ret) {
    uint32_t data_lo = readincr(in);
    uint32_t data_hi = readincr(in);
    ret.data = ((uint64)data_hi << 32) | (uint64)data_lo;
}

void write_hs(output_stream_uint32 * out, arg_out& value) {
    writeincr(out, (uint32_t)(value.data & 0xFFFFFFFF));
    writeincr(out, (uint32_t)(value.data >> 32));
}

void read_hs(input_stream_uint32 * in, closure_in& ret) {
    uint32_t data_lo = readincr(in);
    uint32_t data_hi = readincr(in);
    ret.data = ((uint64)data_hi << 32) | (uint64)data_lo;
}

void write_hs(output_stream_uint32 * out, closure_in& value) {
    writeincr(out, (uint32_t)(value.data & 0xFFFFFFFF));
    writeincr(out, (uint32_t)(value.data >> 32));
}

void read_hs(input_stream_uint32 * in, fib_cont0_task& ret) {
    ret._counter = readincr(in);
    ret.f1 = readincr(in);
    uint32_t cont_lo = readincr(in);
    uint32_t cont_hi = readincr(in);
    ret._cont = ((uint64_t)cont_hi << 32) | (uint64_t)cont_lo;
    ret.f2 = readincr(in);
    // padding
    for (int i = 0; i < 3; i++) {
        readincr(in);
    }
}

void write_hs(output_stream_uint32 * out, fib_cont0_task& value) {
    writeincr(out, value._counter);
    writeincr(out, value.f1);
    writeincr(out, (uint32_t)(value._cont & 0xFFFFFFFF));
    writeincr(out, (uint32_t)(value._cont >> 32));
    writeincr(out, value.f2);
    // padding
    for (int i = 0; i < 3; i++) {
        writeincr(out, 0);
    }
}

void read_hs(input_stream_uint32 * in, fib_cont0_spawn_next& ret) {
    uint32_t addr_lo = readincr(in);
    uint32_t addr_hi = readincr(in);
    ret.addr = ((uint64_t)addr_hi << 32) | (uint64_t)addr_lo;
    read_hs(in, ret.data);
    ret.size = readincr(in);
    ret.allow = readincr(in);
    // padding
    for (int i = 0; i < 4; i++) {
        readincr(in);
    }
}

void write_hs(output_stream_uint32 * out, fib_cont0_spawn_next& value) {
    writeincr(out, (uint32_t)(value.addr & 0xFFFFFFFF));
    writeincr(out, (uint32_t)(value.addr >> 32));
    write_hs(out, value.data);
    writeincr(out, value.size);
    writeincr(out, value.allow);
    // padding
    for (int i = 0; i < 4; i++) {
        writeincr(out, 0);
    }
}

void read_hs(input_stream_uint32 * in, uint32_t_arg_out& ret) {
    uint32_t addr_lo = readincr(in);
    uint32_t addr_hi = readincr(in);
    ret.addr = ((uint64_t)addr_hi << 32) | (uint64_t)addr_lo;
    ret.data = readincr(in);
    ret.size = readincr(in);
    ret.allow = readincr(in);
    // padding
    for (int i = 0; i < 3; i++) {
        readincr(in);
    }
}

void write_hs(output_stream_uint32 * out, uint32_t_arg_out& value) {
    writeincr(out, (uint32_t)(value.addr & 0xFFFFFFFF));
    writeincr(out, (uint32_t)(value.addr >> 32));
    writeincr(out, value.data);
    writeincr(out, value.size);
    writeincr(out, value.allow);
    // padding
    for (int i = 0; i < 3; i++) {
        writeincr(out, 0);
    }
}

void read_hs(input_stream_uint32 * in, fib_task& ret) {
    uint32_t cont_lo = readincr(in);
    uint32_t cont_hi = readincr(in);
    ret._cont = ((uint64_t)cont_hi << 32) | (uint64_t)cont_lo;
    ret.n = readincr(in);
    // padding
    for (int i = 0; i < 5; i++) {
        readincr(in);
    }
}

void write_hs(output_stream_uint32 * out, fib_task& value) {
    writeincr(out, (uint32_t)(value._cont & 0xFFFFFFFF));
    writeincr(out, (uint32_t)(value._cont >> 32));
    writeincr(out, value.n);
    // padding
    for (int i = 0; i < 5; i++) {
        writeincr(out, 0);
    }
}

template<typename T1, typename T2>
struct stream_union_2 {
    union {
        T1 t1;
        T2 t2;
    };
    uint32 _padding[3];
    uint32 tag;
};

template<typename T1, typename T2>
void read_hs(input_stream_uint32 * in, stream_union_2<T1, T2>& ret) {
    uint32 readBytes = 16;
    if (ret.tag == 0) {
        read_hs(in, ret.t1);
        readBytes += sizeof(T1);
    } else {
        read_hs(in, ret.t2);
        readBytes += sizeof(T2);
    }
    while (sizeof(ret) > readBytes) {
        readincr(in);
        readBytes += 4;
    }
    for (int i = 0; i < 3; i++) {
        ret._padding[i] = readincr(in);
    }
    ret.tag = readincr(in);
}

template<typename T1, typename T2>
void write_hs(output_stream_uint32 * out, stream_union_2<T1, T2>& value) {
    uint32 writtenBytes = 16;
    if (value.tag == 0) {
        write_hs(out, value.t1);
        writtenBytes += sizeof(T1);
    } else {
        write_hs(out, value.t2);
        writtenBytes += sizeof(T2);
    }
    while (sizeof(value) > writtenBytes) {
        writeincr(out, writtenBytes);
        writtenBytes += 4;
    }
    for (int i = 0; i < 3; i++) {
        writeincr(out, i+1);
    }
    writeincr(out, value.tag);
}
#endif