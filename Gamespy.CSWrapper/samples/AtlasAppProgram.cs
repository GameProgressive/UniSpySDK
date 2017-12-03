///////////////////////////////////////////////////////////////////////////////
// File:	AtlasAppProgram.cs
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.using System;

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading;
using Gamespy;
using Gamespy.Auth;
using Gamespy.Common;
using Gamespy.Http;
using Gamespy.Atlas;

namespace gamespySample
{
    // This is an auxilary base class for the Atlas Key & Value pairs
    public class AtlasKey
    {
        public UInt16 keyId;
        public AtlasKey(UInt16 _keyId)
        {
            keyId = _keyId;
        }
        public virtual SCResult ToReport(IntPtr atlasReport) 
        { 
            return SCResult.SCResult_INVALID_DATATYPE ;
        }   
    }
    // Byte Atlas Key type
    public class AtlasKeyByte : AtlasKey
    {
        public Byte value;
        public AtlasKeyByte(UInt16 _keyId, Byte _value)
            : base(_keyId)
        {
            value = _value;
        }
        public override SCResult  ToReport(IntPtr report)
        {
 	        return gamespyAtlas.scReportAddByteValue(report, keyId, value);
        }
    }

    // Int32 Atlas Key type
    public class AtlasKeyInt32 : AtlasKey
    {
        public Int32 value;
        public AtlasKeyInt32(UInt16 _keyId, Int32 _value)
            : base(_keyId)
        {
            value = _value;
        }
        public override SCResult  ToReport(IntPtr report)
        {
 	        return gamespyAtlas.scReportAddIntValue(report, keyId, value);
        }
    }

    // Int64 Atlas Key type
    public class AtlasKeyInt64 : AtlasKey
    {
        public Int64 value;
        public AtlasKeyInt64(UInt16 _keyId, Int64 _value)
            : base(_keyId)
        {
            value = _value;
        }
        public override SCResult  ToReport(IntPtr report)
        {
 	        return gamespyAtlas.scReportAddInt64Value(report, keyId, value);
        }
    }

    // String Atlas Key type
    public class AtlasKeyString : AtlasKey
    {
        public String value;
        public AtlasKeyString(UInt16 _keyId, String _value)
            : base(_keyId)
        {
            value = _value;
        }
        public override SCResult  ToReport(IntPtr report)
        {
 	        return gamespyAtlas.scReportAddStringValue(report, keyId, value);
        }
    }

    // A container class to keep a list of Atlas Keys
    public class AtlasKeyList
    {
        public List<AtlasKey> keyList;
        public AtlasKeyList()
        {
            keyList = new List<AtlasKey> ();
            keyList.Clear();
        }

        public void addKey(UInt16 id, Int32 value)
        {
            AtlasKeyInt32 key = new AtlasKeyInt32(id, value);
            keyList.Add(key);
        }

        public void addKey(UInt16 id, Int64 value)
        {
            AtlasKeyInt64 key = new AtlasKeyInt64(id, value);
            keyList.Add(key);
        }

        public void addKey(UInt16 id, String value)
        {
            AtlasKeyString key = new AtlasKeyString(id, value);
            keyList.Add(key);
        }

        public SCResult generateReport(IntPtr report)
        {
            SCResult result = SCResult.SCResult_NO_ERROR;

            for (int i = 0; i < keyList.Count; i++)
            {
                result = keyList[i].ToReport(report);
                if (result != SCResult.SCResult_NO_ERROR)
                {
                    return result;
                }
            }
            return result;
        }
    }


    // The Sample Atlas Application
    class AtlasAppProgram
    {
        // The following is a list of data values to initialize Atlas.
        private static string   _AccessKey = "39cf9714e24f421c8ca07b9bcb36c0f5";
        private static string   _SecretKey = "Zc0eM6";
        private static Int32    _GameId     = 1649;
        private static Int32    _PartnerId  = 0;
        private static Int32    _NameSpace  = 1;
        private static bool     respReceived= false;
        private static string   _NickName   = "sctest01";
        private static string   _Email      = "sctest@gamespy.com";
        private static string   _Password   = "gspy";
        private static string   _CdKey      = "";
        private static string   _GameName   = "atlasSamples";
        private static gamespyAuth.GSLoginCertificate gameCertificate;
        private static gamespyAuth.GSLoginCertificatePrivate gamePrivateData;
        private static bool     authenticated = false;

        // The following is specific data for this particular application
        private static UInt32 _AtlasRuleSetVersion = 1;

        private enum AtlasKeys
        {
	        ATLAS_KEY_RACE_TIME = 1,        // [TYPE: int64] [DESC: This single key facilitates all the race match statistics]
	        ATLAS_KEY_USED_VEHICLE_1 = 2,   // [TYPE: byte] [DESC: For Metrics -- Indicates "Vehicle 1" was used]
	        ATLAS_KEY_USED_VEHICLE_2 = 3,   // [TYPE: byte] [DESC: For Metrics -- Indicates "Vehicle 2" was used]
	        ATLAS_KEY_USED_VEHICLE_3 = 4,   // [TYPE: byte] [DESC: For Metrics -- Indicates "Vehicle 3" was used]
	        ATLAS_KEY_USED_TRACK_1 = 5,     // [TYPE: byte] [DESC: For Metrics -- Indicates "Track 1" was used]
	        ATLAS_KEY_USED_TRACK_2 = 6,     // [TYPE: byte] [DESC: For Metrics -- Indicates "Track 2" was used]
	        ATLAS_KEY_USED_TRACK_3 = 7      // [TYPE: byte] [DESC: For Metrics -- Indicates "Track 3" was used]
        };


        // Callback Implementations

        public static void loginCallback( GHTTPResult httpResult, IntPtr theResponse, IntPtr userData)
        {

            respReceived = true;

            if (httpResult != GHTTPResult.GHTTPSuccess)
	        {
		        Console.WriteLine("Failed on player login, HTTP error: {0}", httpResult);
                return;		        
	        }
            
            gamespyAuth.WSLoginResponse  resp = (gamespyAuth.WSLoginResponse)Marshal.PtrToStructure(theResponse, typeof(gamespyAuth.WSLoginResponse));
            
            if (resp.mLoginResult != WSLoginValue.WSLogin_Success)
	        {
		        Console.WriteLine("Failed on player login, Login result: {0}", resp.mLoginResult);
	        }
	        else
	        {
                Console.WriteLine("Player with profile Id {0} logged in.", resp.mCertificate.mProfileId);
                gameCertificate = resp.mCertificate;
                gamePrivateData = resp.mPrivateData;
                authenticated = true;
            }
        }

        public static void competitionCreateSessionCallback
         (
             IntPtr theInterface,   //const SCInterfacePtr
             GHTTPResult httpResult,
             SCResult result,
             IntPtr userData
         )
        {
            respReceived = true;
            if (result == SCResult.SCResult_NO_ERROR)
            {

                string sessionId = Marshal.PtrToStringAnsi(gamespyAtlas.scGetSessionId(theInterface));
                string connectionId = Marshal.PtrToStringAnsi(gamespyAtlas.scGetConnectionId(theInterface));
                Console.WriteLine("SUCCESS: CreateSession");
                Console.WriteLine("Session Id {0}", sessionId);
                Console.WriteLine("Connection Id {0}", connectionId);
            }
            else
            {
                Console.WriteLine("FAILURE: {0}", result);
            }
        }

        public static void competitionSetReportIntentionCallback(
            IntPtr theInterface,   //const SCInterfacePtr
            GHTTPResult httpResult,
            SCResult result,
            IntPtr userData
        )
        {
            respReceived = true;
            if (result == SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("SUCCESS: Set Report Intention");
            }
            else
            {
                Console.WriteLine("FAILURE: {0}", result);
            }
        }

        
        public static void competitionSubmitReportCallback(
            IntPtr theInterface,   //const SCInterfacePtr
            GHTTPResult httpResult,
            SCResult result,
            IntPtr userData
        )
        {
            Console.Write("\nIn {0} : ", System.Reflection.MethodBase.GetCurrentMethod().Name);

            respReceived = true;
            if (result == SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("SUCCESS: Submit Report ");
            }
            else
            {
                Console.WriteLine("FAILURE: {0}", result);
            }
        }

        /// Atlas Application code
        
        // Atlas related declarations 
        
        public static IntPtr atlasInterface = new IntPtr();
        public static IntPtr atlasReport = new IntPtr();
 
        // Application methods
        public static bool Initialize()
        {
            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
            byte[] accessKey = encoding.GetBytes(_AccessKey);
            byte[] secretKey = encoding.GetBytes(_SecretKey);

            // Now initialize the common core
            gamespyCommon.gsCoreInitialize();
            gamespyAuth.wsSetGameCredentials(accessKey, _GameId, secretKey);
            gamespyAuth.WSLoginCallback myLoginCallback = new gamespyAuth.WSLoginCallback(loginCallback);
            
            respReceived = false;
            gamespyAuth.wsLoginProfile(_GameId, _PartnerId, _NameSpace, _NickName, _Email, _Password, _CdKey, myLoginCallback, IntPtr.Zero);
            
            while (respReceived == false)
	        {
                gamespyCommon.gsCoreThink(0);
                Thread.Sleep(new TimeSpan(100000));
	        }

            if (authenticated)
            {
                //Now Initialize Atlas
                SCResult atlasResult = gamespyAtlas.scInitialize(_GameId, ref atlasInterface);
                if (atlasResult != SCResult.SCResult_NO_ERROR)
                {
                    Console.WriteLine("Atlas initialization failed {0} ", atlasResult);
                    return false;
                }
                return true;
            }
            else
                return false;
        }
         
        public static bool Terminate()
        {
            gamespyAtlas.scShutdown(atlasInterface);
            gamespyHttp.ghttpCleanup();
            gamespyCommon.gsCoreShutdown();
            while (gamespyCommon.gsCoreIsShutdown() != GSCoreState.GSCoreState_SHUTDOWN_COMPLETE)
            {
                gamespyCommon.gsCoreThink(0);
                if (gamespyCommon.gsCoreIsShutdown() != GSCoreState.GSCoreState_SHUTDOWN_COMPLETE)
                    Thread.Sleep(new TimeSpan(100000));
            }
            return true;
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
	        } while(aResult == GSIACResult.GSIACWaiting );

	        // Check the result
	        switch(aResult)
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
        
        private void WaitForResponse(IntPtr atlasInterface)
        {
            respReceived = false;
            
            // WaitforResponse
            while (respReceived == false)
            {
                gamespyAtlas.scThink(atlasInterface);
                if (respReceived == false) Thread.Sleep(new TimeSpan(100000));
            }
        }
        
        public void initGameKeys(AtlasKeyList gameKeys)
        {
            Int64 raceTime = DateTime.Now.ToBinary();
            gameKeys.addKey((UInt16) AtlasKeys.ATLAS_KEY_USED_TRACK_1, 1);
            gameKeys.addKey((UInt16)AtlasKeys.ATLAS_KEY_USED_VEHICLE_1, 1);
            gameKeys.addKey((UInt16)AtlasKeys.ATLAS_KEY_RACE_TIME, raceTime);
        }

        public void initPlayerKeys(AtlasKeyList playerKeys)
        {
            Int64 raceTime = DateTime.Now.ToBinary();
            playerKeys.addKey((UInt16)AtlasKeys.ATLAS_KEY_USED_TRACK_1, 1);
            playerKeys.addKey((UInt16)AtlasKeys.ATLAS_KEY_USED_VEHICLE_1, 1);
            playerKeys.addKey((UInt16)AtlasKeys.ATLAS_KEY_RACE_TIME, raceTime);
        }
        public void Run()
        {
            AtlasAppProgram atlasapp = new AtlasAppProgram();
            string gamename = _GameName;
            SCResult result = SCResult.SCResult_NO_ERROR;
            Console.WriteLine("\n------------ Atlas C# Sample --------------------------");

            if (atlasapp.CheckServices(gamename) != GSIACResult.GSIACAvailable)
            {
                Console.WriteLine("{0}: Online services are unavailable now.\n", gamename);
                return;
            }
            
#if GSI_COMMON_DEBUG            
            gamespyCommonDebug.gsOpenDebugFile("Debug_Atlas.log");
            gamespyCommonDebug.gsSetDebugLevel(GSIDebugCategory.GSIDebugCat_All, GSIDebugType.GSIDebugType_All, GSIDebugLevel.Hardcore);
#endif
            if (Initialize())
            {
                Console.WriteLine("{0}: Initialized.\n", gamename);
            }
            else
            {
                Console.WriteLine("{0}: Failed initialize.\n", gamename);
                Terminate(); 
                Console.WriteLine("------------ Atlas C# Sample Terminated------------------------\n");
                Console.WriteLine("Press AnyKey Continue....");
                Console.ReadLine();

                return;
            }

            // scCreateSession example
            gamespyAtlas.SCCreateSessionCallback createSessionCallback = new gamespyAtlas.SCCreateSessionCallback(competitionCreateSessionCallback);
            
            Console.WriteLine("\nAtlas : Create Session\n");
            result = gamespyAtlas.scCreateSession(atlasInterface, ref gameCertificate, ref gamePrivateData, createSessionCallback, 0, IntPtr.Zero);
            if ( result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Create Session : {0}\n", result);
                return;
            }

            WaitForResponse(atlasInterface);
            Console.WriteLine("Create Session completed. \n");

            gamespyAtlas.SCSetReportIntentionCallback setReportIntentionCallback = new gamespyAtlas.SCSetReportIntentionCallback(competitionSetReportIntentionCallback);
            respReceived = false;
            Console.WriteLine("Set Report Intention\n");
            result = gamespyAtlas.scSetReportIntention(atlasInterface, null, true, ref gameCertificate, ref gamePrivateData, setReportIntentionCallback, 0, IntPtr.Zero);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Set Report Intention : {0}\n", result);
                return;
            }

            while (respReceived == false)
            {
                gamespyAtlas.scThink(atlasInterface);
                if (respReceived == false) Thread.Sleep(new TimeSpan(100000));
            }
            Console.WriteLine("Set Report Intention completed. \n");

            Console.WriteLine("Create Report");
            result = gamespyAtlas.scCreateReport(atlasInterface, _AtlasRuleSetVersion, 1, 0, ref atlasReport);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Create Report : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Report Created succesfully!\n");
            }

            Console.WriteLine("Create Report completed. \n");

            //  Begin the game's (global data) section of the report
	        result = gamespyAtlas.scReportBeginGlobalData(atlasReport);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Report Begin Global Data : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Report Begin Global Data completed succesfully!\n");
            }

            // add the game-level key values

            AtlasKeyList gameKeys = new AtlasKeyList();
            initGameKeys(gameKeys);
            result = gameKeys.generateReport(atlasReport);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Generate Report : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Generate Report completed Succesfully!\n");
            }
            	
            result = gamespyAtlas.scReportBeginPlayerData(atlasReport);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Report Begin Player Data : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Report Begin Player Data completed succesfully!\n");
            }

            // This is the start of a new player 
            result = gamespyAtlas.scReportBeginNewPlayer(atlasReport);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Report Begin New Player Data : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Report Begin New Player Data completed succesfully!\n");
            }
            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
            byte[] connectionId = encoding.GetBytes(Marshal.PtrToStringAnsi(gamespyAtlas.scGetConnectionId(atlasInterface)));
            result = gamespyAtlas.scReportSetPlayerData
                ( 
                    atlasReport, 
                    0, 
                    connectionId, 
                    0,
                    SCGameResult.SCGameResult_WIN, 
                    gameCertificate.mProfileId, 
                    ref gameCertificate, 
                    null
                );
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Set Player Data : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Set Player Data completed succesfully!\n");
            }

            AtlasKeyList playerKeys = new AtlasKeyList();
            initPlayerKeys(playerKeys);
            result = playerKeys.generateReport(atlasReport);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Generate Report : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Generate Report completed succesfully!\n");
            }

            result = gamespyAtlas.scReportBeginTeamData(atlasReport);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Begin Team Data : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Begin Team Data completed succesfully!\n");
            }

            result = gamespyAtlas.scReportEnd(atlasReport, true, SCGameStatus.SCGameStatus_COMPLETE);
            if (result != SCResult.SCResult_NO_ERROR)
            {
                Console.WriteLine("Error in Report End : {0}\n", result);
                return;
            }
            else
            {
                Console.WriteLine("Atlas Report End completed succesfully!\n");
            }
            gamespyAtlas.SCSubmitReportCallback submitReportcallback = new gamespyAtlas.SCSubmitReportCallback(competitionSubmitReportCallback);
            Console.WriteLine("\nAtlas : Set Report Intention\n");
            Console.WriteLine("Submitting a report for the ATLAS session...\n");
            result = gamespyAtlas.scSubmitReport(atlasInterface, atlasReport, true, ref gameCertificate,
                ref gamePrivateData, submitReportcallback, 0, IntPtr.Zero);

            WaitForResponse(atlasInterface);

            Terminate();
            Console.WriteLine("\n------------ Atlas C# Sample Completed------------------------\n");
            Console.WriteLine("Press Enter Continue....");
            Console.ReadLine();

        }
    }
}
