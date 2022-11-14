// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xgeneric_accel.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XGeneric_accel_CfgInitialize(XGeneric_accel *InstancePtr, XGeneric_accel_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XGeneric_accel_Start(XGeneric_accel *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL) & 0x80;
    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XGeneric_accel_IsDone(XGeneric_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XGeneric_accel_IsIdle(XGeneric_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XGeneric_accel_IsReady(XGeneric_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XGeneric_accel_EnableAutoRestart(XGeneric_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XGeneric_accel_DisableAutoRestart(XGeneric_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_AP_CTRL, 0);
}

void XGeneric_accel_Set_data_in(XGeneric_accel *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_IN_DATA, (u32)(Data));
    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_IN_DATA + 4, (u32)(Data >> 32));
}

u64 XGeneric_accel_Get_data_in(XGeneric_accel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_IN_DATA);
    Data += (u64)XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_IN_DATA + 4) << 32;
    return Data;
}

void XGeneric_accel_Set_data_out(XGeneric_accel *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_OUT_DATA, (u32)(Data));
    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_OUT_DATA + 4, (u32)(Data >> 32));
}

u64 XGeneric_accel_Get_data_out(XGeneric_accel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_OUT_DATA);
    Data += (u64)XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_DATA_OUT_DATA + 4) << 32;
    return Data;
}

u64 XGeneric_accel_Get_start_time(XGeneric_accel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_START_TIME_DATA);
    Data += (u64)XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_START_TIME_DATA + 4) << 32;
    return Data;
}

u64 XGeneric_accel_Get_end_time(XGeneric_accel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_END_TIME_DATA);
    Data += (u64)XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_END_TIME_DATA + 4) << 32;
    return Data;
}

u32 XGeneric_accel_Get_pgm_BaseAddress(XGeneric_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Control_BaseAddress + XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE);
}

u32 XGeneric_accel_Get_pgm_HighAddress(XGeneric_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Control_BaseAddress + XGENERIC_ACCEL_CONTROL_ADDR_PGM_HIGH);
}

u32 XGeneric_accel_Get_pgm_TotalBytes(XGeneric_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XGENERIC_ACCEL_CONTROL_ADDR_PGM_HIGH - XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + 1);
}

u32 XGeneric_accel_Get_pgm_BitWidth(XGeneric_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGENERIC_ACCEL_CONTROL_WIDTH_PGM;
}

u32 XGeneric_accel_Get_pgm_Depth(XGeneric_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGENERIC_ACCEL_CONTROL_DEPTH_PGM;
}

u32 XGeneric_accel_Write_pgm_Words(XGeneric_accel *InstancePtr, int offset, word_type *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XGENERIC_ACCEL_CONTROL_ADDR_PGM_HIGH - XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Control_BaseAddress + XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XGeneric_accel_Read_pgm_Words(XGeneric_accel *InstancePtr, int offset, word_type *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XGENERIC_ACCEL_CONTROL_ADDR_PGM_HIGH - XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Control_BaseAddress + XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + (offset + i)*4);
    }
    return length;
}

u32 XGeneric_accel_Write_pgm_Bytes(XGeneric_accel *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XGENERIC_ACCEL_CONTROL_ADDR_PGM_HIGH - XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Control_BaseAddress + XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XGeneric_accel_Read_pgm_Bytes(XGeneric_accel *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XGENERIC_ACCEL_CONTROL_ADDR_PGM_HIGH - XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Control_BaseAddress + XGENERIC_ACCEL_CONTROL_ADDR_PGM_BASE + offset + i);
    }
    return length;
}

void XGeneric_accel_InterruptGlobalEnable(XGeneric_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_GIE, 1);
}

void XGeneric_accel_InterruptGlobalDisable(XGeneric_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_GIE, 0);
}

void XGeneric_accel_InterruptEnable(XGeneric_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_IER);
    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_IER, Register | Mask);
}

void XGeneric_accel_InterruptDisable(XGeneric_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_IER);
    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_IER, Register & (~Mask));
}

void XGeneric_accel_InterruptClear(XGeneric_accel *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGeneric_accel_WriteReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_ISR, Mask);
}

u32 XGeneric_accel_InterruptGetEnabled(XGeneric_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_IER);
}

u32 XGeneric_accel_InterruptGetStatus(XGeneric_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGeneric_accel_ReadReg(InstancePtr->Control_BaseAddress, XGENERIC_ACCEL_CONTROL_ADDR_ISR);
}

