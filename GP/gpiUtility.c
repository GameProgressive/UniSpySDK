///////////////////////////////////////////////////////////////////////////////
// File:	gpiUtility.c
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

//INCLUDES
//////////
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "gpi.h"

//DEFINES
/////////
#define OUTPUT_MAX_COL     100

//FUNCTIONS
void
strzcpy(
  char * dest,
  const char * src,
  size_t len
)
{
	GS_ASSERT(dest != NULL);
	GS_ASSERT(src != NULL);

	strncpy(dest, src, len);
	dest[len - 1] = '\0';
}

GPIBool
gpiCheckForError(
  GPConnection * connection,
  const char * input,
  GPIBool callErrorCallback
)
{
	char buffer[16];
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	if(strncmp(input, "\\error\\", 7) == 0)
	{
		// Get the err code.
		if(gpiValueForKey(input, "\\err\\", buffer, sizeof(buffer)))
			iconnection->errorCode = (GPErrorCode)atoi(buffer);
		
		// Get the error string.
		if(!gpiValueForKey(input, "\\errmsg\\", iconnection->errorString, sizeof(iconnection->errorString)))
			iconnection->errorString[0] = '\0';

#ifdef GSI_UNICODE
		// Update the UNICODE version.
		UTF8ToUCSString(iconnection->errorString, iconnection->errorString_W);
#endif
		// Call the error callback?
		if(callErrorCallback)
		{
			GPIBool fatal = (GPIBool)(strstr(input, "\\fatal\\") != NULL);
			gpiCallErrorCallback(connection, GP_SERVER_ERROR, fatal ? GP_FATAL : GP_NON_FATAL);
		}
		
		return GPITrue;
	}

	return GPIFalse;
}

GPIBool
gpiValueForKeyWithIndex(
  const char * command,
  const char * key,
  int * index,
  char * value,
  int len
)
{
    char delimiter;
    const char * start;
    int i;
    char c;

    // Check for NULL.
    GS_ASSERT(command != NULL);
    GS_ASSERT(key != NULL);
    GS_ASSERT(value != NULL);
    GS_ASSERT(len > 0);

    // Find which char is the delimiter.
    delimiter = key[0];

    // Find the key - first navigate to the index.
    command += *index;
    start = strstr(command, key);
    if(start == NULL)
        return GPIFalse;

    // Get to the start of the value.
    start += strlen(key);

    // Copy in the value.
    len--;
    for(i = 0 ; (i < len) && ((c = start[i]) != '\0') && (c != delimiter) ; i++)
    {
        value[i] = c;
    }
    value[i] = '\0';

    // Copy back current end point for index.
    *index += (int)((start - command) + strlen(value));

    return GPITrue;
}

GPIBool
gpiValueForKey(
  const char * command,
  const char * key,
  char * value,
  int len
)
{
	char delimiter;
	const char * start;
	int i;
	char c;

	// Check for NULL.
	GS_ASSERT(command != NULL);
	GS_ASSERT(key != NULL);
	GS_ASSERT(value != NULL);
	GS_ASSERT(len > 0);

	// Find which char is the delimiter.
	delimiter = key[0];

	// Find the key.
	start = strstr(command, key);
	if(start == NULL)
		return GPIFalse;

	// Get to the start of the value.
	start += strlen(key);

	// Copy in the value.
	len--;
	for(i = 0 ; (i < len) && ((c = start[i]) != '\0') && (c != delimiter) ; i++)
	{
		value[i] = c;
	}
	value[i] = '\0';

	return GPITrue;
}

char *
gpiValueForKeyAlloc(
  const char * command,
  const char * key
)
{
	char delimiter;
	const char * start;
	char c;
	char * value;
	size_t len;

	// Check for NULL.
	GS_ASSERT(command != NULL);
	GS_ASSERT(key != NULL);

	// Find which char is the delimiter.
	delimiter = key[0];

	// Find the key.
	start = strstr(command, key);
	if(start == NULL)
		return NULL;

	// Get to the start of the value.
	start += strlen(key);

	// Find the key length.
	for(len = 0 ; ((c = start[len]) != '\0') && (c != delimiter) ; len++)  { };

	// Allocate the value.
	value = (char *)gsimalloc(len + 1);
	if(!value)
		return NULL;

	// Copy in the value.
	memcpy(value, start, len);
	value[len] = '\0';

	return value;
}

GPResult
gpiCheckSocketConnect(
  GPConnection * connection,
  SOCKET sock,
  int * state
)
{
	int aWriteFlag  = 0;
	int aExceptFlag = 0;
	int aReturnCode = 0;

	// Check if the connect is completed.
	aReturnCode = GSISocketSelect(sock, NULL, &aWriteFlag, &aExceptFlag);
	if ( gsiSocketIsError(aReturnCode))
	{
		gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Network, GSIDebugLevel_HotError,
			"Error connecting\n");
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_NETWORK, "There was an error checking for a completed connection.");
	}

	if (aReturnCode > 0)
	{
		// Check for a failed attempt.
		if(aExceptFlag)
		{
			gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Network, GSIDebugLevel_HotError,
				"Connection rejected\n");
			*state = GPI_DISCONNECTED;
			return GP_NO_ERROR;
		}

		// Check for a successful attempt.
		if(aWriteFlag)
		{
			gsDebugFormat(GSIDebugCat_GP, GSIDebugType_Network, GSIDebugLevel_Notice,
				"Connection accepted\n");
			*state = GPI_CONNECTED;
			return GP_NO_ERROR;
		}
	}

	// Not connected yet.
	*state = GPI_NOT_CONNECTED;
	return GP_NO_ERROR;
}

GPResult
gpiReadKeyAndValue(
  GPConnection * connection,
  const char * buffer,
  int * index,
  char key[512],
  char value[512]
)
{
	int c;
	int i;
	char * start;

	GS_ASSERT(buffer != NULL);
	GS_ASSERT(key != NULL);
	GS_ASSERT(value != NULL);

	buffer += *index;
	start = (char *)buffer;

	if(*buffer++ != '\\')
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");

	for(i = 0 ; (c = *buffer++) != '\\' ; i++)
	{
		if(c == '\0')
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		if(i == 511)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		*key++ = (char)c;
	}
	*key = '\0';

	for(i = 0 ; ((c = *buffer++) != '\\') && (c != '\0') ; i++)
	{
		if(i == 511)
			CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Parse Error.");
		*value++ = (char)c;
	}
	*value = '\0';

	*index += (int)(buffer - start - 1);

	return GP_NO_ERROR;
}

void
gpiSetError(
  GPConnection * connection,
  GPErrorCode errorCode,
  const char * errorString
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	// Copy the string.
	strzcpy(iconnection->errorString, errorString, GP_ERROR_STRING_LEN);

#ifdef GSI_UNICODE
	// Update the unicode version.
	UTF8ToUCSStringLen(iconnection->errorString, iconnection->errorString_W, GP_ERROR_STRING_LEN);
#endif

	// Set the code.
	iconnection->errorCode = errorCode;
}

void
gpiSetErrorString(
  GPConnection * connection,
  const char * errorString
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;
	
	// Copy the string.
	strzcpy(iconnection->errorString, errorString, GP_ERROR_STRING_LEN);

#ifdef GSI_UNICODE
	// Update the unicode version.
	UTF8ToUCSStringLen(iconnection->errorString, iconnection->errorString_W, GP_ERROR_STRING_LEN);
#endif

}

void
gpiEncodeString(
  const char * unencodedString,
  char * encodedString
)
{
	size_t i;
	const int useAlternateEncoding = 1;

	// Encrypt the password (xor with random values).
	char passwordxor[GP_PASSWORD_LEN];
	size_t passwordlen = strlen(unencodedString);
	
	Util_RandSeed((unsigned long)GP_XOR_SEED);
	for (i=0; i < passwordlen; i++)
	{
		// XOR each character with the next rand.
		char aRand = (char)Util_RandInt(0, 0xFF);
		passwordxor[i] = (char)(unencodedString[i] ^ aRand);
	}
	passwordxor[i] = '\0';

	// Base 64 it (printable chars only)
	B64Encode(passwordxor, encodedString, (int)passwordlen, useAlternateEncoding);
}
