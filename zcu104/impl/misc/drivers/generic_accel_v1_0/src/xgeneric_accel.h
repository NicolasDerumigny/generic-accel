// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XGENERIC_ACCEL_H
#define XGENERIC_ACCEL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xgeneric_accel_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u64 Control_BaseAddress;
} XGeneric_accel_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XGeneric_accel;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XGeneric_accel_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XGeneric_accel_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XGeneric_accel_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XGeneric_accel_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XGeneric_accel_Initialize(XGeneric_accel *InstancePtr, u16 DeviceId);
XGeneric_accel_Config* XGeneric_accel_LookupConfig(u16 DeviceId);
int XGeneric_accel_CfgInitialize(XGeneric_accel *InstancePtr, XGeneric_accel_Config *ConfigPtr);
#else
int XGeneric_accel_Initialize(XGeneric_accel *InstancePtr, const char* InstanceName);
int XGeneric_accel_Release(XGeneric_accel *InstancePtr);
#endif

void XGeneric_accel_Start(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_IsDone(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_IsIdle(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_IsReady(XGeneric_accel *InstancePtr);
void XGeneric_accel_EnableAutoRestart(XGeneric_accel *InstancePtr);
void XGeneric_accel_DisableAutoRestart(XGeneric_accel *InstancePtr);

void XGeneric_accel_Set_data_in(XGeneric_accel *InstancePtr, u64 Data);
u64 XGeneric_accel_Get_data_in(XGeneric_accel *InstancePtr);
void XGeneric_accel_Set_data_out(XGeneric_accel *InstancePtr, u64 Data);
u64 XGeneric_accel_Get_data_out(XGeneric_accel *InstancePtr);
u64 XGeneric_accel_Get_start_time(XGeneric_accel *InstancePtr);
u64 XGeneric_accel_Get_end_time(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_Get_pgm_BaseAddress(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_Get_pgm_HighAddress(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_Get_pgm_TotalBytes(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_Get_pgm_BitWidth(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_Get_pgm_Depth(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_Write_pgm_Words(XGeneric_accel *InstancePtr, int offset, word_type *data, int length);
u32 XGeneric_accel_Read_pgm_Words(XGeneric_accel *InstancePtr, int offset, word_type *data, int length);
u32 XGeneric_accel_Write_pgm_Bytes(XGeneric_accel *InstancePtr, int offset, char *data, int length);
u32 XGeneric_accel_Read_pgm_Bytes(XGeneric_accel *InstancePtr, int offset, char *data, int length);

void XGeneric_accel_InterruptGlobalEnable(XGeneric_accel *InstancePtr);
void XGeneric_accel_InterruptGlobalDisable(XGeneric_accel *InstancePtr);
void XGeneric_accel_InterruptEnable(XGeneric_accel *InstancePtr, u32 Mask);
void XGeneric_accel_InterruptDisable(XGeneric_accel *InstancePtr, u32 Mask);
void XGeneric_accel_InterruptClear(XGeneric_accel *InstancePtr, u32 Mask);
u32 XGeneric_accel_InterruptGetEnabled(XGeneric_accel *InstancePtr);
u32 XGeneric_accel_InterruptGetStatus(XGeneric_accel *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
