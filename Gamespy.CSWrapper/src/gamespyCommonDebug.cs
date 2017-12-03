///////////////////////////////////////////////////////////////////////////////
// File:	gamespyCommonDebug.cs
// SDK:		GameSpy Common C# Wrapper
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
    namespace Common
    {
        class gamespyCommonDebug
        {
            // Common SDK API 

            public delegate void GSIDebugCallback
            (
                GSIDebugCategory debugCategory,
                GSIDebugType debugType,
                byte debugLevel,
                IntPtr token, 
                IntPtr paramslist
            );

            [DllImport("gamespy.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
            public static extern void  gsSetDebugCallback(GSIDebugCallback theCallback);
            
            [DllImport("gamespy.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
            public static extern void gsSetDebugLevel
            (
                GSIDebugCategory    theCat, 
                GSIDebugType        theType,
                GSIDebugLevel       theLevel
            );
            
            [DllImport("gamespy.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr gsOpenDebugFile([MarshalAs(UnmanagedType.LPStr)] string fileName);
        }
    }
}
