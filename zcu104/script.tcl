############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
############################################################
open_project accel-hlac
set_top generic_accel
add_files accel-hlac/src/core.cpp
add_files accel-hlac/src/core.hpp
add_files accel-hlac/src/dma.hpp
add_files -tb accel-hlac/test/main.cpp
open_solution "zcu104" -flow_target vivado
set_part {xczu7ev-ffvc1156-2-e}
create_clock -period 10 -name default
config_export -flow impl -format ip_catalog -rtl verilog -vivado_clock 10 -vivado_phys_opt all
source "./accel-hlac/zcu104/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -rtl verilog -format ip_catalog
