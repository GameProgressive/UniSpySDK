///////////////////////////////////////////////////////////////////////////////
// File:	gvFrame.h
// SDK:		GameSpy Voice 2 SDK
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2004-2009 GameSpy Industries, Inc.

#ifndef _GV_FRAME_H_
#define _GV_FRAME_H_

#include "gvMain.h"

// max value for a framestamp
#define GVI_FRAMESTAMP_MAX   0xFFFF

#if defined(_MACOSX)
	#define GVI_PRE_DECODE 1
#else
	#define GVI_PRE_DECODE 0
#endif

// when allocated, enough memory is allocated to fit an entire
// frame into the m_frame array
typedef struct GVIPendingFrame
{
	GVFrameStamp m_frameStamp;
	struct GVIPendingFrame * m_next;
	// m_frame must be the last member of this struct
#if GVI_PRE_DECODE
	GVSample m_frame[1];
#else
	GVByte m_frame[1];
#endif
} GVIPendingFrame;

void gviFramesStartup(void);
void gviFramesCleanup(void);

GVIPendingFrame * gviGetPendingFrame(void);
void gviPutPendingFrame(GVIPendingFrame * frame);

// a > b
GVBool gviIsFrameStampGT(GVFrameStamp a, GVFrameStamp b);
// a >= b
GVBool gviIsFrameStampGTE(GVFrameStamp a, GVFrameStamp b);

#endif
