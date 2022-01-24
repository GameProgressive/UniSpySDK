///////////////////////////////////////////////////////////////////////////////
// File:	gpiUnique.c
// SDK:		GameSpy Presence and Messaging SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed
// use or use in a manner not expressly authorized by IGN or GameSpy Technology
// is prohibited.

//INCLUDES
#include "gpi.h"

//FUNCTIONS
static GPResult
gpiSendRegisterUniqueNick(
  GPConnection * connection,
  const char uniquenick[GP_UNIQUENICK_LEN],
  const char cdkey[GP_CDKEY_LEN],
  int operationid
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;

	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\registernick\\\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\uniquenick\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, uniquenick);
	if(cdkey)
	{
		gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\cdkey\\");
		gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, cdkey);
	}
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\partnerid\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->partnerID);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\id\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, operationid);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");
	
	return GP_NO_ERROR;
}

GPResult gpiRegisterUniqueNick(
  GPConnection * connection,
  const char uniquenick[GP_UNIQUENICK_LEN],
  const char cdkey[GP_CDKEY_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIOperation * operation = NULL;
	GPResult result;

	// Add the operation.
	CHECK_RESULT(gpiAddOperation(connection, GPI_REGISTER_UNIQUENICK, NULL, &operation, blocking, callback, param));

	// Send a request for info.
	result = gpiSendRegisterUniqueNick(connection, uniquenick, cdkey, operation->id);
	CHECK_RESULT(result);

	// Process it if blocking.
	if(blocking)
	{
		result = gpiProcess(connection, operation->id);
		CHECK_RESULT(result);
	}

	return GP_NO_ERROR;
}

GPResult gpiProcessRegisterUniqueNick(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
)
{
	GPICallback callback;

	// Check for an error.
	if(gpiCheckForError(connection, input, GPITrue))
		return GP_SERVER_ERROR;

	// This should be \rn\.
	if(strncmp(input, "\\rn\\", 4) != 0)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexpected data was received from the server.");

	// Call the callback.
	callback = operation->callback;
	if(callback.callback != NULL)
	{
		GPRegisterUniqueNickResponseArg * arg;
		arg = (GPRegisterUniqueNickResponseArg *)gsimalloc(sizeof(GPRegisterUniqueNickResponseArg));
		if(arg == NULL)
			Error(connection, GP_MEMORY_ERROR, "Out of memory.");

		arg->result = GP_NO_ERROR;

		CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, 0));
	}

	// This operation is complete.
	gpiRemoveOperation(connection, operation);

	return GP_NO_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// Registration of cdKey now offered separately from uniquenick.
static GPResult
gpiSendRegisterCdKey(
  GPConnection * connection,
  const char cdkey[GP_CDKEY_LEN],
  int gameId,
  int operationid
)
{
	// TODO: Looks like some code using gameId not ported from Open SDK.
	GSI_UNUSED(gameId);

	GPIConnection * iconnection = (GPIConnection*)*connection;

	// Encrypt the cdkey (xor with random values).
	const int useAlternateEncoding = 1;
	char cdkeyxor[GP_CDKEY_LEN];
	char cdkeyenc[GP_CDKEYENC_LEN];
	int cdkeylen = (int)strlen(cdkey);		
	int i=0;
	
	Util_RandSeed((unsigned long)GP_XOR_SEED);
	for (i=0; i < cdkeylen; i++)
	{
		// XOR each character with the next rand.
		char aRand = (char)Util_RandInt(0, 0xFF);
		cdkeyxor[i] = (char)(cdkey[i] ^ aRand);
	}
	cdkeyxor[i] = '\0';

	// Base 64 it (printable chars only).
	B64Encode(cdkeyxor, cdkeyenc, (int)cdkeylen, useAlternateEncoding);

	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\registercdkey\\\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\cdkeyenc\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, cdkeyenc);
//	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\partnerid\\");
//	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->partnerID);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\id\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, operationid);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");
	
	return GP_NO_ERROR;
}

GPResult gpiRegisterCdKey(
  GPConnection * connection,
  const char cdkey[GP_CDKEY_LEN],
  int gameId,
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIOperation * operation = NULL;
	GPResult result;

	// Add the operation.
	CHECK_RESULT(gpiAddOperation(connection, GPI_REGISTER_CDKEY, NULL, &operation, blocking, callback, param));

	// Send a request for info.
	result = gpiSendRegisterCdKey(connection, cdkey, gameId, operation->id);
	CHECK_RESULT(result);

	// Process it if blocking.
	if(blocking)
	{
		result = gpiProcess(connection, operation->id);
		CHECK_RESULT(result);
	}

	return GP_NO_ERROR;
}

GPResult gpiProcessRegisterCdKey(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
)
{
	GPICallback callback;

	// Check for an error.
	if(gpiCheckForError(connection, input, GPITrue))
		return GP_SERVER_ERROR;

	// This should be \rc\.
	if(strncmp(input, "\\rc\\", 4) != 0)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexpected data was received from the server.");

	// Call the callback.
	callback = operation->callback;
	if(callback.callback != NULL)
	{
		GPRegisterCdKeyResponseArg * arg;
		arg = (GPRegisterCdKeyResponseArg *)gsimalloc(sizeof(GPRegisterCdKeyResponseArg));
		if(arg == NULL)
			Error(connection, GP_MEMORY_ERROR, "Out of memory.");

		arg->result = GP_NO_ERROR;

		CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, 0));
	}

	// This operation is complete.
	gpiRemoveOperation(connection, operation);

	return GP_NO_ERROR;
}
