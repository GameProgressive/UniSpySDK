///////////////////////////////////////////////////////////////////////////////
// File:	gamespyQueryAndReport.cs
// SDK:		GameSpy Query and Reporting 2 (QR2) SDK C# Wrapper
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
using Gamespy.Common;
namespace Gamespy
{
    namespace QueryAndReport
    {
        class gamespyQueryReport
        {
            
            public const int MAX_NUM_PORTS_TO_TRY=100;

            // These values will be used at the start of all QR2 query packets. If you are processing query
            // data on your game socket, use these byte values to determine if a packet is for QR2 SDK.
            public const uint QR_MAGIC_1 = 0xFE;
            public const uint QR_MAGIC_2 = 0xFD;
            public const int  REQUEST_KEY_LEN = 4;
            public const int MAX_DATA_SIZE=1400;
            public const int MAX_REGISTERED_KEYS=254;
            public const int RECENT_CLIENT_MESSAGES_TO_TRACK=10;
            
            /* ip verification / spoof prevention */
            public const int QR2_IPVERIFY_TIMEOUT=4000;     // timeout after 4 seconds round trip time
            public const int QR2_IPVERIFY_ARRAY_SIZE=200;   // allowed outstanding queries in those 4 seconds
            public const int QR2_IPVERIFY_MAXDUPLICATES=5;  // allow maximum of 5 requests per IP/PORT

            // Structures            
            [StructLayout(LayoutKind.Sequential)]
            public struct qr2Buffer
            {
            	[MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_DATA_SIZE)] 
	            byte[] buffer;
	            int len;
            };
            
            [StructLayout(LayoutKind.Sequential)]
            public struct qr2Keybuffer
            {
                [MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_REGISTERED_KEYS)] 
                byte[] keys;
	            int numKeys;
            };
    
            public struct qr2IPVerifyInfo
            {
                public IntPtr   addr;      // sockaddr_in; addr = 0 when not in use
	            public uint     challenge;
	            public ulong    createTime; 
            };

            //calback functions
            
            // NegotiateProgressFunc - The callback that gets executed from NNBeginNegotiation
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void QRServerKeyCallback (int keyId, ref qr2Buffer /*qr2_buffer_t*/ outBuf, IntPtr userData);

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  void QRPlayerTeamKeyCallback (int keyId, int index, ref qr2Buffer /*qr2_buffer_t*/ outBuf, IntPtr userData);	

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void QRKeyListCallback(QueryReportKeyType keyType, ref qr2Keybuffer /*qr2_keybuffer_t*/ keyBuffer, IntPtr userData);	

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  int QRCountCallback(QueryReportKeyType keyType, IntPtr userData);	

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  void QRAddErrorCallback (QueryReportResult /*qr2_error_t*/ error, String errMsg, IntPtr userData);	

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  void QRNatnegCallback(int cookie, IntPtr userData);	

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  void QRClientMessageCallback (String data, int len, IntPtr userData);	

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  void QRPublicAddressCallback (uint ip, ushort port, IntPtr userData);

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  void QRClientConnectedCallback (IntPtr gameSocket, IntPtr /*gamespySocketIF.sockaddr_in*/ remoteAddr, IntPtr userData);

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate  void QRHostRegisteredCallback (IntPtr userData);

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void CDKeyProcess(IntPtr buf, int len, IntPtr /*gamespySocketIF.sockaddr_in*/ fromAddr);

            // Structures
            
            // This structure is declared solely for consistency with the current SDK.
            // It is used only when reporting multiple servers within the same game instance.
            // Therefore, it is very rare that it needs to be used.
            // Hence, it is not fully tested and supported in the C# package. 
            public struct qr2_implementation_s
            {
	            public IntPtr heartBeatSock;
	            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)] 
	            public byte[] gameName;
	            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)] 
	            public byte[]  secretKey;
	            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
	            public byte[] instanceKey;
	            public QRServerKeyCallback serverKeyCallback;
	            public QRPlayerTeamKeyCallback playerKeyCallback;
	            public QRPlayerTeamKeyCallback teamKeyCallback;
	            public QRKeyListCallback keyListCallback;
	            public QRCountCallback playerteamCountCallback;
	            public QRAddErrorCallback adderrorCallback;					
	            public QRNatnegCallback natnegCallback;
	            public QRClientMessageCallback clientMsgCallback;
	            public QRPublicAddressCallback publicAddrCallback;
	            public QRClientConnectedCallback clientConnCallback;
	            public QRHostRegisteredCallback hostRegCallback;
	            public ulong lastHeartBeat;
	            public ulong lastKeepAlive;
	            public int userStateChangeRequested;
	            public int listedState;
	            public int isPublic;	 
	            public int qport;
	            public int readSocket;
	            public int natNegotiate;
	            public gamespySocketIF.sockaddr_in hearBeatAddr;
	            public CDKeyProcess cdKeyProcess;
	            [MarshalAs(UnmanagedType.ByValArray, SizeConst = RECENT_CLIENT_MESSAGES_TO_TRACK)] 
	            public int[] clientMessageKeys;
	            public int currentMessageKey;
	            public uint publicIp;
	            public ushort publicPort;
	            public IntPtr userData;

	            public byte backendOptions; // received from server inside challenge packet 

                public IntPtr ipVerify; // struct qr2_ipverify_info_s ipverify[QR2_IPVERIFY_ARRAY_SIZE];
            };         

            // API functions
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_register_keyW(int keyId, /*[MarshalAs(UnmanagedType.LPTStr)] string*/ IntPtr key);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern QueryReportResult qr2_initW
            (
                IntPtr /*[out] qr2_t* */ qrec, 
                String ip, 
                int basePort,
                [MarshalAs(UnmanagedType.LPTStr)] string gameName,
                [MarshalAs(UnmanagedType.LPTStr)] string secretKey,
	            int isPublic,       // is the server public as opposed to LAN.
	            int natNegotiate,   // this is true when private ip should be visible from the server browser listing.
                QRServerKeyCallback serverKeyCallback,
                QRPlayerTeamKeyCallback playerKeyCallback,
                QRPlayerTeamKeyCallback teamKeyCallback,
                QRKeyListCallback keyListCallback,
                QRCountCallback playerTeamCountCallback,
                QRAddErrorCallback addErrorCallback,
	            IntPtr userData
	        );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern QueryReportResult qr2_init_socketW
            (
                IntPtr /*[out] qr2_t* */ qrec, 
                IntPtr /*SOCKET*/ sock, 
                int boundPort, 
                String gameName, 
                String secretKey,
                int isPublic, 
                int natNegotiate,
                QRServerKeyCallback serverKeyCallback,
                QRPlayerTeamKeyCallback playerKeyCallback,
                QRPlayerTeamKeyCallback teamKeyCallback,
                QRKeyListCallback keyListCallback,
                QRCountCallback playerTeamCountCallback,
                QRAddErrorCallback addErrorCallback,
                IntPtr userData
            );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_think(IntPtr /* qr2_t */  qrec);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_parse_queryW(IntPtr /* qr2_t */  qrec, [MarshalAs(UnmanagedType.LPTStr)] string query, int len, IntPtr /* gamespySocketIF.sockaddr* */ sender);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_send_statechanged(IntPtr /*qr2_t*/ qrec);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_shutdown(IntPtr /*qr2_t*/ qrec);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool qr2_keybuffer_add(ref qr2Keybuffer keyBuffer, int keyId);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool qr2_buffer_addW(ref qr2Buffer outBuf, [MarshalAs(UnmanagedType.LPTStr)] string value);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool qr2_buffer_add_int(ref qr2Buffer outBuf, int value);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_register_natneg_callback(IntPtr /*qr2_t*/ qrec, QRNatnegCallback natnegCallback);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_register_clientmessage_callback(IntPtr /*qr2_t*/ qrec, QRClientMessageCallback clientMsgCallback);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_register_publicaddress_callback(IntPtr /*qr2_t*/ qrec, QRPublicAddressCallback publicAddrcallback);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_register_clientconnected_callback(IntPtr /*qr2_t*/ qrec, QRClientConnectedCallback clinetConnCallback);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void qr2_register_hostregistered_callback(IntPtr /*qr2_t*/ qrec, QRHostRegisteredCallback HostRegCallback);
            
        }
    }
}
