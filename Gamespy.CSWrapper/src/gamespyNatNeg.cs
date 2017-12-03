///////////////////////////////////////////////////////////////////////////////
// File:	gamespyNatNeg.cs
// SDK:		GameSpy Nat Negotiation SDK C# Wrapper
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
using System.Net.Sockets;
using System.Threading;
using Gamespy.Auth;

namespace Gamespy
{
    namespace NatNegoation
    {    
        class gamespyNatNeg
        {
            ///////////////
            
            public const int NATNEG_MAGIC_LEN = 6;
            public const int NN_MAGIC_0     = 0xFD;
            public const int NN_MAGIC_1     = 0xFC;
            public const int NN_MAGIC_2     = 0x1E;
            public const int NN_MAGIC_3     = 0x66;
            public const int NN_MAGIC_4     = 0x6A;
            public const int NN_MAGIC_5     = 0xB2;
            public static int[] nnMagic = new int[NATNEG_MAGIC_LEN] { 0xfd, 0xfc, 0x1e, 0x66, 0x6a, 0xb2 }; 
            
            public enum NatifyPacket
            { 
                packet_map1a =0 , 
                packet_map2, 
                packet_map3, 
                packet_map1b, 
                NUM_PACKETS 
            }; 
            public enum NatType
            { 
                no_nat, 
                firewall_only, 
                full_cone, 
                restricted_cone, 
                port_restricted_cone, 
                symmetric, 
                unknown, 
                NUM_NAT_TYPES 
            } ;
            
            public enum NatPromiscuity
            { 
                promiscuous, 
                not_promiscuous, 
                port_promiscuous, 
                ip_promiscuous, 
                promiscuity_not_applicable, 
                NUM_PROMISCUITY_TYPES 
            } ;
            
            public enum NatMappingScheme
            { 
                unrecognized, 
                private_as_public, 
                consistent_port, 
                incremental, 
                mixed, 
                NUM_MAPPING_SCHEMES 
            } ;

            // Structures
            
            // AddressMapping - Internal and external address pairing for an observed network address translation.
            [StructLayout(LayoutKind.Sequential)]
            public struct AddressMapping 
            {
	            public uint     privateIp;		// Internal IP address.
	            public ushort   privatePort;	// Internal port number.
	            public uint     publicIp;		// External IP address.
	            public ushort   publicPort;	    // External port number.
            };
            
            // NAT - contains as many properties of the NAT as could be determined.
            [StructLayout(LayoutKind.Sequential)]
            public struct NAT 
            {
	            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)] 
	            public byte[] brand ;           // NAT device brand/vendor (not currently used).
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
                public byte[] model;		    // NAT device model name/number (not currently used).
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
                public byte[] firmware;	        // NAT device brand/vendor (not currently used).
                public int /*bool*/ ipRestricted;	// true if the NAT drops packets from unsolicited IP addresses.
                public int /*bool*/ portRestricted;	// true if the NAT drops packets from unsolicited ports.
                public NatPromiscuity promiscuity;	// The type of promiscuity the NAT allows.
                public NatType natType;			// The type of NAT as defined by RFC2663.
                public NatMappingScheme mappingScheme;	    // The type of port mapping/allocation scheme used by the NAT.
	            //[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 4)] public AddressMapping[] mappings; // Port mappings observed during the detection process.
                IntPtr mappings;
                public int /*bool*/ qr2Compatible;	// true if the NAT is compatible with QR2.
            };

            // This external array contains all 6 magic bytes to quickly check incoming packets for the bytes
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)] public byte[] NNMagicData;

            // NegotiateProgressFunc - The callback that gets executed from NNBeginNegotiation
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void NegotiateProgressFunc
            (
                NatNegotiateState state,
                IntPtr  userData
            );

            // NegotiateCompletedFunc - The callback that gets executed from NNBeginNegotiation when negotiation is complete.
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void NegotiateCompletedFunc
            (
                NatNegotiateResult result, 
                IntPtr          gamesocket, // pointer to Socket
                IntPtr          remoteaddr, // pointer to struct sockaddr_in
                IntPtr          userdata
            );

            // NatDetectionResultsFunc
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void NatDetectionResultsFunc(bool success, [MarshalAs(UnmanagedType.Struct)] NAT nat);

            // NNBeginNegotiation
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern NatNegotiateError NNBeginNegotiation
            (
                int cookie, 
                int clientindex, 
                NegotiateProgressFunc progresscallback, 
                NegotiateCompletedFunc completedcallback, 
                IntPtr userdata
            );

            // NNBeginNegotiationWithSocket
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern NatNegotiateError NNBeginNegotiationWithSocket
            (
                IntPtr gamesocket, 
                int cookie, 
                int clientindex, 
                NegotiateProgressFunc progresscallback, 
                NegotiateCompletedFunc completedcallback, 
                IntPtr userdata
            );

            // NNThink
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void NNThink();

            // NNProcessData - Processes data received from a shared socket.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void NNProcessData
            (
                /*[MarshalAs(UnmanagedType.LPStr)] String*/ IntPtr data, 
                int     len, 
                IntPtr  fromaddr // pointer to struct sockaddr_in
            );

            // NNCancel
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void NNCancel(int cookie);

            // NNFreeNegotiateList
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void NNFreeNegotiateList();

            // NNStartNatDetection
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern NatNegotiateError NNStartNatDetection(NatDetectionResultsFunc resultscallback);

            //Used for over-riding the default negotiation hostnames. 
            //Should not be used normally.
            [MarshalAs(UnmanagedType.LPStr)] public string  Matchup3Hostname;
            [MarshalAs(UnmanagedType.LPStr)] public string  Matchup2Hostname;
            [MarshalAs(UnmanagedType.LPStr)] public string  Matchup1Hostname;
        }
    }
}
