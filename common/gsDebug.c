///////////////////////////////////////////////////////////////////////////////
// File:	gsDebug.c
// SDK:		GameSpy Common
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc.  All rights 
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

#include "gsCommon.h"
#include "gsDebug.h"
//#include <stdio.h>
//#include <stdarg.h>


// THIS FILE ONLY INCLUDED WHEN USING GAMESPY DEBUG FUNCTIONS
// (Putting this above the header includes may cause compiler warnings.)
#ifdef GSI_COMMON_DEBUG

#if defined(_NITRO)
#include "../../common/nitro/screen.h"
#define printf Printf
#define vprintf VPrintf
#endif

#if defined(ANDROID)
#include <android/log.h>
#include <stdarg.h>
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Static debug data
static struct GSIDebugInstance gGSIDebugInstance; // simple singleton "class"

// Line prefixes, e.g. "[ cat][type][ lev] text"
char* gGSIDebugCatStrings[GSIDebugCat_Count] =
{
	" APP", " GP ", "PEER", " QR2", "  SB", "  V2", "  AD", "  NN", "HTTP", "CDKY", "D2G", "BRIG", "AUTH", "SAKE", " CMN", "ATLS", " GT2"
};
char* gGSIDebugTypeStrings[GSIDebugType_Count] =
{
	" NET", "FILE", " MEM", "STAT", "MISC"
};
char* gGSIDebugLevelStrings[GSIDebugLevel_Count] =
{
	"*ERR", "****", "----", "    ", "    ", "    ", "  ->"
};
char* gGSIDebugLevelDescriptionStrings[8] =
{
	"None", "<None+1>", "<Normal>", "<Debug>", "<Verbose>", "<Verbose+1>", "<Verbose+2>", "<Hardcore>"
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// utility to convert bit flag back to base  (e.g. 1<<2 returns 2)
static gsi_u32 gsiDebugLog2(gsi_u32 theInt)
{
	gsi_u32 total = 0;
	while (theInt > 1)
	{
		theInt = theInt >> 1;
		total++;
	}
	return total;
}


// default supplied debug function, will receive debug text
// this is platform specific
static void gsiDebugCallback(GSIDebugCategory category, GSIDebugType type,
						GSIDebugLevel level, const char * format, va_list params)
{
	#if defined(_PSP) || defined(_PSP2)
		// Output line prefix
		vprintf(format, params);
		//gsDebugTTyPrint(string);
	#elif defined(_PS2)
		// Output line prefix
		vprintf(format, params);

	#elif defined(_PS3)
		// Output line prefix
		vprintf(format, params);

	#elif defined(ANDROID)
		static char string[512];
		android_LogPriority priority;
		switch(level)
		{
			case GSIDebugLevel_HotError:
				priority = ANDROID_LOG_ERROR;
				break;
			case GSIDebugLevel_WarmError:
				priority = ANDROID_LOG_ERROR;
				break;
			case GSIDebugLevel_Warning:
				priority = ANDROID_LOG_WARN;
				break;
			case GSIDebugLevel_Notice:
				priority = ANDROID_LOG_INFO;
				break;
			case GSIDebugLevel_Comment:
				priority = ANDROID_LOG_INFO;
				break;
			case GSIDebugLevel_RawDump:
				priority = ANDROID_LOG_DEBUG;
				break;
			case GSIDebugLevel_StackTrace:
				priority = ANDROID_LOG_VERBOSE;
				break;

			default:
				priority = ANDROID_LOG_VERBOSE;
				break;
		}
		vsprintf(string,format,params);
		__android_log_print(priority, "GAMESPY", "[%s] [%s] %s",
				gGSIDebugCatStrings[category],gGSIDebugTypeStrings[type],string);

	#elif defined(_WIN32)
		static char string[256];
		vsprintf(string, format, params); 			
		OutputDebugStringA(string);

	#elif defined(_LINUX) || defined(_MACOSX) || defined (_IPHONE)
		//static char    string[256];
		//vsprintf(string, format, params); 			
		vprintf(format, params);
	#elif defined(_NITRO)
		VPrintf(format, params);
	#elif defined(_REVOLUTION)
		static char string[256];
		vsprintf(string, format, params);
		OSReport(string);
	#else
		va_list argptr;
		static char    string[256];
		va_start(argptr, format);
		vsprintf(string, format, argptr); 
		va_end(argptr);
		gsDebugTTyPrint(string);
	#endif
	
	GSI_UNUSED(category);
	GSI_UNUSED(type);
	GSI_UNUSED(level);
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// process debug output
void gsDebugVaList(GSIDebugCategory theCat, GSIDebugType theType, 
					  GSIDebugLevel theLevel, const char* theTokenStr, 
					  va_list theParamList)
{
	// Retrieve the current debug level
	GSIDebugLevel aCurLevel;

	// Verify Parameters
	GS_ASSERT(theCat   <= GSIDebugCat_Count);
	GS_ASSERT(theType  <= GSIDebugType_Count);
	GS_ASSERT(theLevel <= (1<<GSIDebugLevel_Count));
	GS_ASSERT(theTokenStr);

	// Make thread safe
	if (gGSIDebugInstance.mInitialized == 0)
	{
		// Warning: Slight race condition risk here the first time
		//          gsDebug functions are used.
		//          The risk is minimal since you usually set
		//          debug levels and targets at program startup
		gGSIDebugInstance.mInitialized = 1;
		gsiInitializeCriticalSection(&gGSIDebugInstance.mDebugCrit);
	}

	gsiEnterCriticalSection(&gGSIDebugInstance.mDebugCrit);

	// Are we currently logging this type and level?
	aCurLevel = gGSIDebugInstance.mGSIDebugLevel[theCat][theType];
	if (aCurLevel & theLevel) // check the flag
	{
#if !defined(_NITRO)
		// Output line prefix
		if (gGSIDebugInstance.mGSIDebugFile)
		{
			fprintf(gGSIDebugInstance.mGSIDebugFile, "[%s][%s][%s] ", 
				gGSIDebugCatStrings[theCat], 
				gGSIDebugTypeStrings[theType],
				gGSIDebugLevelStrings[gsiDebugLog2(theLevel)]);
			
			// Output to file
			vfprintf(gGSIDebugInstance.mGSIDebugFile, theTokenStr, 
				theParamList);
		}
#endif
		// Output to developer function if provided
		if (gGSIDebugInstance.mDebugCallback != NULL)
		{
			(*gGSIDebugInstance.mDebugCallback)(theCat, theType, theLevel,
			                                     theTokenStr, theParamList);
		}
		else
		{
			gsiDebugCallback(theCat, theType, theLevel,
			                                     theTokenStr, theParamList);
		}
	}
	
	gsiLeaveCriticalSection(&gGSIDebugInstance.mDebugCrit);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// process debug output
void gsDebugFormat(GSIDebugCategory theCat, GSIDebugType theType, 
					  GSIDebugLevel theLevel, const char* theTokenStr, 
					  ...)
{
	va_list aParameterList;

	// Verify Parameters
	GS_ASSERT(theCat   <= GSIDebugCat_Count);
	GS_ASSERT(theType  <= GSIDebugType_Count);
	GS_ASSERT(theLevel <= (1<<GSIDebugLevel_Count));
	GS_ASSERT(theTokenStr);

	// Find start of var arg list
	va_start(aParameterList, theTokenStr);
	
	// Pass to VA version
	gsDebugVaList(theCat, theType, theLevel, theTokenStr, aParameterList);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Converts binary buffer to memory view form:
//    0000 0000 0000 0000 0000 0000 0000 0000  ................
static void HexEncode16(const char* theInStream, char* theOutStream, 
				 unsigned int theInLen)
{
	const int  aRowWidth     = 64;     // width of the output
	const char aReplaceChar  = '.';    // Replace non print characters
	const int  aTextOffSet   = 41;     // text comes after hex bytes
	char* aTextOutStream = (theOutStream+aTextOffSet); // set the write ptr
	const unsigned int aWriteBit = theInLen & 1; // write on odd or even bytes?

	GS_ASSERT(theInLen <= 16);

	// Set buffer to ' '
	memset(theOutStream, ' ', aRowWidth);

	// Convert characters one at a time
	while(theInLen--)
	{
		// Read the next byte
		unsigned char aChar = (unsigned char)(*theInStream++);

		// Write one byte in hex form
		sprintf(theOutStream, "%02X", aChar);

		// Write the printable character
		if (isgraph(aChar))
			*(aTextOutStream++) = (char)aChar;
		else
			*(aTextOutStream++) = aReplaceChar;

		// Move to next hex byte
		theOutStream += 2;

		// Insert a space every other byte
		if ((theInLen & 1) == aWriteBit)
			*theOutStream++ = ' ';
	}

	// Remove NULL terminator from last sprintf
	*theOutStream = ' ';

	// NULL terminate the full string
	*(aTextOutStream) = '\0';
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Write binary data as B64 bytes (40 bytes per line)
void gsDebugBinary(GSIDebugCategory theCat, GSIDebugType theType,
             GSIDebugLevel theLevel, const char* theBuffer, gsi_i32 theLength)
{
	int aBytesLeft = theLength;
	const char* aReadPos = theBuffer;
	char aHexStr[80];

	// convert and display in 40 byte segments
	while(aBytesLeft > 0)
	{
		gsi_i32 aBytesToRead = GS_MIN(aBytesLeft, 16);

		HexEncode16(aReadPos, aHexStr, (unsigned int)aBytesToRead);
		gsDebugFormat(theCat, theType, theLevel, "  %s\r\n", aHexStr);

		aReadPos   += aBytesToRead;
		aBytesLeft -= aBytesToRead;
	};
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void gsiSetDebugLevel(GSIDebugCategory theCat, GSIDebugType theType, 
					  GSIDebugLevel theLevel, gsi_bool notifyChange)
{
	// Verify Parameters
	GS_ASSERT(theCat   <= GSIDebugCat_Count);
	GS_ASSERT(theType  <= GSIDebugType_Count);

	// Set for all categories?
	if (theCat == GSIDebugCat_Count)
	{
		int i=0;

		if (gsi_is_true(notifyChange))
		{
			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Misc,
				GSIDebugLevel_Debug,
				"Debug level set to %s for all categories (SDKs)\r\n",
				gGSIDebugLevelDescriptionStrings[gsiDebugLog2(theLevel)]);
		}

		for (; i<GSIDebugCat_Count; i++)
			gsiSetDebugLevel((GSIDebugCategory)i, theType, theLevel, gsi_false);
		
		return;
	}
	
	// Set for all types?
	else if (theType == GSIDebugType_Count)
	{
		int i=0;

		if (gsi_is_true(notifyChange))
		{

			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Misc,
				GSIDebugLevel_Debug,
				"Debug level set to %s for %s (all types)\r\n",
				gGSIDebugLevelDescriptionStrings[gsiDebugLog2(theLevel)],
				gGSIDebugCatStrings[theCat]);
		}
		
		for (; i<GSIDebugType_Count; i++)
			gsiSetDebugLevel(theCat, (GSIDebugType)i, theLevel, gsi_false);

		return;
	}

	// Is the new level different from the old?
	if (gGSIDebugInstance.mGSIDebugLevel[theCat][theType] != theLevel)
	{
		if (gsi_is_true(notifyChange))
		{
			// Notify of the change
			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Misc, 
				GSIDebugLevel_Comment,
				"Changing debug level: [%s][%s][%02X]\r\n",
				gGSIDebugCatStrings[theCat], 
				gGSIDebugTypeStrings[theType], 
				theLevel );
		}
		gGSIDebugInstance.mGSIDebugLevel[theCat][theType] = theLevel;
	}
}
void gsSetDebugLevel(GSIDebugCategory theCat, GSIDebugType theType, GSIDebugLevel theLevel)
{
	gsiSetDebugLevel(theCat, theType, theLevel, gsi_true);
}

#if !defined(_NITRO)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Set the debug output file to an already open file
// Set to "stdout" for console output
void gsSetDebugFile(FILE* theFile)
{
	if (theFile != gGSIDebugInstance.mGSIDebugFile)
	{
		// If the old file is valid, notify it of the closing
		if (gGSIDebugInstance.mGSIDebugFile != NULL)
		{
			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Misc,
				GSIDebugLevel_Comment, "Debug disabled in this file\r\n");
		}

		// If the new file is valid, notify it of the opening
		{
			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Misc,
				GSIDebugLevel_Comment, "Debug enabled in this file\r\n");
		}

		gGSIDebugInstance.mGSIDebugFile = theFile;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Opens and sets the debug output file
FILE* gsOpenDebugFile(const char* theFileName)
{
	// The new file
	FILE* aFile = NULL;

	// Verify parameters
	GS_ASSERT(theFileName != NULL);

	// Open the new file (clear contents)
	aFile = gsifopen(theFileName, "w+");
	if (aFile != NULL)
		gsSetDebugFile(aFile);

	return aFile;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Retrieve the current debug file (if any)
FILE* gsGetDebugFile()
{
	return gGSIDebugInstance.mGSIDebugFile;
}

#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Set the developer callback
void gsSetDebugCallback(GSIDebugCallback theCallback)
{
	gGSIDebugInstance.mDebugCallback = theCallback;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // GSI_COMMON_DEBUG
