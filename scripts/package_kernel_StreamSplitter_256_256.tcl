#/*
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: X11
#*/

set path_to_hdl "./output_rtl/${krnl_name}"
set path_to_packaged "./packaged/${suffix}"
set path_to_tmp_project "./packaged/tmp_${suffix}"

create_project -force kernel_pack $path_to_tmp_project 

# Explicitly list all files
add_files -norecurse ${path_to_hdl}/StreamSplitter_256_256.sv
add_files -norecurse ${path_to_hdl}/chext_mem_1w1r.sv

# Set unique library name
set_property library u0004 [get_files ${path_to_hdl}/StreamSplitter_256_256.sv]
set_property library u0004 [get_files ${path_to_hdl}/chext_mem_1w1r.sv]

update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
ipx::package_project -root_dir $path_to_packaged -vendor epfl.ch -library hardcilk -taxonomy /KernelIP -import_files -set_current false
ipx::unload_core $path_to_packaged/component.xml
ipx::edit_ip_in_project -upgrade true -name tmp_edit_project -directory $path_to_packaged $path_to_packaged/component.xml

set_property core_revision 2 [ipx::current_core]
foreach up [ipx::get_user_parameters] {
  ipx::remove_user_parameter [get_property NAME $up] [ipx::current_core]
}
set_property sdx_kernel true [ipx::current_core]
set_property sdx_kernel_type rtl [ipx::current_core]
ipx::create_xgui_files [ipx::current_core]

# Associate all AXI-Stream interfaces with the clock
foreach bus_if [ipx::get_bus_interfaces -of_objects [ipx::current_core]] {
    set bus_type [get_property BUS_TYPE_VLNV $bus_if]
    if {[string match "*axis*" $bus_type]} {
        set bus_name [get_property NAME $bus_if]
        puts "Associating AXI-Stream interface $bus_name with clock"
        ipx::associate_bus_interfaces -busif $bus_name -clock clock [ipx::current_core]
    }
}

ipx::associate_bus_interfaces -clock clock -reset reset_n [ipx::current_core]

set_property xpm_libraries {XPM_CDC XPM_MEMORY XPM_FIFO} [ipx::current_core]
set_property supported_families { } [ipx::current_core]
set_property auto_family_support_level level_2 [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]
close_project -delete
