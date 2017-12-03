///////////////////////////////////////////////////////////////////////////////
// File:	gamespyHttp.cs
// SDK:		GameSpy HTTP SDK C# Wrapper
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading;

namespace Gamespy
{
    namespace Http
    {
        class gamespyHttp
        {
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void ghttpCleanup();
        }
    }
}
