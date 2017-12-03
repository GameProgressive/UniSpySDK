///////////////////////////////////////////////////////////////////////////////
// File:	gamespyAtlas.cs
// SDK:		GameSpy ATLAS Competition SDK C# Wrapper
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
using Gamespy.Auth;

namespace Gamespy
{
    namespace Atlas
    {
        class gamespyAtlas
        {

	        //Defines
	        ///////////////////////////////////////////////////////////////////////////////
	        // Set this to define memory settings for the SDK

	        // The initial (or fixed, for static memory) report buffer size
            public const int SC_REPORT_BUFFER_BYTES = 65536;

	        // URL's for ATLAS services.
            public const int SC_SERVICE_MAX_URL_LEN = 128;

	        // Session GUID size - must match backend
            public const int SC_AUTHDATA_SIZE        = 16;
            public const int SC_SESSION_GUID_SIZE    = 40;
            public const int SC_CONNECTION_GUID_SIZE = 40;

	        // convert the 40 byte string guid into an int, 2 shorts and 8 bytes
            public const int SC_GUID_BINARY_SIZE = 16; 

	        // Limit to the number of teams
            public const int SC_MAX_NUM_TEAMS = 64;

	        // OPTIONS flags - first two bits reserved for authoritative / final flags
            public const int SC_OPTIONS_NONE  = 0;

	        // These length values come from the corresponding ATLAS DB column definitions.
            public const int SC_CATEGORY_MAX_LENGTH     = 64;
            public const int SC_STAT_NAME_MAX_LENGTH    = 128;

            public static byte[] scServiceURL = new byte[SC_SERVICE_MAX_URL_LEN];
	        public static byte[] scGameConfigDataServiceURL = new byte [SC_SERVICE_MAX_URL_LEN];
	        public static byte[] scStatsDataServiceURL = new byte[SC_SERVICE_MAX_URL_LEN];


	        ///////////////////////////////////////////////////////////////////////////////
	        ///////////////////////////////////////////////////////////////////////////////
	        // Data types

	        IntPtr SCInterfacePtr;
	        IntPtr SCReportPtr;
            IntPtr SCQueryParameterListPtr;

	        private static byte[] SCHiddenData = new byte[64];

            [StructLayout(LayoutKind.Sequential)]
            public struct SCQueryParameter
            {
                [MarshalAs(UnmanagedType.LPTStr)]
	            public string  mName;
                [MarshalAs(UnmanagedType.LPTStr)]
	            public string  mValue;
            } ;

            [StructLayout(LayoutKind.Sequential)]
            public struct SCQueryParameterList
            {
	            public UInt32 mCount;
	            public UInt32 mNextSlot;        
	            public IntPtr mQueryParams; //SCQueryParameter *
            };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCStat
	        {
                [MarshalAs(UnmanagedType.LPTStr)]
		        public string	        mName;		// Name for this stat 
                public SCStatDataType   mStatType;	// Type for this stat
                [MarshalAs(UnmanagedType.LPTStr)]
                public string	        mValue;		    // Value for this stat
	        };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCPlayer
	        {
		        public UInt32 mProfileId;	// Profile ID associated with the current player
		        public UInt32 mStatsCount;	// Stats count for mStats below
		        public IntPtr mStats;		// SCStat *:Array of SCStat objects
	        };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCTeam
	        {
		        public UInt32 mTeamId;		// Team ID associated with the current player
		        public UInt32 mStatsCount;	// Stats count for mStats below
                public IntPtr mStats;		// SCStat *: Array of SCStat objects
	        };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCGameStatsCategory
	        {
		        [MarshalAs(UnmanagedType.LPTStr)]
                public string mName;		// Name of the category
		        public UInt32 mStatsCount;	// Player count for mPlayers below
		        public IntPtr mStats;		// SCStat *: Array of SCPlayer objects
	        };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCPlayerStatsCategory
	        {
                [MarshalAs(UnmanagedType.LPTStr)]
		        public string   mName;		    // Name of the category
		        public UInt32   mPlayersCount;	// Player count for mPlayers below
		        public IntPtr   mPlayers;		// SCPlayer *: Array of SCPlayer objects
	        };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCTeamStatsCategory
	        {
                [MarshalAs(UnmanagedType.LPTStr)]
		        public string  mName;		// Name of the category
		        public UInt32  mTeamsCount;	// Player count for mPlayers below
		        public IntPtr  mTeams;		// SCTeam *: Array of SCPlayer objects
	        };
        	
	        [StructLayout(LayoutKind.Sequential)]
            public struct SCGameStatsQueryResponse
	        {
		        public UInt32  mCategoriesCount;
		        public IntPtr  mCategories;   //SCGameStatsCategory *
	        };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCPlayerStatsQueryResponse
	        {
		        public UInt32  mCategoriesCount;
		        public IntPtr  mCategories; //SCPlayerStatsCategory *
	        };

	        [StructLayout(LayoutKind.Sequential)]
            public struct SCTeamStatsQueryResponse
	        {
		        public UInt32  mCategoriesCount;
		        public IntPtr  mCategories; //SCTeamStatsCategory *
	        };

	        // Callbacks
            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
	        public delegate void SCCheckBanListCallback
            (
                 IntPtr       theInterface, // const SCInterfacePtr
		         GHTTPResult  httpResult,
		         SCResult     result,
		         IntPtr       userData,
		         int          resultProfileId,
		         int          resultPlatformId,
		         bool         resultProfileBannedHost
            );

	        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SCCreateSessionCallback
            (
                IntPtr      theInterface,   //const SCInterfacePtr
	            GHTTPResult httpResult,
		        SCResult    result,
		        IntPtr      userData
            );

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SCSetReportIntentionCallback
            (
                IntPtr      theInterface, //const SCInterfacePtr
		        GHTTPResult httpResult,
		        SCResult    result,
		        IntPtr      userData
            );

	        [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
            public delegate void SCSubmitReportCallback
            (
                IntPtr      theInterface, //const SCInterfacePtr
		        GHTTPResult httpResult,
		        SCResult    result,
		        IntPtr      userData
            );

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
	        public delegate void    SCTeamStatsQueryCallback
            (
                IntPtr      theInterface, //const SCInterfacePtr
		        GHTTPResult httpResult,
		        SCResult    result,
		        [MarshalAs(UnmanagedType.LPTStr)]
                string      msg,
		        int         msgLen,
		        IntPtr      response, // SCTeamStatsQueryResponse *
		        IntPtr      userData
            );

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
	        public delegate void SCGameStatsQueryCallback
            (
                IntPtr      theInterface, //const SCInterfacePtr
		        GHTTPResult httpResult,
		        SCResult    result,
		        [MarshalAs(UnmanagedType.LPTStr)]
                string      msg,
		        int         msgLen,
		        IntPtr      response, //SCGameStatsQueryResponse * 
		        IntPtr      userData
            );

            [UnmanagedFunctionPointerAttribute(CallingConvention.Cdecl)]
	        public delegate void SCPlayerStatsQueryCallback
            (
                IntPtr      theInterface, //const SCInterfacePtr
		        GHTTPResult httpResult,
		        SCResult    result,
		        [MarshalAs(UnmanagedType.LPTStr)]
                string      msg,
		        int         msgLen,
		        IntPtr      response, //SCPlayerStatsQueryResponse *  
		        IntPtr      userData
            );
	        // Atlas API
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scInitialize
            (
                int gameId,
	            ref IntPtr theInterfaceOut  //SCInterfacePtr
             );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scShutdown(IntPtr theInterface); //SCInterfacePtr

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scThink(IntPtr theInterface);    //SCInterfacePtr

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scCheckBanList
            (
                IntPtr      theInterface, //SCInterfacePtr
		        ref gamespyAuth.GSLoginCertificate certificate,         // GSLoginCertificate *
                ref gamespyAuth.GSLoginCertificatePrivate privateData,  //GSLoginPrivateData *
		        UInt32      hostProfileId,
		        SCPlatform  hostPlatform,
		        SCCheckBanListCallback  callback,
		        Int32       timeoutMs,
		        IntPtr      userData);

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scCreateSession
            (
                IntPtr  theInterface,    //SCInterfacePtr
                ref gamespyAuth.GSLoginCertificate certificate,         // GSLoginCertificate *
                ref gamespyAuth.GSLoginCertificatePrivate privateData, //GSLoginPrivateData *
		        SCCreateSessionCallback    callback,
		        Int32   timeoutMs,
		        IntPtr  userData
            );


            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scCreateMatchlessSession
            (
                IntPtr                  theInterface, //SCInterfacePtr
                ref gamespyAuth.GSLoginCertificate certificate,         // GSLoginCertificate *
		        ref gamespyAuth.GSLoginCertificatePrivate privateData,  //GSLoginPrivateData *
		        SCCreateSessionCallback callback,
		        Int32                   timeoutMs,
		        IntPtr                  userData
            );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scSetReportIntention
            (
                IntPtr  theInterface, //const SCInterfacePtr
		        byte[]  theConnectionId,
		        bool    isAuthoritative,
		        ref gamespyAuth.GSLoginCertificate certificate,         // GSLoginCertificate *
		        ref gamespyAuth.GSLoginCertificatePrivate privateData,  //GSLoginPrivateData *
		        SCSetReportIntentionCallback callback,
		        Int32   timeoutMs,
		        IntPtr  userData
            );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scSubmitReport      
            (
                IntPtr  theInterface,   //const SCInterfacePtr
		        IntPtr  theReport,      // SCReportPtr
		        bool    isAuthoritative,
		        ref gamespyAuth.GSLoginCertificate certificate,         // GSLoginCertificate *
		        ref gamespyAuth.GSLoginCertificatePrivate privateData,    //GSLoginPrivateData *
		        SCSubmitReportCallback callback,
		        Int32   timeoutMs,
		        IntPtr  userData
            );

        	 
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scSetSessionId
            (
                IntPtr  theInterface, //const SCInterfacePtr 
                byte[]  theSessionId
            );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr scGetSessionId(IntPtr theInterface); // const SCInterfacePtr ; returns byte[]

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr scGetConnectionId(IntPtr theInterface); //const SCInterfacePtr; returns byte[]

	        // Report generation functions

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scCreateReport
            (
                IntPtr theInterface, // const SCInterfacePtr
		        UInt32 theRulesetVersion, 
		        UInt32 thePlayerCount, 
		        UInt32 theTeamCount, 
		        ref IntPtr theReportOut // SCReportPtr *
            );
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportBeginGlobalData(IntPtr theReportData); //SCReportPtr
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportBeginPlayerData(IntPtr theReportData); //SCReportPtr
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportBeginTeamData(IntPtr theReportData); //SCReportPtr
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportBeginNewPlayer(IntPtr theReportData); // SCReportPtr
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportSetPlayerData 
            (
                IntPtr          theReport, // SCReportPtr
		        UInt32          thePlayerIndex,
		        byte[]          thePlayerConnectionId,
		        UInt32          thePlayerTeamId,
		        SCGameResult    result,
		        UInt32          theProfileId,
		        ref gamespyAuth.GSLoginCertificate certificate, // GSLoginCertificate *
                byte[]          theAuthData
            );
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportBeginNewTeam(IntPtr theReportData); //SCReportPtr
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportSetTeamData 
            (
                IntPtr  theReport, //SCReportPtr
		        UInt32  theTeamId,
		        SCGameResult result
            );
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportEnd
            (
                IntPtr theReport, //SCReportPtr
		        bool    isAuth, 
		        SCGameStatus theStatus
            );
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportSetAsMatchless(IntPtr theReport); //SCReportPtr

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportAddIntValue
            (
                IntPtr  theReportData, //SCReportPtr
		        UInt16 theKeyId,
		        Int32 theValue
            );
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportAddInt64Value
            (
                IntPtr  theReportData, //SCReportPtr
		        UInt16  theKeyId,
		        Int64   theValue
            );
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportAddShortValue
            (
                IntPtr  theReportData, //SCReportPtr
		        UInt16  theKeyId,
		        Int16   theValue
            );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportAddByteValue
            (
                IntPtr  theReportData, //SCReportPtr
		        UInt16  theKeyId,
		        byte    theValue
            );

        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportAddFloatValue
            (
                IntPtr  theReportData, //SCReportPtr
		        UInt16  theKeyId,
		        float   theValue
            );
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scReportAddStringValue
            (
                IntPtr  theReportData, //SCReportPtr
		        UInt16  theKeyId,
                [MarshalAs(UnmanagedType.LPTStr)]
		        string  theValue
            );
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scDestroyReport(IntPtr theReport); //SCReportPtr

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scCreateQueryParameterList
            (
                ref IntPtr queryParams,  //SCQueryParameterListPtr * 
                UInt32 queryParamsCount
            );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scAddQueryParameterToList
            (
                IntPtr queryParams, //SCQueryParameterListPtr 
                [MarshalAs(UnmanagedType.LPTStr)]
                string name, 
                [MarshalAs(UnmanagedType.LPTStr)]
                string value
            );
                    
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scDestroyQueryParameterList(ref IntPtr queryParams); //SCQueryParameterListPtr *
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scDestroyGameStatsQueryResponse(IntPtr response); //SCGameStatsQueryResponse **
        	
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scDestroyPlayerStatsQueryResponse(IntPtr response); //SCPlayerStatsQueryResponse **
            
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scDestroyTeamStatsQueryResponse(IntPtr response);// SCTeamStatsQueryResponse **

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scRunGameStatsQuery
            (
                IntPtr  interfacePtr, //SCInterfacePtr
		        ref gamespyAuth.GSLoginCertificate certificate,           // GSLoginCertificate *
		        ref gamespyAuth.GSLoginCertificatePrivate privateData,    //GSLoginPrivateData *
		        int     ruleSetVersion,
		        byte[]  queryId,
		        IntPtr  queryParameters, //SCQueryParameterListPtr
		        SCGameStatsQueryCallback callback,
		        IntPtr  userData
            );
              
            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scRunPlayerStatsQuery    
            (
                IntPtr interfacePtr, //SCInterfacePtr
		        ref gamespyAuth.GSLoginCertificate certificate,         // GSLoginCertificate *
		        ref gamespyAuth.GSLoginCertificatePrivate privateData,  //GSLoginPrivateData *
		        int     ruleSetVersion,
		        byte[]  queryId,
		        IntPtr  queryParameters, //SCQueryParameterListPtr
		        SCPlayerStatsQueryCallback		callback,
		        IntPtr  userData
            );

            [DllImport("gamespy.dll", CallingConvention = CallingConvention.Cdecl)]
            public static extern SCResult scRunTeamStatsQuery
            (
                IntPtr  theInterface, //SCInterfacePtr
		        ref gamespyAuth.GSLoginCertificate certificate,         // GSLoginCertificate *
		        ref gamespyAuth.GSLoginCertificatePrivate privateData,  //GSLoginPrivateData *
		        int     ruleSetVersion,
		        byte[]  queryId,
		        IntPtr  queryParameters, //SCQueryParameterListPtr
		        SCTeamStatsQueryCallback    callback,
		        IntPtr  userData
            );

        }
    }
}
