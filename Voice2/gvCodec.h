///////////////////////////////////////////////////////////////////////////////
// File:	gvCodec.h
// SDK:		GameSpy Voice 2 SDK
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2004-2009 GameSpy Industries, Inc.

#ifndef _GV_CODEC_H_
#define _GV_CODEC_H_

#include "gvMain.h"

/************
** GLOBALS **
************/
extern int GVISamplesPerFrame;
extern int GVIBytesPerFrame;
extern int GVIEncodedFrameSize;
extern GVRate GVISampleRate;			//In samples per second.
extern int GVIBytesPerSecond;

/**************
** FUNCTIONS **
**************/
void gviCodecsInitialize(void);
void gviCodecsCleanup(void);

GVBool gviSetCodec(GVCodec codec);
void gviSetCustomCodec(GVCustomCodecInfo * info);

void gviSetSampleRate(GVRate sampleRate);
GVRate gviGetSampleRate(void);

GVBool gviNewDecoder(GVDecoderData * data);
void gviFreeDecoder(GVDecoderData data);

void gviEncode(GVByte * out, const GVSample * in);
void gviDecodeAdd(GVSample * out, const GVByte * in, GVDecoderData data);
void gviDecodeSet(GVSample * out, const GVByte * in, GVDecoderData data);

void gviResetEncoder(void);

#endif
