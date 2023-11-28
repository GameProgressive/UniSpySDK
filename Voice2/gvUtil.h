///////////////////////////////////////////////////////////////////////////////
// File:	gvUtil.c
// SDK:		GameSpy Voice 2 SDK
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2004-2009 GameSpy Industries, Inc.

#ifndef _GV_UTIL_H_
#define _GV_UTIL_H_

#include "gvMain.h"
#include "gvCodec.h"

// gets the volume for a set of samples
GVScalar gviGetSamplesVolume(const GVSample * samplesPtr, int numSamples);

// checks if any samples in the set are above the given threshold
GVBool gviIsOverThreshold(const GVSample * samplesPtr, int numSamples, GVScalar threshold);

// returns the lowest multiple of base that is >= value
int gviRoundUpToNearestMultiple(int value, int base);
// returns the highest multiple of base that is <= value
int gviRoundDownToNearestMultiple(int value, int base);
// rounds the multiple of base that is closest to value
int gviRoundToNearestMultiple(int value, int base);

// multiply or divide by bytes per millisecond
int gviMultiplyByBytesPerMillisecond(int value);
int gviDivideByBytesPerMillisecond(int value);

#endif
