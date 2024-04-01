///////////////////////////////////////////////////////////////////////////////
// File:	ps2pad.h
// SDK:		GameSpy Common EE code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef __PS2PAD_H__
#define __PS2PAD_H__

#include "../../../nonport.h"

typedef enum
{
	PadLeft,
	PadDown,
	PadRight,
	PadUp,
	PadStart,
	PadRightStick,
	PadLeftStick,
	PadSelect,
	PadSquare,
	PadX,
	PadCircle,
	PadTriangle,
	PadR1,
	PadL1,
	PadR2,
	PadL2,
	NumPadEvents
} PadEvents;

int PadInit(void);
void PadReadInput(int events[NumPadEvents]);

#endif
