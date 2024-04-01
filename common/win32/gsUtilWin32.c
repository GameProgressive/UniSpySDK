///////////////////////////////////////////////////////////////////////////////
// File:	gsUtilWin32.c
// SDK:		GameSpy Common Windows code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "../gsCommon.h"

gsi_i64 gsiStringToInt64(const char *theNumberStr)
{
	return _atoi64(theNumberStr);
}

void gsiInt64ToString(char theNumberStr[33], gsi_i64 theNumber)
{
	// you want to fit the number! 
	// give me a valid string!
	GS_ASSERT(theNumberStr != NULL);
	
#if _MSC_VER > 1300
	sprintf(theNumberStr, "%lld", theNumber);
#else 
	sprintf(theNumberStr, "%I64d", theNumber);
#endif
}

double gsiWStringToDouble(const wchar_t *inputString)
{
	return _wtof(inputString);
}
// ErrorMessage: Displays message and goes into an infinite loop
// continues rendering
void  _gsDebugAssert(const char *string)
{

	//DebugBreak();
	

	#ifdef _CONSOLE   //,_MBCS
		gsi_bool test = gsi_true;
		gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Misc, GSIDebugLevel_HotError, "%s\n", string);
		while (test) {}

	#else
	{		
		OutputDebugStringA( string);

		#ifndef GS_ASSERT_NO_MESSAGEBOX
		{
			int rcode = MessageBoxA(NULL,string,"Assert Failed",MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

			if (rcode == IDABORT)
			{
				exit(0);
			}
			if (rcode == IDRETRY)
			{
				DebugBreak();
				return;
			}
		}
		#else
			DebugBreak();
		#endif
	}
	#endif
}

void gsiDebugPrint(const char* format, va_list params)
{
	static char string[256];
	vsprintf(string, format, params); 			
	OutputDebugStringA(string);
}

#if defined(_MSC_VER) && (_MSC_VER < 1300)
	//extern added for vc6 compatability.
	extern void* __cdecl _aligned_malloc(size_t size, int boundary);
#endif
	
void* GS_STATIC_CALLBACK _gsi_memalign(size_t boundary, size_t size)
{
	return  _aligned_malloc(size, (int)boundary);
}
