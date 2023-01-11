############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
############################################################
open_project generic-accel
set_top generic_accel
add_files generic-accel/src/rw.hpp
add_files generic-accel/src/rw.cpp
add_files generic-accel/src/lbg.hpp
add_files generic-accel/src/lbg.cpp
add_files generic-accel/src/fu.hpp
add_files generic-accel/src/fu.cpp
add_files generic-accel/src/dma.hpp
add_files generic-accel/src/dma.cpp
add_files generic-accel/src/core.hpp
add_files generic-accel/src/core.cpp
add_files generic-accel/src/config.hpp
add_files generic-accel/src/agu.hpp
add_files generic-accel/src/agu.cpp
add_files -tb generic-accel/test/main.cpp -cflags "-Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas -Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "zcu104" -flow_target vivado
set_part {xczu7ev-ffvc1156-2-e}
create_clock -period 10 -name default
config_export -flow impl -format ip_catalog -rtl verilog -vivado_clock 10 -vivado_phys_opt all
source "./generic-accel/zcu104/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -rtl verilog -format ip_catalog
