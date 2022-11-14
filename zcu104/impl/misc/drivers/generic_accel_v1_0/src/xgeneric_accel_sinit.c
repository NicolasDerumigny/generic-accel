// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xgeneric_accel.h"

extern XGeneric_accel_Config XGeneric_accel_ConfigTable[];

XGeneric_accel_Config *XGeneric_accel_LookupConfig(u16 DeviceId) {
	XGeneric_accel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XGENERIC_ACCEL_NUM_INSTANCES; Index++) {
		if (XGeneric_accel_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XGeneric_accel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XGeneric_accel_Initialize(XGeneric_accel *InstancePtr, u16 DeviceId) {
	XGeneric_accel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XGeneric_accel_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XGeneric_accel_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

