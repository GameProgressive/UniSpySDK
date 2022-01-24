///////////////////////////////////////////////////////////////////////////////
// File:	ghttpEncryption.h
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#ifndef __GHTTPENCRYPTION_H__
#define __GHTTPENCRYPTION_H__

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Encryption method
typedef enum
{
	GHIEncryptionMethod_None,
	GHIEncryptionMethod_Encrypt,  // encrypt raw data written to buffer
	GHIEncryptionMethod_Decrypt   // decrypt raw data written to buffer
} GHIEncryptionMethod;

// Encryption results
typedef enum
{
	GHIEncryptionResult_None,
	GHIEncryptionResult_Success,        // successfully encrypted/decrypted
	GHIEncryptionResult_BufferTooSmall, // buffer was too small to hold converted data
	GHIEncryptionResult_Error           // some other kind of error
} GHIEncryptionResult;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct GHIEncryptor; // forward declare for callbacks
struct GHIConnection;

// Called to init the encryption engine
typedef GHIEncryptionResult (*GHTTPEncryptorInitFunc)   (struct GHIConnection * theConnection, 
											   struct GHIEncryptor * theEncryptor);

// Called to connect the socket (some engines do this internally)
typedef GHIEncryptionResult (*GHTTPEncryptorConnectFunc)(struct GHIConnection * theConnection, 
											   struct GHIEncryptor * theEncryptor);

// Called to start the handshake process engine
typedef GHIEncryptionResult (*GHTTPEncryptorStartFunc)(struct GHIConnection * theConnection, 
											   struct GHIEncryptor * theEncryptor);

// Called to destroy the encryption engine
typedef GHIEncryptionResult (*GHTTPEncryptorCleanupFunc)(struct GHIConnection * theConnection, 
											   struct GHIEncryptor * theEncryptor);

// Called when data needs to be encrypted
//    - entire plain text buffer will be encrypted
typedef GHIEncryptionResult (*GHTTPEncryptorEncryptFunc)(struct GHIConnection * theConnection, 
											   struct GHIEncryptor * theEncryptor,
											   const char * thePlainTextBuffer,
											   int          thePlainTextLength, // [in]
											   char *       theEncryptedBuffer,
											   int *        theEncryptedLength); // [in/out]

// Called when data needs to be decrypted 
//    - encrypted data may be left in the buffer
//    - decrypted buffer is appended to, not overwritten
typedef GHIEncryptionResult (*GHTTPEncryptorDecryptFunc)(struct GHIConnection * theConnection, 
											   struct GHIEncryptor* theEncryptor,
											   const char * theEncryptedBuffer,
											   int *        theEncryptedLength, // [in/out]
											   char *       theDecryptedBuffer,
											   int *        theDecryptedLength);// [in/out]


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct GHIEncryptor
{
	void*     mInterface;   // only SSL is currently supported
	GHTTPEncryptionEngine mEngine;
	GHTTPBool mInitialized;
	GHTTPBool mSessionStarted;      // handshake started?
	GHTTPBool mSessionEstablished;  // handshake completed?
	
	// (As coded, these two are exclusive!)
	//    pattern 1 = manually encrypt the buffer, then send using normal socket functions
	//    pattern 2 = send plain text through the encryption engine, it will send
	GHTTPBool mEncryptOnBuffer;  // engine encrypts when writing to a buffer? (pattern 1)
	GHTTPBool mEncryptOnSend;    // engine encrypts when sending over socket? (pattern 2)

	// If GHTTPTrue, the SSL library handles sending/receiving handshake messages
	GHTTPBool mLibSendsHandshakeMessages;  

	// Functions for engine use
	GHTTPEncryptorInitFunc      mInitFunc;
	GHTTPEncryptorCleanupFunc   mCleanupFunc;
	GHTTPEncryptorStartFunc     mStartFunc;  // start the handshake process
	GHTTPEncryptorEncryptFunc   mEncryptFunc;
	GHTTPEncryptorDecryptFunc   mDecryptFunc;
#if defined(TWLSSL) && defined(_NITRO)	
	gsi_bool  mNotInCloseSocket; // This is special when TWLSSL & NITRO
#endif
} GHIEncryptor;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// ssl encryption
GHIEncryptionResult ghiEncryptorSslInitFunc(struct GHIConnection * connection,
									struct GHIEncryptor  * theEncryptor);
GHIEncryptionResult ghiEncryptorSslCleanupFunc(struct GHIConnection * connection,
									   struct GHIEncryptor  * theEncryptor);

GHIEncryptionResult ghiEncryptorSslStartFunc(struct GHIConnection * connection,
								struct GHIEncryptor * theEncryptor);
									   
GHIEncryptionResult ghiEncryptorSslEncryptFunc(struct GHIConnection * connection,
									   struct GHIEncryptor  * theEncryptor,
									   const char * thePlainTextBuffer,
									   int          thePlainTextLength,
									   char *       theEncryptedBuffer,
									   int *        theEncryptedLength);
GHIEncryptionResult ghiEncryptorSslDecryptFunc(struct GHIConnection * connection,
									   struct GHIEncryptor  * theEncryptor,
									   const char * theEncryptedBuffer,
									   int *        theEncryptedLength,
									   char *       theDecryptedBuffer,
									   int *        theDecryptedLength);
GHIEncryptionResult ghiEncryptorSslEncryptSend(struct GHIConnection * connection,
                                 struct GHIEncryptor * theEncryptor,
                                 const char * thePlainTextBuffer,
                                 int thePlainTextLength,
                                 int * theBytesSent);
GHIEncryptionResult ghiEncryptorSslDecryptRecv(struct GHIConnection * connection,
                                 struct GHIEncryptor * theEncryptor,
                                 char * theDecryptedBuffer,
                                 int * theDecryptedLength);
                                 
GHTTPBool ghiEncyptorSetRootCAList( char *url, void *theRootCAList);
GHTTPBool ghiEncyptorCleanupRootCAList( char *url);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __GHTTPENCRYPTION_H__
