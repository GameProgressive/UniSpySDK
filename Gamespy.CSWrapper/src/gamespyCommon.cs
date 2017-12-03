///////////////////////////////////////////////////////////////////////////////
// File:	gamespyCommon.cs
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
        class gamespyCommon
        {
            // Common SDK API 

            [DllImport("gamespy.dll", CharSet=CharSet.Auto, CallingConvention=CallingConvention.Cdecl)]
            public static extern void GSIStartAvailableCheck( String gamename);

            [DllImport("gamespy.dll", CallingConvention=CallingConvention.Cdecl)]
            public static extern GSIACResult GSIAvailableCheckThink();

            [DllImport("gamespy.dll", CallingConvention=CallingConvention.Cdecl)]
            public static extern void gsCoreInitialize();

            [DllImport("gamespy.dll", CallingConvention=CallingConvention.Cdecl)]
            public static extern void gsCoreThink(UInt32 msTime);

            [DllImport("gamespy.dll", CallingConvention=CallingConvention.Cdecl)]
            public static extern void gsCoreShutdown();

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern GSCoreState gsCoreIsShutdown();

        }
        
        // Socket class to interface with the C library
        class gamespySocketIF
        {
            // C# equivalent to C++'s sockaddr_in / SOCKADDR_IN
            [StructLayout(LayoutKind.Sequential)]
            public struct in_addr
            {
                public UInt32 S_addr;
            }

            [StructLayout(LayoutKind.Sequential, Size = 16)]
            public struct sockaddr_in
            {
                public short sin_family;
                public ushort sin_port;

                public in_addr sin_addr;
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
                public byte[] padding;
            }

            [StructLayout(LayoutKind.Sequential, Size = 16)]
            public struct sockaddr {
                ushort sa_family;              /* address family */
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = 14)]
                public byte[] sa_data;            /* up to 14 bytes of direct address */
            };
        }
    }
}
