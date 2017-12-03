///////////////////////////////////////////////////////////////////////////////
// File:	QR2AppProgram.cs
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
using Gamespy.QueryAndReport;
using System.Net.Sockets;
using System.Net;

namespace gamespySample
{
    public class gameApp
    {
        public const int MAX_PLAYER_COUNT = 4;
        public const int team0 = 0;
        public const int team1 = 1;
        
    	public List<player> players;
	    public List<team> teams;
	    public String mapName;
	    public String hostName;
	    public String gameType;
	    public int numTeams;
	    public int numPlayers;
	    public int maxPlayers;
	    public int fragLimit;
	    public int timeLimit;
	    public int teamPlay;
	    public int rankingsOn;
	    public int gravity;
	    public int hostPort;
	    
	    public gameApp()
	    {
	        players = new List<player>();
	        players.Add(new player("Ryan", 27, 2, team0));
	        players.Add(new player("Nursel", 20, 3, team1)); 
	        
	        teams   = new List<team>();
	        teams.Add(new team("team0", 270));
	        teams.Add(new team("team1", 200));
	        
	        this.mapName = "dungeon";
	        this.gameType = "death match";
	        this.hostName = "C Sharp QR2 Sample";
	        this.fragLimit = 30;
	        this.timeLimit = 100;
	        this.teamPlay = 1;  // team vs team
	        this.numPlayers = players.Count();
	        this.maxPlayers = gameApp.MAX_PLAYER_COUNT;
	        this.numTeams = teams.Count();
	        this.rankingsOn = 1 ; // 0 if the score should not be in the leaderboards.
	        this.gravity = 800;
	        this.hostPort = 12345;
	    }
    }
        

    //representative of a game player 
    public class  player
    {
        public String   name;
        public int      frags;
        public int      deaths;
        public int      ping;
        public int      team;
        
        public player(string _name, int _frags, int _deaths, int _team )
        {
            this.name = new String(_name.ToCharArray());
            this.frags = _frags;
            this.deaths = _deaths;
            this.team = _team;
        }
    };
    //representative of a team 
    public class team
    {
        public String       name;
        public int          score;
        public team(String _name, int _score)
        {
            this.name = new String(_name.ToCharArray());
            this.score = _score;
        }
    };
         
    class QR2AppProgram
    {
        // create dummy data for test purposes 
        public static gameApp myGame = new gameApp();
         
        public static IPEndPoint remoteEndPoint = new IPEndPoint(IPAddress.Any, 0);

        private const int GRAVITY_KEY = (int) qr2DefaultKeys.NUM_RESERVED_KEYS+1; // a sample custom key

        public static void qrServerKeyCallback(int keyId, ref gamespyQueryReport.qr2Buffer /*qr2_buffer_t*/ outBuf, IntPtr userData)
        {

            Console.Write("\nIn {0} : ", System.Reflection.MethodBase.GetCurrentMethod().Name);

            Console.Write("Adding server key {0}", (qr2DefaultKeys)keyId);
            
            switch ((qr2DefaultKeys)keyId)
            {
                case qr2DefaultKeys.HOSTNAME_KEY:
	                gamespyQueryReport.qr2_buffer_addW(ref outBuf , myGame.hostName);
	                break;
                case qr2DefaultKeys.HOSTPORT_KEY:
                    gamespyQueryReport.qr2_buffer_add_int(ref outBuf, myGame.hostPort);
	                break;
                case qr2DefaultKeys.MAPNAME_KEY:
	                gamespyQueryReport.qr2_buffer_addW(ref outBuf , myGame.mapName);
	                break;
                case qr2DefaultKeys.GAMETYPE_KEY:
	                gamespyQueryReport.qr2_buffer_addW(ref outBuf , myGame.gameType);
	                break;
                case qr2DefaultKeys.NUMPLAYERS_KEY:
	                gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.numPlayers);
	                break;
                case qr2DefaultKeys.NUMTEAMS_KEY:
	                gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.numTeams);
	                break;
                case qr2DefaultKeys.MAXPLAYERS_KEY:
	                gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.maxPlayers);
	                break;
                case qr2DefaultKeys.TEAMPLAY_KEY:
	                gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.teamPlay);
	                break;
                case qr2DefaultKeys.FRAGLIMIT_KEY:
	                gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.fragLimit);
	                break;
                case qr2DefaultKeys.TIMELIMIT_KEY:
	                gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.timeLimit);
	                break;
                default:
                {
                    if (keyId == GRAVITY_KEY)
                    {
                        Console.Write(" = custom Key [GRAVITY_KEY]");
                        gamespyQueryReport.qr2_buffer_add_int(ref outBuf, myGame.gravity);
                    }
                    else
	                    gamespyQueryReport.qr2_buffer_addW(ref outBuf , "");
                    break;
	            }
            }
            Console.WriteLine();
        }

        public static void qrPlayerKeyCallback(int keyId, int index, ref gamespyQueryReport.qr2Buffer /*qr2_buffer_t*/ outBuf, IntPtr userData)
        {
            
           //check for valid index
           if (index >= myGame.numPlayers)
           {
              gamespyQueryReport.qr2_buffer_addW(ref outBuf, "");
              return;
           }

           Console.Write("\nIn {0} : ", System.Reflection.MethodBase.GetCurrentMethod().Name);

           switch ((qr2DefaultKeys)keyId)
            {
                case qr2DefaultKeys.PLAYER__KEY:
                    gamespyQueryReport.qr2_buffer_addW(ref outBuf , myGame.players[index].name);
                    Console.WriteLine(" Reporting player[{0}] key {1} = {2}", index, (qr2DefaultKeys)keyId, myGame.players[index].name);
                    break;
                case qr2DefaultKeys.SCORE__KEY:
                    gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.players[index].frags);
                    Console.WriteLine(" Reporting player[{0}] key {1} = {2}", index, (qr2DefaultKeys)keyId, myGame.players[index].frags);
                    break;
                case qr2DefaultKeys.DEATHS__KEY:
                    gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.players[index].deaths);
                    Console.WriteLine(" Reporting player[{0}] key {1} = {2}", index, (qr2DefaultKeys)keyId, myGame.players[index].deaths);
                    break;
                case qr2DefaultKeys.TEAM__KEY:
                    gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.players[index].team);
                    Console.WriteLine(" Reporting player[{0}] key {1} = {2}", index, (qr2DefaultKeys)keyId, myGame.players[index].team);
                    break;
                default:
                    gamespyQueryReport.qr2_buffer_addW(ref outBuf, "");
                    break;
            }
        }

        public static void qrTeamKeyCallback(int keyId, int index, ref gamespyQueryReport.qr2Buffer /*qr2_buffer_t*/ outBuf, IntPtr userData)
        {
            Console.WriteLine("\nIn {0} : Reporting team keys", System.Reflection.MethodBase.GetCurrentMethod().Name);

            //check for valid index
            if (index >= myGame.numTeams)
            {
                gamespyQueryReport.qr2_buffer_addW(ref outBuf, "");
                return;
            }
            switch ((qr2DefaultKeys)keyId)
            {
                case qr2DefaultKeys.TEAM_T_KEY:
                    gamespyQueryReport.qr2_buffer_addW(ref outBuf , myGame.teams[index].name);
                    Console.WriteLine("Reporting team[{0}] key {1} = {2}", index, (qr2DefaultKeys)keyId, myGame.teams[index].name);
                    break;
                case qr2DefaultKeys.SCORE_T_KEY:
                    gamespyQueryReport.qr2_buffer_add_int(ref outBuf , myGame.teams[index].score);
                    Console.WriteLine("Reporting team[{0}] key {1} = {2}", index, (qr2DefaultKeys)keyId, myGame.teams[index].score);
                    break;
                default:
                    gamespyQueryReport.qr2_buffer_addW(ref outBuf, "");
                    break;
            }
        }

        public static void qrKeyListCallback(QueryReportKeyType keyType, ref gamespyQueryReport.qr2Keybuffer /*qr2_keybuffer_t*/ keyBuffer, IntPtr userData)
        {
            Console.Write("\nIn {0} : ", System.Reflection.MethodBase.GetCurrentMethod().Name);

            Console.WriteLine("Reporting keylist for {0}", keyType);

            //need to add all the keys we support
            switch (keyType)
            {
                case QueryReportKeyType.key_server:
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.HOSTNAME_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.HOSTPORT_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.MAPNAME_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.GAMETYPE_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.NUMPLAYERS_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.NUMTEAMS_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.MAXPLAYERS_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.TEAMPLAY_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.FRAGLIMIT_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.TIMELIMIT_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, GRAVITY_KEY); //a custom key
                    break;
                case QueryReportKeyType.key_player:
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.PLAYER__KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.SCORE__KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.DEATHS__KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.TEAM__KEY);
                    break;
                case QueryReportKeyType.key_team:
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.TEAM_T_KEY);
                    gamespyQueryReport.qr2_keybuffer_add(ref keyBuffer, (int)qr2DefaultKeys.SCORE_T_KEY);
                    break;
                default: break;
            }
        }

        public static int qrCountCallback(QueryReportKeyType keyType, IntPtr userData)
        {
            Console.Write("\nIn {0} : ",System.Reflection.MethodBase.GetCurrentMethod().Name);
            Console.Write("Reporting {0}  = ", keyType); 
            
            if (keyType == QueryReportKeyType.key_player)
            {
                Console.WriteLine("{0} players", myGame.numPlayers);
                return myGame.numPlayers;
            }
            else if (keyType == QueryReportKeyType.key_team)
            {
                Console.WriteLine("{0} teams", myGame.numTeams);
                return myGame.numTeams;
            }
            else 
                return 0;
        }

        public static void qrAddErrorCallback(QueryReportResult /*qr2_error_t*/ error, String errMsg, IntPtr userData)
        {
            Console.WriteLine("Error adding server: {0}, {1}", error, errMsg);
        }

        public static void qrHostRegisteredCallback(IntPtr userData)
        {
            Console.Write("\nIn {0} : ", System.Reflection.MethodBase.GetCurrentMethod().Name);

            Console.WriteLine("Server listed on Master Server");
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

        public void Run()
        {

            string gamename = "gmtest";
            string secretKey = "HA6zkS";

            Console.WriteLine("\n------------ Query and Report (QR2) C# sample ------------------------\n");

            if (CheckServices(gamename) != GSIACResult.GSIACAvailable)
            {
                Console.WriteLine("{0}: Online services are unavailable now.", gamename);
                return;
            }
#if GSI_COMMON_DEBUG
            StringBuilder debugFileName = new StringBuilder("Debug_QR2");
            debugFileName.Append(".log");

            gamespyCommonDebug.gsOpenDebugFile(debugFileName.ToString());
            gamespyCommonDebug.gsSetDebugLevel(GSIDebugCategory.GSIDebugCat_All, GSIDebugType.GSIDebugType_All, GSIDebugLevel.Hardcore);
#endif
            string customKey = "gravity";
            IntPtr customKeyPtr = new IntPtr();
            customKeyPtr = Marshal.StringToHGlobalUni(customKey);
            gamespyQueryReport.qr2_register_keyW(GRAVITY_KEY, customKeyPtr);

            gamespyQueryReport.QRServerKeyCallback serverKeyCb = new gamespyQueryReport.QRServerKeyCallback(qrServerKeyCallback);
            gamespyQueryReport.QRPlayerTeamKeyCallback playerKeyCb = new gamespyQueryReport.QRPlayerTeamKeyCallback(qrPlayerKeyCallback);
            gamespyQueryReport.QRPlayerTeamKeyCallback teamKeyCb = new gamespyQueryReport.QRPlayerTeamKeyCallback(qrTeamKeyCallback);

            gamespyQueryReport.QRKeyListCallback keyListCb = new gamespyQueryReport.QRKeyListCallback(qrKeyListCallback);
            gamespyQueryReport.QRCountCallback countCb      = new gamespyQueryReport.QRCountCallback(qrCountCallback);
            gamespyQueryReport.QRAddErrorCallback addErrorCb = new gamespyQueryReport.QRAddErrorCallback(qrAddErrorCallback);
            gamespyQueryReport.QRHostRegisteredCallback hostRegisteredCb = new gamespyQueryReport.QRHostRegisteredCallback(qrHostRegisteredCallback);
            
            Console.WriteLine("qr2_initW()");
            QueryReportResult result = gamespyQueryReport.qr2_initW(  IntPtr.Zero, 
                                                                    null, 
                                                                    12345, // baseport
                                                                    gamename, 
                                                                    secretKey, 
                                                                    1,   // isPublic true 
                                                                    1,   // natNegoatiate true
                                                                    serverKeyCb,
                                                                    playerKeyCb,
                                                                    teamKeyCb,
                                                                    keyListCb,
                                                                    countCb,
                                                                    addErrorCb,
                                                                    IntPtr.Zero);
            
            gamespyQueryReport.qr2_register_hostregistered_callback(IntPtr.Zero,hostRegisteredCb);
       
            double startTimeInMs = DateTime.Now.TimeOfDay.TotalMilliseconds;
            bool sendUpdate = true;
            int  listTimeout = 120000;
            while ((DateTime.Now.TimeOfDay.TotalMilliseconds - startTimeInMs) < listTimeout) //60000)
            {
               gamespyQueryReport.qr2_think(IntPtr.Zero);
               Thread.Sleep(10);
               if (((DateTime.Now.TimeOfDay.TotalMilliseconds - startTimeInMs) >= listTimeout/2 ) && sendUpdate)
               {
                    sendUpdate = false;
                    myGame.mapName = "bridge";
                    Console.WriteLine("qr2_send_statechanged: mapName = {0}", myGame.mapName);
                    gamespyQueryReport.qr2_send_statechanged(IntPtr.Zero);
               }
            }
            
            Console.WriteLine("\n------------ QR2 C# Sample Completed------------------------\n");
            Console.WriteLine("Press Enter Continue....");
            Console.ReadLine();

        }
    }
}