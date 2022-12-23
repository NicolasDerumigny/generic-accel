// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// control
// 0x000 : Control signals
//         bit 0  - ap_start (Read/Write/COH)
//         bit 1  - ap_done (Read/COR)
//         bit 2  - ap_idle (Read)
//         bit 3  - ap_ready (Read/COR)
//         bit 7  - auto_restart (Read/Write)
//         bit 9  - interrupt (Read)
//         others - reserved
// 0x004 : Global Interrupt Enable Register
//         bit 0  - Global Interrupt Enable (Read/Write)
//         others - reserved
// 0x008 : IP Interrupt Enable Register (Read/Write)
//         bit 0 - enable ap_done interrupt (Read/Write)
//         bit 1 - enable ap_ready interrupt (Read/Write)
//         others - reserved
// 0x00c : IP Interrupt Status Register (Read/TOW)
//         bit 0 - ap_done (Read/TOW)
//         bit 1 - ap_ready (Read/TOW)
//         others - reserved
// 0x010 : Data signal of data_in
//         bit 31~0 - data_in[31:0] (Read/Write)
// 0x014 : Data signal of data_in
//         bit 31~0 - data_in[63:32] (Read/Write)
// 0x018 : reserved
// 0x01c : Data signal of data_out
//         bit 31~0 - data_out[31:0] (Read/Write)
// 0x020 : Data signal of data_out
//         bit 31~0 - data_out[63:32] (Read/Write)
// 0x024 : reserved
// 0x028 : Data signal of start_time
//         bit 31~0 - start_time[31:0] (Read)
// 0x02c : Data signal of start_time
//         bit 31~0 - start_time[63:32] (Read)
// 0x030 : reserved
// 0x040 : Data signal of end_time
//         bit 31~0 - end_time[31:0] (Read)
// 0x044 : Data signal of end_time
//         bit 31~0 - end_time[63:32] (Read)
// 0x048 : reserved
// 0x200 ~
// 0x3ff : Memory 'pgm' (512 * 8b)
//         Word n : bit [ 7: 0] - pgm[4n]
//                  bit [15: 8] - pgm[4n+1]
//                  bit [23:16] - pgm[4n+2]
//                  bit [31:24] - pgm[4n+3]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL         0x000
#define XGENERIC_ACCEL_CONTROL_ADDR_GIE             0x004
#define XGENERIC_ACCEL_CONTROL_ADDR_IER             0x008
#define XGENERIC_ACCEL_CONTROL_ADDR_ISR             0x00c
#define XGENERIC_ACCEL_CONTROL_ADDR_DATA_IN_DATA    0x010
#define XGENERIC_ACCEL_CONTROL_BITS_DATA_IN_DATA    64
#define XGENERIC_ACCEL_CONTROL_ADDR_DATA_OUT_DATA   0x01c
#define XGENERIC_ACCEL_CONTROL_BITS_DATA_OUT_DATA   64
#define XGENERIC_ACCEL_CONTROL_ADDR_START_TIME_DATA 0x028
#define XGENERIC_ACCEL_CONTROL_BITS_START_TIME_DATA 64
#define XGENERIC_ACCEL_CONTROL_ADDR_END_TIME_DATA   0x040
#define XGENERIC_ACCEL_CONTROL_BITS_END_TIME_DATA   64
#define XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE        0x200
#define XGENERIC_ACCEL_CONTROL_ADDR_PGM_HIGH        0x3ff
#define XGENERIC_ACCEL_CONTROL_WIDTH_PGM            8
#define XGENERIC_ACCEL_CONTROL_DEPTH_PGM            512

