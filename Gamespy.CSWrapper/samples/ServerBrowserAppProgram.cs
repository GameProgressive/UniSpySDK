///////////////////////////////////////////////////////////////////////////////
// File:	ServerBrowserAppProgram.cs
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
using Gamespy;
using Gamespy.Common;
using Gamespy.ServerBrowser;
using System.Net.Sockets;
using System.Net;

namespace gamespySample
{
    class ServerBrowserAppProgram
    {
        
        private const int   VERSION   = 0;      // ServerBrowserNew parameter; set to 0 unless otherwise directed by GameSpy
        private const int   MAX_QUERY = 20;	    // max number of queries the ServerBrowsing SDK will send out at one time
        private const bool  LAN_BROWSE = false; // set true for LAN only browsing

        /* ServerBrowserUpdate parameters */
        private const bool ASYNC = true;     // we will run the updates asynchronously
        private const bool DISC_ON_COMPLETE = true; // disconnect from the master server after completing update 
                                    // (future updates will automatically re-connect)
        private const string SECRET_KEY = "HA6zkS";
        private static bool  responseReceived = false;

        private  byte[] basicFields = { (byte)qr2DefaultKeys.HOSTNAME_KEY, 
                                        (byte)qr2DefaultKeys.GAMETYPE_KEY, 
                                        (byte)qr2DefaultKeys.MAPNAME_KEY, 
                                        (byte)qr2DefaultKeys.NUMPLAYERS_KEY, 
                                        (byte)qr2DefaultKeys.MAXPLAYERS_KEY 
                                      };
                                      
        public static void serverBrowserGeneralCallback
        (
            IntPtr serverBrowserInstance,
            ServerBrowserCallbackReason reason,
            IntPtr aServer,
            IntPtr userData
        )
        { 
            string defaultString = "No Value";
            StringBuilder serverAddress = new StringBuilder("");
            ushort port = 0;
            
            if (aServer != IntPtr.Zero)
            {
                serverAddress.Append(Marshal.PtrToStringAnsi(gamespyServerBrowser.SBServerGetPublicAddress(aServer)));
                port = gamespyServerBrowser.SBServerGetPublicQueryPort(aServer);
            }
            
	        switch (reason)
	        {
	            case ServerBrowserCallbackReason.sbc_serveradded:  // new SBServer added to the server browser list
		            // output the server's IP and port (the rest of the server's basic keys may not yet be available)
		            Console.WriteLine("\nServer Added: {0}:{1}", serverAddress, port);
		            break;
	            case ServerBrowserCallbackReason.sbc_serverchallengereceived: // received ip verification challenge from server
		            // informational, no action required
		            break;
	            case ServerBrowserCallbackReason.sbc_serverupdated:  // either basic or full information is now available for this server
		            // retrieve and print the basic server fields (specified as a parameter in ServerBrowserUpdate)
		            Console.WriteLine("\nServer updated: {0}:{1}", serverAddress, port);
		            StringBuilder name = new StringBuilder();
		            name.Append( Marshal.PtrToStringUni(gamespyServerBrowser.SBServerGetStringValueW(aServer,"hostname", defaultString)));
		            Console.WriteLine("  Host: {0}",  Marshal.PtrToStringUni(gamespyServerBrowser.SBServerGetStringValueW(aServer,"hostname", defaultString)));
		            Console.WriteLine("  Gametype: {0}", Marshal.PtrToStringUni(gamespyServerBrowser.SBServerGetStringValueW(aServer, "gametype", defaultString)));
                    Console.WriteLine("  Map: {0}", Marshal.PtrToStringUni(gamespyServerBrowser.SBServerGetStringValueW(aServer, "mapname", defaultString)));
		            Console.WriteLine("  Players/MaxPlayers: {0}/{1}", gamespyServerBrowser.SBServerGetIntValueW(aServer, "numplayers", 0), gamespyServerBrowser.SBServerGetIntValueW(aServer, "maxplayers", 0));
		            Console.WriteLine("  Ping: {0} ms", gamespyServerBrowser.SBServerGetPing(aServer));
		
		            // if the server has full keys (ServerBrowserAuxUpdate), print them
                    if (gamespyServerBrowser.SBServerHasFullKeys(aServer))
		            {
			            // print some non-basic server info
			            Console.WriteLine("  Frag limit: {0}", gamespyServerBrowser.SBServerGetIntValueW(aServer, "fraglimit", 0));
			            Console.WriteLine("  Time limit: {0} minutes", gamespyServerBrowser.SBServerGetIntValueW(aServer, "timelimit", 0));
			            Console.WriteLine("  Gravity: {0}", gamespyServerBrowser.SBServerGetIntValueW(aServer, "gravity", 0));

			            // print player info
			            Console.WriteLine("  Players:");
			            for(int i = 0; i < gamespyServerBrowser.SBServerGetIntValueW(aServer, "numplayers", 0); i++) // loop through all players on the server 
			            {
				            // print player key info for the player at index i
				            Console.WriteLine("\t{0}", Marshal.PtrToStringUni(gamespyServerBrowser.SBServerGetPlayerStringValueW(aServer, i, "player", defaultString)));
				            Console.WriteLine("\t\tScore: {0}", gamespyServerBrowser.SBServerGetPlayerIntValueW(aServer, i, "score", 0));
				            Console.WriteLine("\t\tDeaths: {0}", gamespyServerBrowser.SBServerGetPlayerIntValueW(aServer, i, "deaths", 0));
				            Console.WriteLine("\t\tTeam (0=Red/1=Blue): {0}", gamespyServerBrowser.SBServerGetPlayerIntValueW(aServer, i, "team", 0));
				            Console.WriteLine("\t\tPing: {0}", gamespyServerBrowser.SBServerGetPlayerIntValueW(aServer, i, "ping", 0));

			            }
			            // print team info (team name and team score)
			            Console.WriteLine("\tTeams (Score):");
                        for (int i = 0; i < gamespyServerBrowser.SBServerGetIntValueW(aServer, "numteams", 0); i++) 
			            {
				            Console.WriteLine("\t\t{0} ({1})", 
				                                Marshal.PtrToStringUni(gamespyServerBrowser.SBServerGetTeamStringValueW(aServer, i, "team", defaultString)),
                                                gamespyServerBrowser.SBServerGetTeamIntValueW(aServer, i, "score", 0));
			            }
                        responseReceived = true;

		            }
		            break;
	            case ServerBrowserCallbackReason.sbc_serverupdatefailed:
		            Console.WriteLine("\nServer Update Failed: {0}:{1}", serverAddress, port);
		            break;
	            case ServerBrowserCallbackReason.sbc_updatecomplete: // update is complete; server query engine is now idle (not called upon AuxUpdate completion)
		            Console.WriteLine("\nServer Browser Update Complete"); 
		            responseReceived = true; // this will let us know to stop calling ServerBrowserThink
		            break;
	            case ServerBrowserCallbackReason.sbc_queryerror: // the update returned an error 
		            Console.WriteLine("\nQuery Error: {0}", gamespyServerBrowser.ServerBrowserListQueryError(serverBrowserInstance));
		            responseReceived = true; // set to true here since we won't get an updatecomplete call
		            break;
	            default:
		            break;
	        }
            return; 
        }
	    
	    public static void aServerConnectCallback(IntPtr serverBrowser, ServerBrowserConnectRequestResult state, IntPtr gamesocket, IntPtr remoteaddr, IntPtr userData)
        {
	        if  (state == ServerBrowserConnectRequestResult.sbcs_succeeded)
		            Console.WriteLine("Connected to server");
		    else
                    Console.WriteLine("Failed to connect to server {0} ", state);
            
            responseReceived = true;
        }
        
        private void WaitForResponse(IntPtr serverBrowserInstance)
        {
            responseReceived = false;
            // think while the update is in progress
            while (!responseReceived)
            {
                ServerBrowserError nError = gamespyServerBrowser.ServerBrowserThink(serverBrowserInstance);
                if (nError != ServerBrowserError.sbe_noerror)
                {
                    Console.WriteLine("ServerBrowserThink Error 0x%x\n", nError);
                    return;
                }
                Thread.Sleep(new TimeSpan(100000));  // think should be called every 10-100ms; quicker calls produce more accurate ping measurements
            }    
        }
        
        public GSIACResult CheckServices(string gamename)
        {
            GSIACResult aResult;

            gamespyCommon.GSIStartAvailableCheck(gamename);

            // Continue processing while the check is in progress
            do
            {
                aResult = gamespyCommon.GSIAvailableCheckThink();
                Thread.Sleep(new TimeSpan(100000));
            } while (aResult == GSIACResult.GSIACWaiting);

            // Check the result
            switch (aResult)
            {
                case GSIACResult.GSIACAvailable:
                    Console.WriteLine("{0}: Online Services are available.", gamename);
                    break;
                case GSIACResult.GSIACUnavailable:
                    Console.WriteLine("{0}: Online services are unavailable.", gamename);
                    break;
                case GSIACResult.GSIACTemporarilyUnavailable:
                    Console.WriteLine("{0}: Online services are temporarily unavailable.", gamename);
                    break;
                default:
                    break;
            };
            return aResult;
        }

        public static gamespyServerBrowser.ServerBrowserCallback serverBrowserCallback = new gamespyServerBrowser.ServerBrowserCallback(serverBrowserGeneralCallback);
        public static gamespyServerBrowser.ServerBrowserConnectToServerCallback serverConnectCallback = new gamespyServerBrowser.ServerBrowserConnectToServerCallback(aServerConnectCallback);
       
        public void Run()
        {
            IntPtr updateServer = IntPtr.Zero; // this is the server we will call the Aux update for.

            string gamename = "gmtest";
            Console.WriteLine("\n------------ Server Browser C# sample ------------------------\n");

            if (CheckServices(gamename) != GSIACResult.GSIACAvailable)
            {
                Console.WriteLine("{0}: Online services are unavailable now.", gamename);
                return;
            }
                        
#if GSI_COMMON_DEBUG
            StringBuilder debugFileName = new StringBuilder("Debug_ServerBrowser");
            debugFileName.Append(".log");

            gamespyCommonDebug.gsOpenDebugFile(debugFileName.ToString());
            gamespyCommonDebug.gsSetDebugLevel(GSIDebugCategory.GSIDebugCat_All, GSIDebugType.GSIDebugType_All, GSIDebugLevel.Hardcore);
#endif
            IntPtr serverBrowserInstance = gamespyServerBrowser.ServerBrowserNewW(  gamename, 
                                                                                    gamename, 
                                                                                    SECRET_KEY, 
                                                                                    VERSION, 
                                                                                    MAX_QUERY, 
                                                                                    gamespyServerBrowser.QVERSION_QR2, 
                                                                                    LAN_BROWSE, 
                                                                                    serverBrowserCallback,
                                                                                    IntPtr.Zero
                                                                                  );
                                                                                  
            
            // Now get all the servers listed with a list of keys
            IntPtr basicFieldsPtr = Marshal.AllocHGlobal(basicFields.Length * Marshal.SizeOf(typeof(byte)));
            Marshal.Copy(basicFields, 0, basicFieldsPtr, basicFields.Length);

            Console.WriteLine("\nGet Server List form Backend\n");
            ServerBrowserError nError = gamespyServerBrowser.ServerBrowserUpdateW( serverBrowserInstance,   // server browser instance
                                                                                    ASYNC,                  // asynchronous request
                                                                                    DISC_ON_COMPLETE,       // diconnect when complete
                                                                                    basicFieldsPtr,         // key list
                                                                                    basicFields.Length,     // number of keys
                                                                                    string.Empty            // server filter
                                                                                 );
            if ( nError != ServerBrowserError.sbe_noerror)
            {
                Console.WriteLine("ServerBrowserUpdate Error 0x{0}", nError);
                return;
            }
            WaitForResponse(serverBrowserInstance);
            
            
            // Sort the server list (local call) according to a given criteri on a certain field
            gamespyServerBrowser.ServerBrowserSortW(serverBrowserInstance,
                                                   true, // ascending
                                                   "ping",
                                                   gamespyServerBrowser.ServerBrowserCompareMode.sbcm_int);
                                                   
            // Retrieve the number of servers from the local list
            int serverListCount = gamespyServerBrowser.ServerBrowserCount(serverBrowserInstance);
            Console.WriteLine("\nNumber of Servers : {0}", serverListCount);
            
            if (serverListCount > 0) Console.WriteLine("Sorted Servers in ascending order according to ping time:");
            
            for (int i = 0; i < serverListCount; i++)
            {
                IntPtr aServer = gamespyServerBrowser.ServerBrowserGetServer(serverBrowserInstance, i);
                Console.WriteLine("\tServer Address {0}:{1}", Marshal.PtrToStringAnsi(gamespyServerBrowser.SBServerGetPublicAddress(aServer)),
                                                            gamespyServerBrowser.SBServerGetPublicQueryPort(aServer));
                                                            
                // check if this is our resident sample server
                if (Marshal.PtrToStringUni(gamespyServerBrowser.SBServerGetStringValueW(aServer, "hostname", "No Value")) == 
                    "GameSpy QR2 Sample")
                {
                    updateServer = aServer;
                }
            }

            // If we found the resident running QR2 server          
            // Get all the keys of specific server
            if (updateServer != IntPtr.Zero) 
            {
                Console.WriteLine("\nGet Server Keys for {0}:{1}",
                                Marshal.PtrToStringAnsi(gamespyServerBrowser.SBServerGetPublicAddress(updateServer)),
                                gamespyServerBrowser.SBServerGetPublicQueryPort(updateServer));
                                
                gamespyServerBrowser.ServerBrowserAuxUpdateServer( serverBrowserInstance,
                                                                       updateServer, 
                                                                       ASYNC,
                                                                       true /*fullUpdate*/
                                                                 );
                WaitForResponse(serverBrowserInstance);
            }
                
            // Clear server list. This is a local call resets the server browser list.
            gamespyServerBrowser.ServerBrowserClear(serverBrowserInstance);
            
            Console.WriteLine("\n------------ Server Browser C# Sample Completed------------------------\n");
            Console.WriteLine("\nPress Enter to Continue....");
            Console.ReadLine();
        } 
    }
}