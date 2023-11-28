///////////////////////////////////////////////////////////////////////////////
// File:	gvCustomDevice.h
// SDK:		GameSpy Voice 2 SDK
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2004-2009 GameSpy Industries, Inc.

#ifndef _GV_CUSTOM_DEVICE_H_
#define _GV_CUSTOM_DEVICE_H_

#include "gvMain.h"
#include "gvDevice.h"

GVDevice gviCustomNewDevice(GVDeviceType type);

GVBool gviGetCustomPlaybackAudio(GVIDevice * device, GVSample * audio, int numSamples);

GVBool gviSetCustomCaptureAudio(GVDevice device, const GVSample * audio, int numSamples,
                                GVByte * packet, int * packetLen, GVFrameStamp * frameStamp, GVScalar * volume);

#endif
