/*
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: X11
*/

#include <iostream>
#include <cstring>
#include <regOffsets.h>
#include <vector>
#include <cstdint>
#include <unistd.h>

#include <chrono>


// XRT includes
#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include <experimental/xrt_xclbin.h>
#include <experimental/xrt_ip.h>

#define buf_size 4096 

// Define the struct for the fib arguments
struct fib_task {
  uint64_t cont;
  uint32_t n;
  uint8_t _padding[20];
};

// Define the struct for the sum arguments
struct sum_task {
  uint32_t _counter;
  uint32_t f1;
  uint64_t cont;
  uint32_t f2;
  uint8_t _padding[12];
};

uint64_t inline readRegister64(xrt::ip & ip, uint64_t offset) {
    //return  ip.read_register(offset);
    uint64_t high = ip.read_register(offset + 4);
    uint64_t low  = ip.read_register(offset);
    return (high << 32) | low;
}
void inline writeRegister64(xrt::ip & ip, uint64_t offset, uint64_t value) {
    ip.write_register(offset, static_cast<uint32_t>(value & 0xFFFFFFFF));
    ip.write_register(offset + 4, static_cast<uint32_t>((value >> 32) & 0xFFFFFFFF));
}






template <typename T>
void inline initializeScheduler( std::vector<T> base_task_data, 
                          uint64_t bufferSize,
                          uint64_t rAddr_shift,
                          uint64_t maxLength_shift,
                          uint64_t fifoTailReg_shift,
                          uint64_t fifoHeadReg_shift,
                          uint64_t currLen_shift,
                          xrt::ip & ip1,
                          xrt::device & device,
                          int memoryBank,
                          xrt::bo* preallocated_bo = nullptr,
                          // pre allocated map
                          T** preallocated_map = nullptr
                        ){

    // Create a host bufferSize as a bo object
    size_t vector_size_bytes = bufferSize * sizeof(T);
    auto scheduler_virtual_queue = *preallocated_bo;
    auto scheduler_virtual_queue_map = *preallocated_map;
    
    // fill with zeros but use explicit pointer conversion to avoid issues with std::fill and non-trivial types
    std::fill(reinterpret_cast<uint8_t*>(scheduler_virtual_queue_map), reinterpret_cast<uint8_t*>(scheduler_virtual_queue_map) + vector_size_bytes, 0);
    
    // if base_task_data is not empty, copy the base_task_data to the head of the scheduler_virtual_queue_map
    if(!base_task_data.empty()){
        std::copy(base_task_data.begin(), base_task_data.end(), scheduler_virtual_queue_map);
    }


    // Synchronize buffer content with device side
    scheduler_virtual_queue.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // read back the content to verify
    scheduler_virtual_queue.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    auto verify_map = scheduler_virtual_queue.map<T*>();
    for(size_t i = 0; i < base_task_data.size(); i++){
        if(std::memcmp(&verify_map[i], &base_task_data[i], sizeof(T)) != 0){
            std::cerr << "Error: base_task_data mismatch at index " << i << std::endl;
        }


    }

    // if base_task_data is not empty, log the first base task read back
    if(!base_task_data.empty()){
        std::cout << "First base task read back: ";
        for(size_t i = 0; i < sizeof(T); i++){
            std::cout << std::hex << static_cast<uint32_t>(reinterpret_cast<uint8_t*>(&verify_map[0])[i]) << " ";
        }
        std::cout << std::dec << std::endl;
    }

    // get the physical address of the bo object
    uint64_t addr = scheduler_virtual_queue.address();
    std::cout << "Scheduler virtual queue address: " << std::hex << addr << std::dec << std::endl;

    // initialize the scheduler server
    writeRegister64(ip1, rAddr_shift, addr);
    writeRegister64(ip1, maxLength_shift, bufferSize);
    writeRegister64(ip1, fifoTailReg_shift, base_task_data.size());
    writeRegister64(ip1, fifoHeadReg_shift, 0x0);
    writeRegister64(ip1, currLen_shift, base_task_data.size());


    // Read back the values to verify
    uint64_t val = 0;
    val = readRegister64(ip1, rAddr_shift);
    if(val != addr){
        std::cerr << "Error: rAddr register value mismatch. Expected: " << std::hex << addr << ", Got: " << val << std::dec << std::endl;
    }
    val = readRegister64(ip1, maxLength_shift);
    if(val != bufferSize){
        std::cerr << "Error: maxLength register value mismatch. Expected: " << std::hex << bufferSize << ", Got: " << val << std::dec << std::endl;
    }
    val = readRegister64(ip1, fifoTailReg_shift);
    if(val != base_task_data.size()){
        std::cerr << "Error: fifoTailReg register value mismatch. Expected: " << std::hex << base_task_data.size() << ", Got: " << val << std::dec << std::endl;
    }
    val = readRegister64(ip1, fifoHeadReg_shift);
    if(val != 0x0){
        std::cerr << "Error: fifoHeadReg register value mismatch. Expected: " << std::hex << 0x0 << ", Got: " << val << std::dec << std::endl;
    }
    val = readRegister64(ip1, currLen_shift);
    if(val != base_task_data.size()){
        std::cerr << "Error: currLen register value mismatch. Expected: " << std::hex << base_task_data.size() << ", Got: " << val << std::dec << std::endl;
    }
}

void inline logRpause(xrt::ip & ip, uint64_t offset, const std::string& name) {
    uint64_t value = 0;
    value = ip.read_register(offset);
    value = value | (static_cast<uint64_t>(ip.read_register(offset + 4)) << 32);
    std::cout << "rPause " << name << ": " << std::hex << value << std::dec << std::endl;
}

void inline logAllRpause(xrt::ip & ip) {
    logRpause(ip, REG_SUM_SCHEDULER_0_RPAUSE, "scheduler fib 0");
    logRpause(ip, REG_SUM_SCHEDULER_1_RPAUSE, "scheduler fib 1");
    logRpause(ip, REG_SUM_ALLOCATOR_0_RPAUSE, "allocator 0");
    logRpause(ip, REG_FIB_SCHEDULER_0_RPAUSE, "sum scheduler 0");
}

template <typename T>
void inline initializeAllocator(uint64_t bufferSize,
                          uint64_t rAddr_shift,
                          uint64_t availableSize_shift,
                          xrt::ip & ip1,
                          xrt::device & device,
                          int memoryBank_0,
                          int memoryBank_1,
                          xrt::bo* preallocated_bo = nullptr,
                          // pre allocated map
                          T** preallocated_map = nullptr,
                          xrt::bo* preallocated_address_bo = nullptr,
                          uint64_t** preallocated_address_map = nullptr
                        ){
    
    // Create a host bufferSize as a bo object for continuation virtual queue
    size_t vector_size_bytes = bufferSize * sizeof(T);
    auto allocator_virtual_queue = *preallocated_bo;
    auto allocator_virtual_queue_map = *preallocated_map;

    // fill with zeros but use explicit pointer conversion to avoid issues with std::fill and non-trivial types
    std::fill(reinterpret_cast<uint8_t*>(allocator_virtual_queue_map), reinterpret_cast<uint8_t*>(allocator_virtual_queue_map) + vector_size_bytes, 0);

    // Synchronize buffer content with device side
    allocator_virtual_queue.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // Create a vector of uint64_t addresses that point to the continuation tasks
    std::vector<uint64_t> addresses;
    u_int64_t addr = allocator_virtual_queue.address();

    // log the base address of the continuation tasks
    std::cout << "Allocator virtual queue address: " << std::hex << addr << std::dec << std::endl;

    for(uint64_t i = 0; i < bufferSize; i++){
        addresses.push_back(addr + i * sizeof(T));
    }

    // Create a host bufferSize as a bo object for continuation addresses
    size_t address_vector_size_bytes = bufferSize * sizeof(uint64_t);
    auto allocator_address_queue = *preallocated_address_bo;
    
    auto allocator_address_queue_map = *preallocated_address_map;
    std::copy(addresses.begin(), addresses.end(), allocator_address_queue_map);
    
    // Synchronize buffer content with device side
    allocator_address_queue.sync(XCL_BO_SYNC_BO_TO_DEVICE);


    // get the physical address of the bo object
    uint64_t addr_queue = allocator_address_queue.address();
    std::cout << "Allocator address queue address: " << std::hex << addr_queue << std::dec << std::endl;

    // initialize the allocator server
    writeRegister64(ip1, rAddr_shift, addr_queue);
    writeRegister64(ip1, availableSize_shift, addresses.size());


    // Read back the values to verify
    uint64_t val = 0;
    val = readRegister64(ip1, rAddr_shift);
    if(val != addr_queue){
        std::cerr << "Error: rAddr register value mismatch. Expected: " << std::hex << addr_queue << ", Got: " << val << std::dec << std::endl;
    }
    val = readRegister64(ip1, availableSize_shift);
    if(val != addresses.size()){
        std::cerr << "Error: availableSize register value mismatch. Expected: " << std::hex << addresses.size() << ", Got: " << val << std::dec << std::endl;
    }
}


int main(int argc, char** argv) {

    // Check command line arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <xclbin file> <fib n>" << std::endl;
        return -1;
    }
    std::string binaryFile = argv[1];
    uint32_t fib_value = std::stoi(argv[2]);
    
    auto xclbin = xrt::xclbin(binaryFile);
    int device_index = 0;

    std::cout << "Open the device " << device_index << std::endl;
    auto device = xrt::device(device_index);
    std::cout << "Load the xclbin " << binaryFile << std::endl;
    auto uuid = device.load_xclbin(binaryFile);
 

    auto ip1 = xrt::ip(device, uuid, "fibonacci:{fibonacci_1}");


    std::cout << "Log the rPause registers before starting the servers" << std::endl;

   
    // Log the rPause registers before starting the servers
    logAllRpause(ip1);

    // A loop to read all data at the register file from address 0x0 to 0x100 logging the address and the data as hex 64-bit values
    // std::cout << "Reading all registers from 0x0 to 0x100" << std::endl;
    // for(uint64_t addr = 0x0; addr <= 0x100; addr += 0x4){
    //     uint64_t data = ip1.read_register(addr);
    //     std::cout << "Address: " << std::hex << addr << " Data: " << data << std::dec << std::endl;
    // }

    
    sum_task sum_task_0 = {4, 0, 0, 0, {0}};   // Create a continuation task of sum_task
    auto sum_task_bo = xrt::bo(device, sizeof(sum_task_0), 0); // Move it to a buffer object to move to the FPGA
    auto sum_task_map = sum_task_bo.map<sum_task*>();
    std::copy(&sum_task_0, &sum_task_0 + 1, sum_task_map);
    sum_task_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    
    uint64_t sum_task_addr = sum_task_bo.address();     // Get the physical address of the sum_task_bo
    std::cout << "sum_task address: " << std::hex << sum_task_addr << std::dec << std::endl;


    // Read back the sum_task structure to verify it was written correctly
    sum_task sum_task_readback = {0, 0, 0, 0, 0};
    sum_task_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    std::cout << "Synchronized sum task bo from device" << std::endl;
    std::copy(sum_task_map, sum_task_map + 1, &sum_task_readback);
    std::cout << "Read Sum task back" << std::endl;
    if(std::memcmp(&sum_task_0, &sum_task_readback, sizeof(sum_task_0)) != 0){
        std::cerr << "Error: sum_task structure mismatch after write and readback" << std::endl;
        return -1;
    }


    fib_task args = {sum_task_addr+0x4, fib_value, {0}};     // Create the fib base task using fib args
    std::vector<fib_task> base_task_data = {args};




    std::cout << "Initializing the scheduler and allocator servers" << std::endl;
    // Initialize the scheduler server 0 for fib with empty base task data
    auto fib_task_bo = xrt::bo(device, buf_size * sizeof(fib_task), 0);

    std::cout << "Allocation successful for fib_task_bo" << std::endl;
    auto fib_task_map = fib_task_bo.map<fib_task*>();
    initializeScheduler<fib_task>( base_task_data,
                                  buf_size,
                                  REG_FIB_SCHEDULER_0_RADDR,
                                  REG_FIB_SCHEDULER_0_MAXLEN,
                                  REG_FIB_SCHEDULER_0_FIFOTAILREG,
                                  REG_FIB_SCHEDULER_0_FIFOHEADREG,
                                  REG_FIB_SCHEDULER_0_CURRLEN,
                                  ip1,
                                  device,
                                  2,
                                  &fib_task_bo,
                                  &fib_task_map);


    // Initialize the scheduler server 0 for sum
    auto sum_scheduler_0_bo = xrt::bo(device, buf_size * sizeof(sum_task), 0);
    auto sum_scheduler_0_map = sum_scheduler_0_bo.map<sum_task*>();
    initializeScheduler<sum_task>({}, 
                                  buf_size,
                                  REG_SUM_SCHEDULER_0_RADDR,
                                  REG_SUM_SCHEDULER_0_MAXLEN,
                                  REG_SUM_SCHEDULER_0_FIFOTAILREG,
                                  REG_SUM_SCHEDULER_0_FIFOHEADREG,
                                  REG_SUM_SCHEDULER_0_CURRLEN,
                                  ip1,
                                  device,
                                  1,
                                  &sum_scheduler_0_bo,
                                  &sum_scheduler_0_map);
    
    // Initialize the scheduler server 1 for sum with empty base task data
    auto sum_scheduler_1_bo = xrt::bo(device, buf_size * sizeof(sum_task), 0);
    auto sum_scheduler_1_map = sum_scheduler_1_bo.map<sum_task*>();
    initializeScheduler<sum_task>({}, 
                                  buf_size,
                                  REG_SUM_SCHEDULER_1_RADDR,
                                  REG_SUM_SCHEDULER_1_MAXLEN,
                                  REG_SUM_SCHEDULER_1_FIFOTAILREG,
                                  REG_SUM_SCHEDULER_1_FIFOHEADREG,
                                  REG_SUM_SCHEDULER_1_CURRLEN,
                                  ip1,
                                  device,
                                  6,
                                  &sum_scheduler_1_bo,
                                  &sum_scheduler_1_map);

    // Initialize the allocator server 0 for sum

    size_t vector_size_bytes = buf_size  * sizeof(sum_task);
    auto allocator_virtual_queue = xrt::bo(device, vector_size_bytes, 0);
    auto allocator_virtual_queue_map = allocator_virtual_queue.map<sum_task*>();
    auto allocator_address_queue = xrt::bo(device, buf_size * sizeof(uint64_t), 0);
    auto allocator_address_queue_map = allocator_address_queue.map<uint64_t*>();

    initializeAllocator<sum_task>(  buf_size,
                                    REG_SUM_ALLOCATOR_0_RADDR,
                                    REG_SUM_ALLOCATOR_AVAILABLESIZE,
                                    ip1,
                                    device,
                                    3,
                                    4,
                                    &allocator_virtual_queue,
                                    &allocator_virtual_queue_map,
                                    &allocator_address_queue,
                                    &allocator_address_queue_map);



    

    // read the addresses between 0x10 and 0x100 with 32 bit steps and log them
    // for(uint64_t addr = 0x10; addr <= 0x100; addr += 0x8){
    //     uint64_t data = readRegister64(ip1, addr);
    //     std::cout << "Address: " << std::hex << addr << " Data: " << data << std::dec << std::endl;
    // }

    // log start time
    auto start = std::chrono::high_resolution_clock::now();
    
    
    // Write 0 to all rPause registers to start the servers
    writeRegister64(ip1, REG_SUM_SCHEDULER_0_RPAUSE, 0x0);
    writeRegister64(ip1, REG_SUM_SCHEDULER_1_RPAUSE, 0x0);
    writeRegister64(ip1, REG_SUM_ALLOCATOR_0_RPAUSE, 0x0);
    writeRegister64(ip1, REG_FIB_SCHEDULER_0_RPAUSE, 0x0);


    // read the addresses between 0x10 and 0x100 with 32 bit steps and log them
    // for(uint64_t addr = 0x10; addr <= 0x100; addr += 0x8){
    //     uint64_t data = readRegister64(ip1, addr);
    //     std::cout << "Address: " << std::hex << addr << " Data: " << data << std::dec << std::endl;
    // }



    // Log the rPause registers after starting the servers
    logAllRpause(ip1);


    // In a loop readback the sum_task structure from the FPGA and log the _counter, f1, f2 values until f1 is not zero and print f1
    
    sum_task result = {0, 0, 0, 0, 0, 0};

    sum_task_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    std::copy(sum_task_map, sum_task_map + 1, &result);
    std::cout << "sum_task _counter: " << result._counter << " f1: " << result.f1 << " f2: " << result.f2 << std::endl;

    // log the current length of the fib scheduler
    uint64_t currLen = readRegister64(ip1, REG_FIB_SCHEDULER_0_CURRLEN);
    std::cout << "Fib scheduler current length: " << currLen << std::endl;


    do {
        sum_task_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        std::copy(sum_task_map, sum_task_map + 1, &result);
        std::cout << "sum_task _counter: " << result._counter << " f1: " << result.f1 << " f2: " << result.f2 << std::endl;

        // log the current length of the fib scheduler
        uint64_t currLen = readRegister64(ip1, REG_FIB_SCHEDULER_0_CURRLEN);
        std::cout << "Fib scheduler current length: " << currLen << std::endl;

        sleep(1);
    } while(result.f1 == 0);

    // log end time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;

    std::cout << "Final Result: Fibonacci(" << fib_value << ") = " << result.f1 << std::endl;


    // std::cout << "Allocate Buffer in Global Memory\n";

    // // print vector size in MB
    // std::cout << "Vector size in bytes: " << vector_size_bytes << " bytes\n";

    // auto ip1_boA = xrt::bo(device, vector_size_bytes, 1);
    // auto ip1_boB = xrt::bo(device, vector_size_bytes, 1);

    // // Map the contents of the buffer object into host memory
    // auto bo0_map = ip1_boA.map<int*>();
    // auto bo1_map = ip1_boB.map<int*>();
 
    // std::fill(bo0_map, bo0_map + DATA_SIZE, 0);
    // std::fill(bo1_map, bo1_map + DATA_SIZE, 0);

    // // Create the test data
    // int bufReference[DATA_SIZE];
    // for (int i = 0; i < DATA_SIZE; ++i) {
    //     bo0_map[i] = i;
    //     bo1_map[i] = i;
    //     bufReference[i] = bo0_map[i] + bo1_map[i]; //Generate check data for validation
    // }

    // std::cout << "loaded the data" << std::endl;
    // uint64_t buf_addr[2];
    // // Get the buffer physical address
    // buf_addr[0] = ip1_boA.address();
    // buf_addr[1] = ip1_boB.address();

    // // Synchronize buffer content with device side
    // std::cout << "synchronize input buffer data to device global memory\n";
    // ip1_boA.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    // ip1_boB.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // std::cout << "INFO: Setting IP Data" << std::endl;
    // std::cout << "Setting Register \"A\" (Input Address)" << std::endl;
    // ip1.write_register(A_OFFSET, buf_addr[0]);
    // ip1.write_register(A_OFFSET + 4, buf_addr[0] >> 32);

    // std::cout << "Setting Register \"B\" (Input Address)" << std::endl;
    // ip1.write_register(B_OFFSET, buf_addr[1]);
    // ip1.write_register(B_OFFSET + 4, buf_addr[1] >> 32);

    // uint32_t axi_ctrl = IP_START;
    // std::cout << "INFO: IP Start" << std::endl;
    // //axi_ctrl = IP_START;
    // ip1.write_register(USER_OFFSET, axi_ctrl);

    // uint32_t krnl_done, krnl_idle;
    // // Wait until the IP is DONE
    // int i = 0;
    // while (krnl_done != IP_DONE) {
    //     axi_ctrl = ip1.read_register(USER_OFFSET);
    //     krnl_done = axi_ctrl & 0xfffffff2;
    //     krnl_idle = axi_ctrl & 0xfffffff4;
    //     i = i + 1;
    //     std::cout << "Read Loop iteration: " << i << " Kernel Done: " << krnl_done << " Kernel Idle: " << krnl_idle << "\n";
    // }

    // std::cout << "INFO: IP Done" << std::endl;

    // // Get the output;
    // std::cout << "Get the output data from the device" << std::endl;
    // ip1_boB.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    // // Validate results
    // if (std::memcmp(bo1_map, bufReference, DATA_SIZE))
    //     throw std::runtime_error("Value read back does not match reference");

    // std::cout << "TEST PASSED\n";
    return 0;
}
