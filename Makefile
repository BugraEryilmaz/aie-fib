
# Copyright (C) 2023 Advanced Micro Devices, Inc
#
# SPDX-License-Identifier: MIT

ECHO=@echo

.PHONY: help

TARGET := hw_emu
# TARGET := hw
PLATFORM := xilinx_vck5000_gen4x8_qdma_2_202220_1
PLATFORM_NAME := $(basename $(notdir $(PLATFORM)))

AIE_OBJ      := ./libadf.a
FINAL_XCLBIN := xclbins/fibonacci_$(TARGET).xclbin
XSA_OBJ      := xclbins/fibonacci_$(TARGET).xsa
EXECUTABLE   := host_overlay.exe

# Discover all RTL kernel directories (./src/rtl/*)
RTL_DIRS := $(wildcard ./src/rtl/*)
# Exclude UpSize_128_512 - not used in connectivity.cfg
RTL_DIRS_FILTERED := $(filter-out ./src/rtl/UpSize_128_512,$(RTL_DIRS))
# Kernel renaming folders i.e. ./src/rtl/fibonacci -> ./output_rtl/fibonacci
RTL_KERNELS := $(patsubst ./src/rtl/%,./output_rtl/%,$(RTL_DIRS_FILTERED))
# Generate XO targets for all kernels
RTL_XOS := $(patsubst %,xos/%-$(TARGET)-$(PLATFORM_NAME).xo,$(notdir $(RTL_DIRS_FILTERED)))

test:
	@$(ECHO) "RTL_DIRS: $(RTL_DIRS)"
	@$(ECHO) "RTL_DIRS_FILTERED: $(RTL_DIRS_FILTERED)"
	@$(ECHO) "RTL_KERNELS: $(RTL_KERNELS)"
	@$(ECHO) "RTL_XOS: $(RTL_XOS)"

###########################################################
# build aie graph
###########################################################
AIE_CXXFLAGS := --target=hw --platform=$(PLATFORM)
AIE_CXXFLAGS += --include="$(XILINX_VITIS)/aietools/include"
$(AIE_OBJ): ./src/aie/*
	@rm -rf Work libadf.a
	@mkdir -p Work
	@$(VPP) -c --mode aie $(AIE_CXXFLAGS) --include="./src/aie" --work_dir=./Work ./src/aie/project.cpp

###########################################################
# clean
###########################################################
.PHONY: clean
clean:
	@rm -rf Work libadf.a xos *.xsa *.xclbin *.o *.so *.exe _x packaged xclbins .Xil *.log host sim *.jou Map_Report.csv sol.db


##########################################################################
# The below commands generate XO files from pre-existing RTL kernels.
# Usage:
#   make xos/fibonacci-hw-xilinx_vck5000_gen4x8_qdma_2_202220_1.xo
#   make xos/DownSize_256_128-hw-xilinx_vck5000_gen4x8_qdma_2_202220_1.xo
#   make all_xos  (builds all discovered RTL kernels)
###########################################################################

# Build all XO files
.PHONY: all_xos
all_xos: $(RTL_XOS)

# Pattern rule for generating XO files from RTL directories
# Extracts kernel name from the target filename
VIVADO := $(XILINX_VIVADO)/bin/vivado
xos/%-$(TARGET)-$(PLATFORM_NAME).xo: ./output_rtl/%
	@mkdir -p xos
	@$(ECHO) "Generating XO file: $@ from $<"
	@$(VIVADO) -mode batch -source scripts/gen_xo.tcl -tclargs $@ $* $(TARGET) $(PLATFORM_NAME)

# List available kernels
.PHONY: list_kernels
list_kernels:
	@$(ECHO) "Available RTL kernels: $(RTL_KERNELS)"
	@$(ECHO) "XO targets: $(RTL_XOS)"

###########################################################################
# xclbin and xsa directory
###########################################################################

$(dir $(FINAL_XCLBIN)):
	mkdir -p $(dir $(FINAL_XCLBIN))

$(dir $(XSA_OBJ)):
	mkdir -p $(dir $(XSA_OBJ))


###########################################################################
# xclbin generation
###########################################################################
VPP := v++ 
VPPFLAGS := --target=$(TARGET) --platform=$(PLATFORM) --save-temps -g --profile.data all:all:all

$(XSA_OBJ): $(AIE_OBJ) $(RTL_XOS) $(dir $(XSA_OBJ))
	@mkdir -p xclbins
	@echo "Linking XSA: $(XSA_OBJ)"
	$(VPP) $(VPPFLAGS) -l -o $@ $(AIE_OBJ) $(RTL_XOS) --config ./connectivity.cfg --vivado.synth.jobs 32 --vivado.impl.jobs 16


###########################################################
# package temp xclbin file
###########################################################
$(FINAL_XCLBIN): $(XSA_OBJ) $(dir $(FINAL_XCLBIN))
	v++ -p -t $(TARGET) -f $(PLATFORM) $(XSA_OBJ) $(AIE_OBJ) -o $@ --package.boot_mode=ospi

host:
	g++ -I./src/host -Wall -O0 -g -std=c++17 src/host/fib_host.cpp -I/opt/xilinx/xrt/include -L/opt/xilinx/xrt/lib -lxrt_coreutil -pthread -o host

###########################################################
# default target
###########################################################
.PHONY: all
all:  $(FINAL_XCLBIN) host

.PHONY: run
run: all
# 	enable_xilinx_2022.2
	if [ "$(TARGET)" = "hw_emu" ]; then \
	  export XCL_EMULATION_MODE=hw_emu; \
	else \
	  unset XCL_EMULATION_MODE; \
	fi; \
	./host $(FINAL_XCLBIN) 5