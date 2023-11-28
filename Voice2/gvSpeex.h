///////////////////////////////////////////////////////////////////////////////
// File:	gvSpeex.h
// SDK:		GameSpy Voice 2 SDK
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2004-2009 GameSpy Industries, Inc.

#ifndef _GV_SPEEX_H_
#define _GV_SPEEX_H_

#include "gvMain.h"

/*
    8000kHz
quality: samplesPerFrame encodedFrameSize bitsPerSecond
0:             160               6             2150
1:             160              10             3950
2:             160              15             5950
3:             160              20             8000
4:             160              20             8000
5:             160              28            11000
6:             160              28            11000
7:             160              38            15000
8:             160              38            15000
9:             160              46            18200
10:            160              62            24600

    16000kHz
quality: samplesPerFrame encodedFrameSize bitsPerSecond
0:             320              10             3950
1:             320              15             5750
2:             320              20             7750
3:             320              25             9800
4:             320              32            12800
5:             320              42            16800
6:             320              52            20600
7:             320              60            23800
8:             320              70            27800
9:             320              86            34200
10:            320             106            42200
*/

GVBool gviSpeexInitialize(int quality, GVRate sampleRate);
void gviSpeexCleanup(void);

int gviSpeexGetSamplesPerFrame(void);
int gviSpeexGetEncodedFrameSize(void);

GVBool gviSpeexNewDecoder(GVDecoderData * data);
void gviSpeexFreeDecoder(GVDecoderData data);

void gviSpeexEncode(GVByte * out, const GVSample * in);
void gviSpeexDecodeAdd(GVSample * out, const GVByte * in, GVDecoderData data);
void gviSpeexDecodeSet(GVSample * out, const GVByte * in, GVDecoderData data);

void gviSpeexResetEncoder(void);

#endif
