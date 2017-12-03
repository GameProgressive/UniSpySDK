///////////////////////////////////////////////////////////////////////////////
// File:	gamespyServerBrowser.cs
// SDK:		GameSpy Server Browsing SDK C# Wrapper
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
// 

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Net.Sockets;
using System.Threading;
using Gamespy.Common;

namespace Gamespy
{
    namespace ServerBrowser
    {
        class gamespyServerBrowser
        {
            public const int QVERSION_QR1 = 0;
            public const int QVERSION_QR2 = 1;
            public const int MAX_FILTER_LEN = 511 ; //Maximum length for the SQL filter string
            
            // Callbacks
            
            //  ServerBrowserCallback - for server browser updates
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
	        public delegate void ServerBrowserCallback 
	        (
	            IntPtr                      serverBrowserInstance, 
	            ServerBrowserCallbackReason reason, 
	            IntPtr aServer, 
	            IntPtr userData
	         );
            
            // SBServerKeyEnumFn
            //		Callback function used for enumerating the keys/values for a server.
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void ServerBrowserServerKeyEnumFunc 
            (
                [MarshalAs(UnmanagedType.LPTStr)] string key,
                [MarshalAs(UnmanagedType.LPTStr)] string value,
                IntPtr userData
            );
            
            //  SBConnectToServerCallback -Gets called when the state of the connect attempt changes.
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
	        public delegate void ServerBrowserConnectToServerCallback
	        (
	            IntPtr serverBrowserInstance, 
	            ServerBrowserConnectRequestResult state, 
	            IntPtr gameSocket, // SOCKET
	            IntPtr remoteAddr, // pointer to gamespySocketIF.sockaddr_in
	            IntPtr userData
	         );

            // API Calls
            
            // ServerBrowserNew - Initialize the ServerBrowser SDK.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr ServerBrowserNewW
            (
                [MarshalAs(UnmanagedType.LPTStr)] 
                string queryForGameName,    // game name to query
                [MarshalAs(UnmanagedType.LPTStr)] 
                string queryFromGameName,   // this game's name
                [MarshalAs(UnmanagedType.LPTStr)] 
                string secretKey,           // secret key assigned to this game
                int queryFromVersion,       // Set to zero unless directed otherwise by GameSpy
                int maxConcUpdates,         // The maximum number of queries the ServerBrowsing SDK will send out. >= 20.
                int queryVersion,           // use QVERSION_QR2
                bool lanBrowse,             // bool to turn on only LAN browsing               
                ServerBrowserCallback callback, 
                IntPtr userData
            );

        // ServerBrowserFree - terminates the sdk

        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ServerBrowserFree
        (
            IntPtr serverBrowser  // Server Browser
        );


        // ServerBrowserUpdate - Retrieves the current list of games from the GameSpy master server.
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ServerBrowserError ServerBrowserUpdateW
        (
            IntPtr serverBrowserInstance, 
            bool async,                     // true if run in non-blocking mode.
            bool disconnectOnComplete,      //  true if disconnect after update received.
            IntPtr basicFields,             // array of strings 
            int numBasicFields,             // number of fields
            [MarshalAs(UnmanagedType.LPTStr)] string serverFilter  // SQL like string used to remove
                                                    // unwanted servers from the downloaded list.
        );

        // ServerBrowserLimitUpdate - Retrieves the current limited list of games from the GameSpy master server. 
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ServerBrowserError ServerBrowserLimitUpdateW
        (
            IntPtr serverBrowserInstance,  // ServerBrowser
            bool async, 
            bool disconnectOnComplete, 
            IntPtr basicFields, // array of char strings - unsigned char *
            int numBasicFields, 
            [MarshalAs(UnmanagedType.LPTStr)] string serverFilter, 
            int maxServers
        );
        
        // ServerBrowserThink - call this in the main loop
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ServerBrowserError ServerBrowserThink
        (
            IntPtr serverBrowserInstance // ServerBrowser
        );


        // ServerBrowserLANUpdate - Retrieves the current list of games broadcasting on the local network.
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ServerBrowserError ServerBrowserLANUpdate
        (
            IntPtr serverBrowserInstance, // ServerBrowser 
            bool async, 
            ushort startSearchPort, 
            ushort endSearchPort
        );

        // ServerBrowserAuxUpdateIPW - Queries key/values from a single server.
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ServerBrowserError ServerBrowserAuxUpdateIP
        (
            IntPtr serverBrowserInstance, // ServerBrowser 
            [MarshalAs(UnmanagedType.LPTStr)] string ip, 
            ushort port, 
            bool viaMaster, 
            bool async, 
            bool fullUpdate
        );

        // ServerBrowserAuxUpdateServer - Query key/values from a single server that has already been added to the internal list.
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ServerBrowserError ServerBrowserAuxUpdateServer
        (
            IntPtr serverBrowserInstance, // ServerBrowser 
            IntPtr aServer,               // SBServer
            bool async, 
            bool fullUpdate
        );

        // ServerBrowserDisconnect
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ServerBrowserDisconnect
        (
            IntPtr serverBrowserInstance // ServerBrowser
        );

        // ServerBrowserState
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ServerBrowserState ServerBrowserState
        (
            IntPtr serverBrowserInstance // ServerBrowser
        );

        // ServerBrowserRemoveIP
        //		Removes a server from the local list.
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ServerBrowserRemoveIP
        (
            IntPtr serverBrowserInstance,       // ServerBrowser 
            [MarshalAs(UnmanagedType.LPTStr)] string ip, 
            ushort port                         // in network byte order.
        );

        // ServerBrowserRemoveServer
        //		Removes a server from the local list.
        [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ServerBrowserRemoveServer
        (
            IntPtr serverBrowserInstance, // ServerBrowser 
            IntPtr aServer
        );

            // ServerBrowserHalt
            //		Stop an update in progress. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void ServerBrowserHalt
            (
                IntPtr serverBrowserInstance // ServerBrowser
            );

            // ServerBrowserClear
            //		Clear the current server list.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void ServerBrowserClear
            (
                IntPtr serverBrowserInstance // ServerBrowser
            );

            // ServerBrowserErrorDesc
            // Summary
            //		Returns a human readable string for the specified SBError.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern string ServerBrowserErrorDesc
            (
                IntPtr serverBrowserInstance, // ServerBrowser
                ServerBrowserError error
            );

            // ServerBrowserListQueryError
            //		Returns the ServerList error string, if any.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern string ServerBrowserListQueryError
            (
                IntPtr serverBrowserInstance // ServerBrowser
            );

            // ServerBrowserGetServer
            //		Returns the SBServer object at the specified index. Pointer to SBServer
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr ServerBrowserGetServer
            (
                IntPtr serverBrowserInstance, // ServerBrowser 
                int index
            );

            // ServerBrowserGetServerByIP
            //		Returns the SBServer with the specified IP.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*SBServer*/ ServerBrowserGetServerByIP
            (
                IntPtr serverBrowserInstance,   // ServerBrowser
                [MarshalAs(UnmanagedType.LPTStr)] string ip, 
                ushort port                     // in network byte order
            );

            // ServerBrowserCount
            // Summary
            //		Retrieves the current list of games from the GameSpy master server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int ServerBrowserCount
            (
                IntPtr serverBrowserInstance // ServerBrowser
            );

            // ServerBrowserPendingQueryCount
            //		Retrieves the number of servers with outstanding queries. Use this to check 
            //		progress while asynchronously updating the server list.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int ServerBrowserPendingQueryCount
            (
                IntPtr serverBrowserInstance // ServerBrowser
            );

            // ServerBrowserGetMyPublicIP
            //		Returns the local client's external (firewall) address. 
            //      Pointer to a byte array
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr ServerBrowserGetMyPublicIP   
            (
                IntPtr serverBrowserInstance // ServerBrowser
            );

            // ServerBrowserGetMyPublicIPAddr
            //		Returns the local client's external (firewall) address.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint ServerBrowserGetMyPublicIPAddr
            (
                IntPtr serverBrowserInstance // ServerBrowser
            );

            // ServerBrowserSendNatNegotiateCookieToServer
            //		Sends a NAT negotiation cookie to the server. The cookie is sent via the master server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ServerBrowserError ServerBrowserSendNatNegotiateCookieToServer
            (
                IntPtr serverBrowserInstance, // ServerBrowser 
                [MarshalAs(UnmanagedType.LPTStr)] string ip, 
                ushort port, 
                int cookie
            );

            // ServerBrowserSendMessageToServer
            //		Sends a game specific message to the specified IP/port. This message is routed through the master server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ServerBrowserError ServerBrowserSendMessageToServer
            (
                IntPtr serverBrowserInstance, // ServerBrowser
                [MarshalAs(UnmanagedType.LPTStr)] string ip, 
                ushort port, 
                IntPtr data, // char* 
                int len
            );

            // ServerBrowserConnectToServer
            //		Connects to a game server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ServerBrowserError ServerBrowserConnectToServer
            (
                IntPtr serverBrowserInstance,   // ServerBrowser
                IntPtr aServer,                 // SBServer
                ServerBrowserConnectToServerCallback callback
            );

            // ServerBrowserConnectToServerWithSocket - Connects to a game server with the gamesocket provided.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ServerBrowserError ServerBrowserConnectToServerWithSocket
            (
                IntPtr serverBrowserInstance, // ServerBrowser
                IntPtr aServer, 
                IntPtr /*SOCKET*/ gamesocket, 
                ServerBrowserConnectToServerCallback callback
            );

            //  ServerBrowserCompareMode
            public  enum ServerBrowserCompareMode
            {
                sbcm_int,		// Assume the values are int, and do an integer comparison.
                sbcm_float,		// Assume the values are float, and do a float comparison.
                sbcm_strcase,	// Assume the values are strings, and do a case-sensitive 
	                        // string comparison.
                sbcm_stricase	// Assume the values are strings, and do a case-insensitive 
	                        // string comparison.
            } ;

            // ServerBrowserSort - Sort the current list of servers.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void ServerBrowserSortW
            (
                IntPtr serverBrowserInstance, // ServerBrowser
                bool ascending, 
                [MarshalAs(UnmanagedType.LPTStr)] string sortkey, 
                ServerBrowserCompareMode comparemode
            );

            // ServerBrowserLANSetLocalAddr - Sets the network adapter to use for LAN broadcasts (optional).
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void ServerBrowserLANSetLocalAddr
            (
                IntPtr serverBrowserInstance, // ServerBrowser 
                [MarshalAs(UnmanagedType.LPStr)] string theAddr
            );

            // SBServerGetConnectionInfo - Checks if Nat Negotiation is required.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SBServerGetConnectionInfo
            ( 
                IntPtr/*ServerBrowser*/ serverBrowser, 
                IntPtr aServer, 
                ushort PortToConnectTo, 
                [MarshalAs(UnmanagedType.LPStr)] string /*char * */ipstring_out
            );

            // SBServerHasPrivateAddress - Tests to see fi a private address is available for the server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SBServerHasPrivateAddress
            (
                IntPtr aServer // SBServer
            );

            // SBServerDirectConnect - Indicates whether the server supports direct UDP connections.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SBServerDirectConnect
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetPing - Returns the stored ping time for the specified server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int SBServerGetPing
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetPublicAddress - Returns the external address of the SBServer, For users behind a NAT or firewall.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*char * */ SBServerGetPublicAddress
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetPrivateAddress - Returns the local ip address of the SBServer. For users behind a NAT or firewall, 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr /*char * */SBServerGetPrivateAddress
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetPublicInetAddress - Returns the external address of the SBServer, if any. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint SBServerGetPublicInetAddress
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetPrivateInetAddress - Returns the internal address of the SBServer, if any.  
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern uint SBServerGetPrivateInetAddress
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetPublicQueryPort - Returns the public query port of the specified server. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ushort SBServerGetPublicQueryPort
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetPrivateQueryPort - Returns the private query port of the specified server. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern ushort SBServerGetPrivateQueryPort
            (
                IntPtr aServer // SBServer
            );

            // SBServerHasBasicKeys - Determine if basic information is available for the specified server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SBServerHasBasicKeys
            (
                IntPtr aServer // SBServer
            );

            // SBServerHasFullKeys - Determine if full information is available for the specified server.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SBServerHasFullKeys
            (
                IntPtr aServer // SBServer
            );

            // SBServerHasValidPing - Determines if a server has a valid ping value (otherwise the ping will be 0).
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SBServerHasValidPing
            (
                IntPtr aServer // SBServer
            );

            // SBServerGetStringValueW - Returns the value associated with the specified key. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr SBServerGetStringValueW
            (
                IntPtr aServer, // SBServer, 
                [MarshalAs(UnmanagedType.LPTStr)] string key, 
                [MarshalAs(UnmanagedType.LPTStr)] string sdefault
            );

            // SBServerGetIntValueW - Returns the value associated with the specified key.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int SBServerGetIntValueW
            (
                IntPtr  aServer,                                //  A valid SBServer object.
                [MarshalAs(UnmanagedType.LPTStr)] string key,   // value associated with this key will be returned
                int idefault
            );

            // SBServerGetFloatValue - Returns the value associated with the specified key. This value is returned 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern double SBServerGetFloatValueW
            (
                IntPtr  aServer,                                //  A valid SBServer object.
                [MarshalAs(UnmanagedType.LPTStr)] string key,   // value associated with this key will be returned
                double fdefault
            );

            // SBServerGetBoolValue - Returns the value associated with the specified key.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern bool SBServerGetBoolValueW
            (
                IntPtr  aServer, //  A valid SBServer object.
                [MarshalAs(UnmanagedType.LPTStr)] string key,   // value associated with this key will be returned
                bool    bdefault
            );

            // SBServerGetPlayerStringValue - Returns the value associated with the specified player's key. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr SBServerGetPlayerStringValueW
            (
               IntPtr serverBrowserInstance, 
               int playerIndex,                                // index of the player
               [MarshalAs(UnmanagedType.LPTStr)] string key,   // value associated with this key will be returned
               [MarshalAs(UnmanagedType.LPTStr)] string sdefault
            );

            // SBServerGetPlayerIntValueW - Returns the value associated with the specified player's key. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int SBServerGetPlayerIntValueW
            (
                IntPtr serverBrowserInstance, 
                int playerIndex,                                // index of the player
                [MarshalAs(UnmanagedType.LPTStr)] string key,   // value associated with this key will be returned
                int idefault                                    // value to return if the key is not found
             );

            // SBServerGetPlayerFloatValue - Returns the value associated with the specified player's key. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern double SBServerGetPlayerFloatValueW
            (
                IntPtr serverBrowserInstance, 
                int playerIndex,                                // index of the player
                [MarshalAs(UnmanagedType.LPTStr)] string key,   // value associated with this key will be returned
                double fdefault                                 // value to return if the key is not found
             );

            // SBServerGetTeamStringValue - Returns the value associated with the specified teams' key. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr SBServerGetTeamStringValueW
            (
                IntPtr  serverBrowserInstance, 
                int     teamIndex,                                  // index of the team
                [MarshalAs(UnmanagedType.LPTStr)] string key,       // value associated with this key will be returned
                [MarshalAs(UnmanagedType.LPTStr)] string sdefault   // value to return if the key is not found
            );

            // SBServerGetTeamIntValueW - Returns the value associated with the specified teams' key.
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern int SBServerGetTeamIntValueW
            (
                IntPtr  serverBrowserInstance, 
                int     teamIndex,                             // index of the team
                [MarshalAs(UnmanagedType.LPTStr)] string key,  // value associated with this key will be returned
                int idefault                                   // value to return if the key is not found
            );

            // SBServerGetTeamFloatValue - Returns the value associated with the specified teams' key. 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern double SBServerGetTeamFloatValueW
            (
                IntPtr serverBrowserInstance, 
                int teamIndex,                                  // index of the team
                [MarshalAs(UnmanagedType.LPTStr)] string  key,  // value associated with this key will be returned
                double fdefault                                 // value to return if the key is not found
            );

            // SBServerEnumKeys - Enumerates the keys/values for a given server by calling KeyEnumFn
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern void SBServerEnumKeysW
            (
                IntPtr serverBrowserInstance,
                ServerBrowserServerKeyEnumFunc serverKeyEnumFunc, 
                IntPtr userData
            ); 
        }
    }
}

