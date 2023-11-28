///////////////////////////////////////////////////////////////////////////////
// File:	gvDevice.c
// SDK:		GameSpy Voice 2 SDK
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2004-2009 GameSpy Industries, Inc.

#include "gvDevice.h"

/***********
** DEVICE **
***********/
GVIDevice * gviNewDevice(GVDeviceID deviceID, GVHardwareType hardwareType, GVDeviceType types, int dataSize)
{
	GVIDevice * device;

	device = (GVIDevice *)gsimalloc(sizeof(GVIDevice));
	if(!device)
		return NULL;

	memset(device, 0, sizeof(GVIDevice));
	memcpy(&device->m_deviceID, &deviceID, sizeof(GVDeviceID));
	device->m_hardwareType = hardwareType;
	device->m_types = types;
	device->m_data = gsimalloc((unsigned int)dataSize);
	if(!device->m_data)
	{
		gsifree(device);
		return NULL;
	}
	memset(device->m_data, 0, (unsigned int)dataSize);

	return device;
}

void gviFreeDevice(GVIDevice * device)
{
	gsifree(device->m_data);
	gsifree(device);
}

void gviDeviceUnplugged(GVIDevice * device)
{
	// if they don't have a callback, we can't free the device without them knowing
	if(!device->m_unpluggedCallback)
		return;

	// let them know it was unplugged...
	device->m_unpluggedCallback(device);

	// ...then free the device
	device->m_methods.m_freeDevice(device);
}

/****************
** DEVICE LIST **
****************/
static int gviFindDeviceIndex(GVIDeviceList devices, GVIDevice * device)
{
	int len;
	int i;

	GS_ASSERT(devices);

	len = ArrayLength(devices);
	for(i = 0 ; i < len ; i++)
	{
		if(device == gviGetDevice(devices, i))
			return i;
	}

	return -1;
}

GVIDeviceList gviNewDeviceList(ArrayElementFreeFn elemFreeFn)
{
	return ArrayNew(sizeof(GVIDevice *), 2, elemFreeFn);
}

void gviFreeDeviceList(GVIDeviceList devices)
{
	GS_ASSERT(devices);

	ArrayFree(devices);
}

void gviAppendDeviceToList(GVIDeviceList devices, GVIDevice * device)
{
	GS_ASSERT(devices);

	ArrayAppend(devices, &device);
}

void gviDeleteDeviceFromList(GVIDeviceList devices, GVIDevice * device)
{
	int index;

	GS_ASSERT(devices);

	// find the device
	index = gviFindDeviceIndex(devices, device);
	if(index == -1)
		return;

	// delete it from the array
	ArrayDeleteAt(devices, index);
}

int gviGetNumDevices(GVIDeviceList devices)
{
	GS_ASSERT(devices);

	return ArrayLength(devices);
}

GVIDevice * gviGetDevice(GVIDeviceList devices, int index)
{
	GS_ASSERT(devices);

	return *(GVIDevice **)ArrayNth(devices, index);
}
