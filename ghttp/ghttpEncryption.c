///////////////////////////////////////////////////////////////////////////////
// File:	ghttpEncryption.c
// SDK:		GameSpy HTTP SDK
//
// Copyright (c) 2012 GameSpy Technology & IGN Entertainment, Inc. All rights
// reserved. This software is made available only pursuant to certain license 
// terms offered by IGN or its subsidiary GameSpy Industries, Inc. Unlicensed 
// use or use in a  manner not expressly authorized by IGN or GameSpy 
// Technology is prohibited.

#include "ghttpCommon.h"
#if defined(MATRIXSSL)
#include <matrixssl/matrixssl.h>
#endif
#if defined(OPENSSL)
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/buffer.h>
#include <openssl/x509v3.h>
#include <openssl/opensslconf.h>
#include <openssl/err.h>
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GHTTPBool ghttpSetRequestEncryptionEngine(GHTTPRequest request, GHTTPEncryptionEngine engine)
{
	GHIConnection * connection = ghiRequestToConnection(request);
	if(!connection)
		return GHTTPFalse;
	
	// Here we translate default into the actual engine name.
	// We don't want to set the engine value to "default" because we'd lose the
	// ability to determine the engine name in other places.
	if (engine == GHTTPEncryptionEngine_Default)
	{
#if defined(OPENSSL)
		engine = GHTTPEncryptionEngine_OpenSSL;
#elif defined(MATRIXSSL)
		engine = GHTTPEncryptionEngine_MatrixSsl;
#elif defined(REVOEXSSL)
		engine = GHTTPEncryptionEngine_RevoEx;
#elif defined(TWLSSL)
		engine = GHTTPEncryptionEngine_Twl;
#else
		engine = GHTTPEncryptionEngine_GameSpy;
#endif
	}

	// If the same engine has previously been set, we're done.
	if (connection->encryptor.mEngine == engine)
		return GHTTPTrue; 

	// If a different engine has previously been set, then we won’t be able to connect.
	if (connection->encryptor.mInterface != NULL &&
		connection->encryptor.mEngine != engine)
	{
		return GHTTPFalse; 
	}

	// If the URL is HTTPS, but the engine is specific as NONE, then we can't connect.
	if((engine == GHTTPEncryptionEngine_None) && (strncmp(connection->URL, "https://", 8) == 0))
		return GHTTPFalse;

	// Initialize the engine.
	connection->encryptor.mEngine = engine;

	if (engine == GHTTPEncryptionEngine_None)
	{
		connection->encryptor.mInterface = NULL;
		return GHTTPTrue; // This is the default, just return it.
	}
	else
	{
		// 02OCT07 BED: Design was changed to only allow one engine at a time.
		//				Assert that the specified engine is the one supported.
		if (engine != GHTTPEncryptionEngine_Default)
		{
			#if defined(OPENSSL)
				GS_ASSERT(engine==GHTTPEncryptionEngine_OpenSSL);
			#elif defined(MATRIXSSL)
				GS_ASSERT(engine==GHTTPEncryptionEngine_MatrixSsl);
			#elif defined(REVOEXSSL)
				GS_ASSERT(engine==GHTTPEncryptionEngine_RevoEx);
			#elif defined(TWLSSL)
				GS_ASSERT(engine==GHTTPEncryptionEngine_Twl);
			#else
				GS_ASSERT(engine==GHTTPEncryptionEngine_GameSpy);
			#endif
		}
		
		connection->encryptor.mInterface   = NULL;
		connection->encryptor.mInitFunc    = ghiEncryptorSslInitFunc;
		connection->encryptor.mStartFunc   = ghiEncryptorSslStartFunc;
		connection->encryptor.mCleanupFunc = ghiEncryptorSslCleanupFunc;
		connection->encryptor.mEncryptFunc = ghiEncryptorSslEncryptFunc;
		connection->encryptor.mDecryptFunc = ghiEncryptorSslDecryptFunc;
		connection->encryptor.mInitialized = GHTTPFalse;
		connection->encryptor.mSessionStarted = GHTTPFalse;
		connection->encryptor.mSessionEstablished = GHTTPFalse;
		connection->encryptor.mEncryptOnBuffer = GHTTPTrue;
		connection->encryptor.mEncryptOnSend   = GHTTPFalse;
		connection->encryptor.mLibSendsHandshakeMessages = GHTTPFalse;
#if defined(TWLSSL) && defined(_NITRO)
	    connection->encryptor.mNotInCloseSocket = gsi_true; // This is special when TWLSSL & NITRO
#endif
		return GHTTPTrue;
	}
}

#if defined(TWLSSL)
static GHTTPBool ghiEncryptorSetTwlRootCA( char *url, void *theRootCAList);
static GHTTPBool ghiEncryptorCleanupTwlRootCA( char *url);
#endif

// Used only by TWL.
GHTTPBool ghiEncyptorSetRootCAList( char *url, void *theRootCAList)
{
    GHTTPBool result = GHTTPFalse;
#if defined(TWLSSL)

    GS_ASSERT(url);
    GS_ASSERT(theRootCAList);

    if (!theRootCAList || !url || strcmp(url, "")== 0) 
    {

        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "%s(@%s:%d):SSL - Null pointer or empty url string; url=0x%X, theRootCAList=0x%X\n", 
            __FILE__, __FUNCTION__, __LINE__, url, theRootCAList);		        

        return result;
    }
    
    result = ghiEncryptorSetTwlRootCA(url, theRootCAList);

#else 

    GSI_UNUSED(url);
    GSI_UNUSED(theRootCAList);
#endif
     
    return result;
}

// Used only by TWL.
GHTTPBool ghiEncyptorCleanupRootCAList( char *url)
{
    GHTTPBool result = GHTTPFalse;
#if defined(TWLSSL)
    result = ghiEncryptorCleanupTwlRootCA(url);
#else 
    GSI_UNUSED(url);
#endif
     
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// *********************  OPENSSL ENCRYPTION ENGINE  *********************** //
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(OPENSSL)
const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS";

typedef struct gsOpenSSLInterface
{
    SSL_CTX *mContext;
    BIO *mBio; // IO interface.
    GHTTPBool mConnected; // means "connected to socket", not "connected to remote machine"
} gsOpenSSLInterface;

int verify_callback(int preverify, X509_STORE_CTX *x509_ctx)
{
	/* For error codes, see http://www.openssl.org/docs/apps/verify.html  */
#ifdef GSI_COMMON_DEBUG
	int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
#endif
	int err = X509_STORE_CTX_get_error(x509_ctx);

#ifdef GSI_COMMON_DEBUG
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"verify_callback (depth=%d)(preverify=%d)\n", depth, preverify);
#endif
 
	if (preverify == 0) {
		if (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError, 
				"  Error = X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY\n");
		else if (err == X509_V_ERR_CERT_UNTRUSTED)
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"  Error = X509_V_ERR_CERT_UNTRUSTED\n");
		else if (err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"  Error = X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN\n");
		else if (err == X509_V_ERR_CERT_NOT_YET_VALID)
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"  Error = X509_V_ERR_CERT_NOT_YET_VALID\n");
		else if (err == X509_V_ERR_CERT_HAS_EXPIRED)
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"  Error = X509_V_ERR_CERT_HAS_EXPIRED\n");
		else if (err == X509_V_OK)
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"  Error = X509_V_OK\n");
		else
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
				"  Error = %d\n", err);
	}

#if !defined(NDEBUG)
	return 1;
#else
	return preverify;
#endif
}

// Init the engine
GHIEncryptionResult ghiEncryptorSslInitFunc(struct GHIConnection *connection,
    struct GHIEncryptor  *theEncryptor)
{
    gsOpenSSLInterface *sslInterface = NULL;

    // There is only one place where this function should be called,
    //  and it should check if the engine has been initialized
    GS_ASSERT(theEncryptor->mInitialized == GHTTPFalse);
    GS_ASSERT(theEncryptor->mInterface == NULL);

    // allocate the interface (need one per connection)
    theEncryptor->mInterface = gsimalloc(sizeof(gsOpenSSLInterface));
    if (theEncryptor->mInterface == NULL)
    {
        // memory allocation failed
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
            "Failed to allocate SSL interface (out of memory: %d bytes)\r\n", sizeof(gsOpenSSLInterface));
        return GHIEncryptionResult_Error;
    }
    memset(theEncryptor->mInterface, 0, sizeof(gsOpenSSLInterface));
    sslInterface = (gsOpenSSLInterface*)theEncryptor->mInterface;

    // Init the OpenSSL library
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    SSL_load_error_strings();
#else
    OPENSSL_init_ssl(0, NULL);
#endif

    // Create an OpenSSL context to use.
    {
#ifdef GSI_COMMON_DEBUG
        unsigned long ssl_err = 0;
#endif
        SSL *ssl;
        char buff[64];

        sslInterface->mContext = SSL_CTX_new(SSLv23_method());

        GS_ASSERT(sslInterface->mContext != NULL);
        if (sslInterface->mContext == NULL)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to allocate OpenSSL context, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }

        SSL_CTX_set_verify(sslInterface->mContext, SSL_VERIFY_PEER, verify_callback);
        SSL_CTX_set_verify_depth(sslInterface->mContext, 5);

        sslInterface->mBio = BIO_new_ssl_connect(sslInterface->mContext);

        if (sslInterface->mBio == NULL)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to allocate OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }

        if (BIO_set_conn_hostname(sslInterface->mBio, connection->serverAddress) != 1)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to set URL for OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }

        snprintf(buff, sizeof(buff), "%hu", connection->serverPort);

        if (BIO_set_conn_port(sslInterface->mBio, buff) != 1)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to set URL for OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }

        BIO_get_ssl(sslInterface->mBio, &ssl);

        if (ssl == NULL)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to get SSL pointer for OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }

        if (SSL_set_cipher_list(ssl, PREFERRED_CIPHERS) != 1)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to set preferred ciphers for OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }

        if (SSL_set_tlsext_host_name(ssl, connection->serverAddress) != 1)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to set URL for OpenSSL connection TLS, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }
    }

    theEncryptor->mInitialized = GHTTPTrue;
    theEncryptor->mSessionStarted = GHTTPFalse;
    theEncryptor->mSessionEstablished = GHTTPFalse; 
    theEncryptor->mEncryptOnBuffer = GHTTPFalse;
    theEncryptor->mEncryptOnSend = GHTTPTrue;
    theEncryptor->mLibSendsHandshakeMessages = GHTTPTrue;

    gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
        "GameSpy SSL (OpenSSL engine) initialized\r\n");

    return GHIEncryptionResult_Success;
}


// Destroy the engine
GHIEncryptionResult ghiEncryptorSslCleanupFunc(struct GHIConnection *connection,
    struct GHIEncryptor  *theEncryptor)
{
    gsOpenSSLInterface *sslInterface = NULL;
    
    if (theEncryptor != NULL) {
        if (theEncryptor->mInterface != NULL) {
            sslInterface = (gsOpenSSLInterface*)theEncryptor->mInterface;

            // Close IO.
            if (sslInterface->mBio != NULL) {
                BIO_free_all(sslInterface->mBio);
                sslInterface->mBio = NULL;
            }

            // Destroy the context.
            if (sslInterface->mContext != NULL) {
                SSL_CTX_free(sslInterface->mContext);
                sslInterface->mContext = NULL;
            }

            // Free the interface.
            gsifree(sslInterface);
            theEncryptor->mInterface = NULL;
        }

        theEncryptor->mInitialized = GHTTPFalse;
        theEncryptor->mSessionStarted = GHTTPFalse;
        theEncryptor->mSessionEstablished = GHTTPFalse;
    }

    return GHIEncryptionResult_Success;
}


GHIEncryptionResult ghiEncryptorSslStartFunc(struct GHIConnection *connection,
    struct GHIEncryptor  *theEncryptor)
{
    gsOpenSSLInterface *sslInterface = (gsOpenSSLInterface*)theEncryptor->mInterface;
    SSL *ssl = NULL;
#ifdef GSI_COMMON_DEBUG
    unsigned long ssl_err;
#endif
    long result;
    GS_ASSERT(theEncryptor->mSessionStarted == GHTTPFalse);

    // Call this only AFTER the socket has been connected to the remote server
    if (!sslInterface->mConnected)
    {
        if (BIO_do_connect(sslInterface->mBio) != 1)
        {
#ifdef GSI_COMMON_DEBUG
            ssl_err = ERR_get_error();
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                "Failed to open OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
            return GHIEncryptionResult_Error;
        }

        sslInterface->mConnected = GHTTPTrue;
    }

    GS_ASSERT(sslInterface->mConnected == GHTTPTrue);

    // begin securing the session
    if (BIO_do_handshake(sslInterface->mBio) != 1)
    {
#ifdef GSI_COMMON_DEBUG
        ssl_err = ERR_get_error();
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
            "Failed handshake on OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
        return GHIEncryptionResult_Error;
    }

    BIO_get_ssl(sslInterface->mBio, &ssl);

    if (ssl == NULL)
    {
#ifdef GSI_COMMON_DEBUG
        ssl_err = ERR_get_error();
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
            "Failed to get SSL pointer for OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
        return GHIEncryptionResult_Error;
    }

    // Verification step 1. get the cert from the server.
    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) X509_free(cert); /* Free immediately */

    GS_ASSERT(cert != NULL);
    if (cert == NULL)
    {
#ifdef GSI_COMMON_DEBUG
        ssl_err = ERR_get_error();
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
            "Failed to get peer certificate for OpenSSL connection, %s.\r\n", ERR_reason_error_string(ssl_err));
#endif
        return GHIEncryptionResult_Error;
    }

    // Step 2, verify the certificate we got.
    result = SSL_get_verify_result(ssl);

    switch (result)
    {
    case X509_V_OK:
        break;
    default:
        // TODO, what errors should we just log and proceed on?
        break;
    }

    // Success
    theEncryptor->mSessionStarted = GHTTPTrue;
    theEncryptor->mSessionEstablished = GHTTPTrue;
    return GHIEncryptionResult_Success;
}

// Encrypt and send some data
GHIEncryptionResult ghiEncryptorSslEncryptSend(struct GHIConnection *connection,
    struct GHIEncryptor  *theEncryptor,
    const char *thePlainTextBuffer,
    int          thePlainTextLength,
    int         *theBytesSentOut)
{
    gsOpenSSLInterface *sslInterface = (gsOpenSSLInterface*)theEncryptor->mInterface;
    *theBytesSentOut = BIO_write(sslInterface->mBio, thePlainTextBuffer, thePlainTextLength);
    GSI_UNUSED(connection);
    return GHIEncryptionResult_Success;
}

// Receive and decrypt some data
GHIEncryptionResult ghiEncryptorSslDecryptRecv(struct GHIConnection * connection,
    struct GHIEncryptor  * theEncryptor,
    char * theDecryptedBuffer,
    int *  theDecryptedLength)
{
    gsOpenSSLInterface *sslInterface = (gsOpenSSLInterface*)theEncryptor->mInterface;
    *theDecryptedLength = BIO_read(sslInterface->mBio, theDecryptedBuffer, *theDecryptedLength);
    GSI_UNUSED(connection);
    return GHIEncryptionResult_Success;
}

GHIEncryptionResult ghiEncryptorSslEncryptFunc(struct GHIConnection * connection,
    struct GHIEncryptor  * theEncryptor,
    const char * thePlainTextBuffer,
    int          thePlainTextLength,
    char *       theEncryptedBuffer,
    int *        theEncryptedLength)
{
    GS_FAIL(); // Should never call this for OpenSSL SSL!  It uses encrypt on send

    GSI_UNUSED(connection);
    GSI_UNUSED(theEncryptor);
    GSI_UNUSED(thePlainTextBuffer);
    GSI_UNUSED(thePlainTextLength);
    GSI_UNUSED(theEncryptedBuffer);
    GSI_UNUSED(theEncryptedLength);

    return GHIEncryptionResult_Error;
}

GHIEncryptionResult ghiEncryptorSslDecryptFunc(struct GHIConnection * connection,
    struct GHIEncryptor  * theEncryptor,
    const char * theEncryptedBuffer,
    int *        theEncryptedLength,
    char *       theDecryptedBuffer,
    int *        theDecryptedLength)
{
    GS_FAIL(); // Should never call this for OpenSSL SSL!  It uses decrypt on recv

    GSI_UNUSED(connection);
    GSI_UNUSED(theEncryptor);
    GSI_UNUSED(theEncryptedBuffer);
    GSI_UNUSED(theEncryptedLength);
    GSI_UNUSED(theDecryptedBuffer);
    GSI_UNUSED(theDecryptedLength);

    return GHIEncryptionResult_Error;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// *********************  MATRIXSSL ENCRYPTION ENGINE  ********************* //
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#elif defined(MATRIXSSL)

// SSL requires a certificate validator.
static int ghiSslCertValidator(struct sslCertInfo* theCertInfo, void* theUserData)
{
	// Taken from matrisSslExample.
	sslCertInfo_t	*next;
/*
	Make sure we are checking the last cert in the chain.
*/
	next = theCertInfo;
	while (next->next != NULL) {
		next = next->next;
	}
	return next->verified;
}

// Initialize the engine.
GHIEncryptionResult ghiEncryptorSslInitFunc(struct GHIConnection * connection,
											  struct GHIEncryptor  * theEncryptor)
{
	sslKeys_t *keys = NULL;
	sslSessionId_t *id = NULL;

	int ecodeResult;

	if (matrixSslOpen() < 0)
		return GHIEncryptionResult_Error;

	if (matrixSslReadKeys(&keys, NULL, NULL, NULL, NULL) < 0)
		return GHIEncryptionResult_Error;

	if (matrixSslNewSession((ssl_t**)&theEncryptor->mInterface, keys, id, 0) < 0)
		return GHIEncryptionResult_Error;

	matrixSslSetCertValidator((ssl_t*)theEncryptor->mInterface, ghiSslCertValidator, NULL);

	theEncryptor->mInitialized = GHTTPTrue;
	return GHIEncryptionResult_Success;
}

// Start the handshake process.
GHIEncryptionResult ghiEncryptorSslInitFunc(struct GHIConnection * connection,
											  struct GHIEncryptor  * theEncryptor)
{
	sslBuf_t helloWrapper;
	
	// Prepare the hello message.
	helloWrapper.buf   = connection->sendBuffer.data;
	helloWrapper.size  = connection->sendBuffer.size;
	helloWrapper.start = connection->sendBuffer.data + connection->sendBuffer.pos;
	helloWrapper.end   = helloWrapper.start; // Start writing here.
	
	ecodeResult = matrixSslEncodeClientHello((ssl_t*)theEncryptor->mInterface, &helloWrapper, 0); // 0 = cipher
	if (ecodeResult != 0) 
		return GHIEncryptionResult_Error; // There was an error.

	// Adjust the sendBuffer to account for the new data.
	connection->sendBuffer.len += (int)(helloWrapper.end - helloWrapper.start);
	connection->sendBuffer.encrypted = GHTTPTrue;
	theEncryptor->mSessionStarted = GHTTPTrue;
}

// Destroy the engine.
GHIEncryptionResult ghiEncryptorSslCleanupFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor)
{
	matrixSslClose();
	return GHIEncryptionResult_Success;
}

// Encrypt some data. theEncryptedLength is reduced by the length of data
// written to theEncryptedBuffer.
GHIEncryptionResult ghiEncryptorSslEncryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * thePlainTextBuffer,
												 int          thePlainTextLength,
												 char *       theEncryptedBuffer,
												 int *        theEncryptedLength)
{
	int encodeResult = 0;

	// This is the SSL buffer wrapper.
	// Append to theDecryptedBuffer.
	sslBuf_t encryptedBuf;
	encryptedBuf.buf   = theEncryptedBuffer;  // buf starts here.
	encryptedBuf.start = theEncryptedBuffer;  // readpos,  set to start.
	encryptedBuf.end   = theEncryptedBuffer;  // writepos, set to start.
	encryptedBuf.size  = *theEncryptedLength; // Total size of buf.
	
	// Perform the encryption.
	encodeResult = matrixSslEncode(connection->encryptor.mInterface, 
		(unsigned char*)thePlainTextBuffer, *thePlainTextLength, &encryptedBuf);

	if (encodeResult == SSL_ERROR)
		return GHIEncryptionResult_Error;
	else if (encodeResult == SSL_FULL)
		return GHIEncryptionResult_BufferTooSmall;
	else
	{
		//*thePlainTextLength = *thePlainTextLength; // We always use the entire buffer.
		*theEncryptedLength -= (int)(encryptedBuf.end - encryptedBuf.start);
		return GHIEncryptionResult_Success;
	}
}

// Decrypt some data.
// During the handshaking process, this may result in data being appended to the send buffer.
// Data may be left in the encrypted buffer.
// theEncryptedLength becomes the length of data read from theEncryptedBuffer.
// theDecryptedLength becomes the length of data written to theDecryptedBuffer.
GHIEncryptionResult ghiEncryptorSslDecryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * theEncryptedBuffer,
												 int *        theEncryptedLength,
												 char *       theDecryptedBuffer,
												 int *        theDecryptedLength)
{
	GHTTPBool decryptMore = GHTTPTrue;
	int decodeResult = 0;

	// SSL buffer wrappers.
	sslBuf_t inBuf;
	sslBuf_t decryptedBuf;
	int encryptedStartSize = *theEncryptedLength;

	// Read from theEncryptedBuffer; we have to cast away the "const".
	inBuf.buf   = (unsigned char*)theEncryptedBuffer;  
	inBuf.start = (unsigned char*)theEncryptedBuffer;
	inBuf.end   = (unsigned char*)theEncryptedBuffer + *theEncryptedLength;
	inBuf.size  = *theEncryptedLength;

	// Append to theDecryptedBuffer.
	decryptedBuf.buf   = theDecryptedBuffer;  // buf starts here.
	decryptedBuf.start = theDecryptedBuffer;  // readpos, set to start.
	decryptedBuf.end   = theDecryptedBuffer;  // writepos, set to start.
	decryptedBuf.size  = *theDecryptedLength; // Total size of buf.
	
	// Perform the decode operation; this may require multiple tries.
	while(decryptMore != GHTTPFalse && ((inBuf.end-inBuf.start) > 0))
	{
		unsigned char error = 0;
		unsigned char alertlevel = 0;
		unsigned char alertdescription = 0;

		// Perform the decode, this will decode a single SSL message at a time.
		decodeResult = matrixSslDecode(theEncryptor->mInterface, &inBuf, &decryptedBuf, 
										&error, &alertlevel, &alertdescription);
		switch(decodeResult)
		{
		case SSL_SUCCESS:          
			// A message was handled internally by matrixssl.
			// No data is appeneded to the decrypted buffer.
			if (matrixSslHandshakeIsComplete(theEncryptor->mInterface))
				theEncryptor->mSessionEstablished = GHTTPTrue;
			break;

		case SSL_PROCESS_DATA:
			// We've received app data, continue on.  
			// App data was appended to the decrypted buffer.
			break;

		case SSL_SEND_RESPONSE:
			{
			// We must send an SSL response which has been written to
			// decryptedBuf.
			// Transfer this response to the connection's sendBuffer.
			int responseSize = decryptedBuf.end - decryptedBuf.start;

			// Force disable-encryption.
			// This may seem like a hack, but it's the best way to avoid
			// unnecessary data copies without modifying matrixSSL.
			theEncryptor->mSessionEstablished = GHTTPFalse;
			ghiTrySendThenBuffer(connection, decryptedBuf.start, responseSize);
			theEncryptor->mSessionEstablished = GHTTPTrue;

			// Remove the bytes from the decrypted buffer (we don't want to
			// return them to the app).
			decryptedBuf.end = decryptedBuf.start; // bug?
			break;
			}

		case SSL_ERROR:            
			// There was an error decoding the data.
			decryptMore = GHTTPFalse;
			break;

		case SSL_ALERT:            
			// The server sent an alert.
			if (alertdescription == SSL_ALERT_CLOSE_NOTIFY)
			decryptMore = GHTTPFalse;
			break;

		case SSL_PARTIAL:          
			// Need to read more data from the socket(inbuf incomplete).
			decryptMore = GHTTPFalse;
			break;

		case SSL_FULL:             
			{
				// decodeBuffer is too small, need to increase size and try again.
				decryptMore = GHTTPFalse;
				break;
			}
		};
	}

	// Store off the lengths
	*theEncryptedLength = encryptedStartSize - (inBuf.end - inBuf.start);
	*theDecryptedLength = decryptedBuf.end - decryptedBuf.start;

	// Return status to app
	if (decodeResult == SSL_FULL)
		return GHIEncryptionResult_BufferTooSmall;
	else if (decodeResult == SSL_ERROR || decodeResult == SSL_ALERT)
		return GHIEncryptionResult_Error;

	//if ((int)(decryptedBuf.end - decryptedBuf.start) > 0)
	//	printf ("Decrypted: %d bytes\r\n", *theDecryptedLength);
	return GHIEncryptionResult_Success;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// *********************  REVOEX SSL ENCRYPTION ENGINE  ******************** //
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#elif defined(REVOEXSSL)
#include <revolution/ssl.h>

typedef struct gsRevoExInterface
{
	SSLId mId;
	SSLClientCertId mClientCertId;
	SSLRootCAId mRootCAId;
	GHTTPBool mConnected; // means "connected to socket", not "connected to remote machine"
} gsRevoExInterface;

// Init the engine
GHIEncryptionResult ghiEncryptorSslInitFunc(struct GHIConnection * connection,
											  struct GHIEncryptor  * theEncryptor)
{
	int i=0;
	gsRevoExInterface* sslInterface = NULL;

	// There is only one place where this function should be called,
	//  and it should check if the engine has been initialized
	GS_ASSERT(theEncryptor->mInitialized == GHTTPFalse);
	GS_ASSERT(theEncryptor->mInterface == NULL);

	// allocate the interface (need one per connection)
	theEncryptor->mInterface = gsimalloc(sizeof(gsRevoExInterface));
	if (theEncryptor->mInterface == NULL)
	{
		// memory allocation failed
		gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"Failed to allocate SSL interface (out of memory: %d bytes)\r\n", sizeof(gsRevoExInterface));
		return GHIEncryptionResult_Error;
	}
	memset(theEncryptor->mInterface, 0, sizeof(gsRevoExInterface));
	sslInterface = (gsRevoExInterface*)theEncryptor->mInterface;

	{
		int verifyOption = 0;
		int rcode = 0;
		
		verifyOption = SSL_VERIFY_COMMON_NAME
			| SSL_VERIFY_ROOT_CA
			| SSL_VERIFY_DATE
			| SSL_VERIFY_CHAIN
			| SSL_VERIFY_SUBJECT_ALT_NAME;

		// todo serverAddress, is this used for certificate name?
		sslInterface->mId = SSLNew((unsigned long)verifyOption, connection->serverAddress);

		rcode = SSLSetBuiltinRootCA(sslInterface->mId, SSL_ROOTCA_NINTENDO_1);
		if(rcode != SSL_ENONE){
			SSLShutdown(sslInterface->mId);
			return GHIEncryptionResult_Error;
		}

		rcode = SSLSetBuiltinClientCert(sslInterface->mId, SSL_CLIENTCERT_NINTENDO_0);
		if(rcode != SSL_ENONE){
			SSLShutdown(sslInterface->mId);
			return GHIEncryptionResult_Error;
		}
	}

	theEncryptor->mInitialized = GHTTPTrue;
	theEncryptor->mSessionStarted = GHTTPFalse;
	theEncryptor->mSessionEstablished = GHTTPFalse;
	theEncryptor->mEncryptOnBuffer = GHTTPFalse;
	theEncryptor->mEncryptOnSend = GHTTPTrue;
	theEncryptor->mLibSendsHandshakeMessages = GHTTPTrue;

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"GameSpy SSL (RevoEx engine) initialized\r\n");

	return GHIEncryptionResult_Success;
}

											  
// Destroy the engine
GHIEncryptionResult ghiEncryptorSslCleanupFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor)
{
	if (theEncryptor != NULL)
	{
		gsRevoExInterface* sslInterface = (gsRevoExInterface*)theEncryptor->mInterface;
		if (sslInterface != NULL)
		{
			SSLShutdown(sslInterface->mId);
			gsifree(sslInterface);
			theEncryptor->mInterface = NULL;
		}
		theEncryptor->mInitialized = GHTTPFalse;
		theEncryptor->mSessionStarted = GHTTPFalse;
		theEncryptor->mSessionEstablished = GHTTPFalse;
	}

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"GameSpy SSL (RevoEx) engine cleanup called\r\n");

	GSI_UNUSED(connection);

	return GHIEncryptionResult_Success;
}


GHIEncryptionResult ghiEncryptorSslStartFunc(struct GHIConnection * connection,
											struct GHIEncryptor  * theEncryptor)
{
	gsRevoExInterface* sslInterface = (gsRevoExInterface*)theEncryptor->mInterface;
	int result = 0;
	
	GS_ASSERT(theEncryptor->mSessionStarted == GHTTPFalse);
	
	// Call this only AFTER the socket has been connected to the remote server
	if (!sslInterface->mConnected)
	{
		result = SSLConnect(sslInterface->mId, connection->socket);
		if (result != SSL_ENONE)
		{
			switch(result)
			{
				case SSL_EFAILED:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"GameSpy SSL (RevoEx) SSLConnect failed (SSL_EFAILED)\r\n");
					break;
				case SSL_ESSLID:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"GameSpy SSL (RevoEx) SSLConnect failed (SSL_ESSLID)\r\n");
					break;
				default:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"GameSpy SSL (RevoEx) SSLConnect failed (Unhandled Error)\r\n");
					break;
			}
			return GHIEncryptionResult_Error;
		}
		sslInterface->mConnected = GHTTPTrue;
	}

	GS_ASSERT(sslInterface->mConnected == GHTTPTrue);
		
	// begin securing the session
	result = SSLDoHandshake(sslInterface->mId);
	if (result != SSL_ENONE)
	{
		// Check for EWOULDBLOCK conditions
		if (result == SSL_EWANT_READ || result == SSL_EWANT_WRITE)
			return GHIEncryptionResult_Success;

		switch(result)
		{
			case SSL_EFAILED:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_EFAILED)\r\n");
				break;
			case SSL_ESSLID:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_ESSLID)\r\n");
				break;
			case SSL_ESYSCALL:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_ESYSCALL)\r\n");
				break;
			case SSL_EZERO_RETURN:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_EZERO_RETURN)\r\n");
				break;
			case SSL_EWANT_CONNECT:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_EWANT_CONNECT)\r\n");
				break;
			case SSL_EVERIFY_COMMON_NAME:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_EVERIFY_COMMON_NAME)\r\n");
				break;
			case SSL_EVERIFY_CHAIN:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_EVERIFY_CHAIN)\r\n");
				break;
			case SSL_EVERIFY_ROOT_CA:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_EVERIFY_ROOT_CA)\r\n");
				break;
			case SSL_EVERIFY_DATE:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (SSL_EVERIFY_DATE)\r\n");
				break;																																				
			default:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLDoHandshake failed (Unhandled Error)\r\n");
				break;
		}
		return GHIEncryptionResult_Error;
	}

	// Success	
	theEncryptor->mSessionStarted = GHTTPTrue;
	theEncryptor->mSessionEstablished = GHTTPTrue;
	return GHIEncryptionResult_Success;
}

// Encrypt and send some data
GHIEncryptionResult ghiEncryptorSslEncryptSend(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * thePlainTextBuffer,
												 int          thePlainTextLength,
												 int *        theBytesSentOut)
{
	gsRevoExInterface* sslInterface = (gsRevoExInterface*)theEncryptor->mInterface;
	int result = 0;
	
	result = SSLWrite((int)sslInterface->mId, thePlainTextBuffer, (unsigned long)thePlainTextLength);
	if (result == SSL_EWANT_WRITE)
	{
		// signal socket error, GetLastError will return EWOULDBLOCK or EINPROGRESS
		*theBytesSentOut = -1; 
	}
	else if (result < 0)
	{
		switch(result)
		{
			case SSL_EFAILED:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLWrite failed (SSL_EFAILED)\r\n");
				break;
			case SSL_ESSLID:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLWrite failed (SSL_ESSLID)\r\n");
				break;
			case SSL_EWANT_READ:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLWrite failed (SSL_EWANT_READ)\r\n");
				break;
			case SSL_ESYSCALL:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLWrite failed (SSL_ESYSCALL)\r\n");
				break;
			case SSL_EWANT_CONNECT:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLWrite failed (SSL_EWANT_CONNECT)\r\n");
				break;
			case SSL_EZERO_RETURN:
				// send 0 is fatal in write
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLWrite failed (SSL_EZERO_RETURN)\r\n");
				break;
			default:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLWrite failed (Unhandled Error)\r\n");
				break;
		}
		return GHIEncryptionResult_Error;
	}
	else
	{
		GS_ASSERT(result > 0);
		*theBytesSentOut = result;	
	}
	GSI_UNUSED(connection);
	return GHIEncryptionResult_Success;
}

// Receive and decrypt some data
GHIEncryptionResult ghiEncryptorSslDecryptRecv(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 char * theDecryptedBuffer,
												 int *  theDecryptedLength)
{
	gsRevoExInterface* sslInterface = (gsRevoExInterface*)theEncryptor->mInterface;
	int result = 0;
	
	result = SSLRead(sslInterface->mId, theDecryptedBuffer, (unsigned long) *theDecryptedLength);
	if (result == SSL_EZERO_RETURN)
	{
		// receive 0 is not fatal
		*theDecryptedLength = 0;
	}
	else if (result == SSL_EWANT_READ)
	{
		// signal socket error, GetLastError will return EWOULDBLOCK or EINPROGRESS
		*theDecryptedLength = -1; 
	}
	else if (result < 0)
	{
		// Fatal errors
		switch(result)
		{
			case SSL_EFAILED:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLRead failed (SSL_EFAILED)\r\n");
				break;
			case SSL_ESSLID:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLRead failed (SSL_ESSLID)\r\n");
				break;
			case SSL_EWANT_WRITE:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLRead failed (SSL_EWANT_WRITE)\r\n");
				break;
			case SSL_ESYSCALL:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLRead failed (SSL_ESYSCALL)\r\n");
				break;
			case SSL_EWANT_CONNECT:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLRead failed (SSL_EWANT_CONNECT)\r\n");
				break;
			default:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"GameSpy SSL (RevoEx) SSLRead failed (Unhandled Error)\r\n");
				break;
		}
		return GHIEncryptionResult_Error;
	}
	else
	{
		GS_ASSERT(result > 0);
		*theDecryptedLength = result;
	}
	GSI_UNUSED(connection);
	return GHIEncryptionResult_Success;
}

GHIEncryptionResult ghiEncryptorSslEncryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * thePlainTextBuffer,
												 int          thePlainTextLength,
												 char *       theEncryptedBuffer,
												 int *        theEncryptedLength)
{
	GS_FAIL(); // Should never call this for RevoEx SSL!  It uses encrypt on send

	GSI_UNUSED(connection);
	GSI_UNUSED(theEncryptor);
	GSI_UNUSED(thePlainTextBuffer);
	GSI_UNUSED(thePlainTextLength);
	GSI_UNUSED(theEncryptedBuffer);
	GSI_UNUSED(theEncryptedLength);
	
	return GHIEncryptionResult_Error;
}

GHIEncryptionResult ghiEncryptorSslDecryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * theEncryptedBuffer,
												 int *        theEncryptedLength,
												 char *       theDecryptedBuffer,
												 int *        theDecryptedLength)
{
	GS_FAIL(); // Should never call this for RevoEx SSL!  It uses decrypt on recv

	GSI_UNUSED(connection);
	GSI_UNUSED(theEncryptor);
	GSI_UNUSED(theEncryptedBuffer);
	GSI_UNUSED(theEncryptedLength);
	GSI_UNUSED(theDecryptedBuffer);
	GSI_UNUSED(theDecryptedLength);

	return GHIEncryptionResult_Error;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// ***********************  TWL SSL ENCRYPTION ENGINE  **********************//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#elif defined(TWLSSL)
#include <nitro.h>
#include <nitroWiFi.h>

typedef struct GHTTPTwlCaList
{
    SOCCaInfo** mCaInfo;
    int         mCaBuiltins;  
}GHTTPTwlCaList;

typedef struct GHTTPCertificateInfo
{
    char           *mUrl;
    GHTTPTwlCaList *mCaList;
    unsigned int   mIpAddress;
    int            mInUseCounter;
} GHTTPCertificateInfo;

static DArray ghttpCertList = NULL;

typedef struct gsTwlSslInterface
{
	SOCSslConnection *mId;
	GHTTPBool mConnected; // means "connected to socket", not "connected to remote machine"
} gsTwlSslInterface;

static int ghiCompareCertUrl(const void *arrayElem, const void *newElem)
{
    char *urlInList = (char *)((GHTTPCertificateInfo*)arrayElem)->mUrl;
    char *urlNew    = (char *)((GHTTPCertificateInfo*)newElem)->mUrl;

    return strcmp(urlInList, urlNew);
}

static GHTTPTwlCaList* ghiEncryptorGetRootCAList(char *url)
{
    GHTTPBool result = GHTTPTrue;
    GHTTPCertificateInfo certInfo;
    int pos = NOT_FOUND;
    
    if (url == NULL || ghttpCertList == NULL || strcmp(url, "")==0)
    {
        // cert list is empty
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
		        "%s(@%s:%d):SSL - Null pointer or empty URL string; url=0x%X, ghttpCertList=0x%X\n", 
		        __FILE__, __FUNCTION__, __LINE__, url, ghttpCertList);		        
		return NULL;
    }
    
    // find the element in the cert list with given URL
    certInfo.mUrl = ghiGetServerAddressFromUrl(url);
    
    pos = ArraySearch(ghttpCertList, &certInfo, ghiCompareCertUrl, 0 ,gsi_true);
    
    // before continuing further free the allocated memory
    gsifree(certInfo.mUrl) ;
    
    if (pos == NOT_FOUND)
    {
        // cert list is empty
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
		        "%s(@%s:%d):SSL - Could not find Url in the Cert List. url=%s\n", 
		        __FILE__, __FUNCTION__, __LINE__, url);
		return NULL;
    }
    else
    {
        GHTTPCertificateInfo *certInfoToReturn;
        // It already is in the List so just Increment the use counter
        certInfoToReturn = (GHTTPCertificateInfo *)ArrayNth(ghttpCertList, pos);
        return certInfoToReturn -> mCaList;
    }
}

static GHTTPCertificateInfo* ghiEncryptorGetCertInfo(char *url)
{
    GHTTPBool result = GHTTPTrue;
    GHTTPCertificateInfo certInfo;
    int pos = NOT_FOUND;
    
    if (url == NULL || ghttpCertList == NULL || strcmp(url, "")==0)
    {
        // cert list is empty
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
		        "%s(@%s:%d):SSL - Null pointer or empty URL string; url=0x%X, ghttpCertList=0x%X\n", 
		        __FILE__, __FUNCTION__, __LINE__, url, ghttpCertList);		        
		return NULL;
    }
    
    // find the element in the cert list with given URL
    certInfo.mUrl = ghiGetServerAddressFromUrl(url);
    
    pos = ArraySearch(ghttpCertList, &certInfo, ghiCompareCertUrl, 0 ,gsi_true);
    
    // before continuing further free the allocated memory
    gsifree(certInfo.mUrl) ;
    
    if (pos == NOT_FOUND)
    {
        // cert list is empty
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
		        "%s(@%s:%d):SSL - Could not find Url in the Cert List. url=%s\n", 
		        __FILE__, __FUNCTION__, __LINE__, url);
		return NULL;
    }
    else
    {
        GHTTPCertificateInfo *certInfoToReturn;
        // It already is in the List so just Increment the use counter
        certInfoToReturn = (GHTTPCertificateInfo *)ArrayNth(ghttpCertList, pos);
        return certInfoToReturn;
    }
}

static void ghiFreeCertificateInfo(void *elem)
{
    GHTTPCertificateInfo *certInfo = elem;
    
    GS_ASSERT(certInfo);
    certInfo->mCaList = NULL;
    gsifree(certInfo->mUrl);
    certInfo->mUrl = NULL;
}


GHTTPBool ghiEncryptorSetTwlRootCA( char *url, void* theRootCAList)
{
    GHTTPBool result = GHTTPTrue;
    GHTTPCertificateInfo certInfo;
    int pos = NOT_FOUND;

    if (url == NULL || theRootCAList == NULL || strcmp(url, "")==0)
    {
        // cert list is empty
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "%s(@%s:%d):SSL - Null pointer or empty url string; url=0x%X, theRootCAList=0x%X\n", 
            __FILE__, __FUNCTION__, __LINE__, url, theRootCAList);

        result = GHTTPFalse;
        return result;
    }

    if (ghttpCertList == NULL)
    {
        // initialize cert list
        ghttpCertList = ArrayNew(sizeof(GHTTPCertificateInfo), 1, ghiFreeCertificateInfo);

        if (ghttpCertList == NULL)
        {
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_HotError,
                "%s(@%s:%d):SSL - Out of memory. Cannot allocate memory for ghttpCertList\n", 
                __FILE__, __FUNCTION__, __LINE__);

            result = GHTTPFalse;
            return result;
        }
    }
    certInfo.mUrl = ghiGetServerAddressFromUrl(url);
    certInfo.mCaList = theRootCAList;
    certInfo.mInUseCounter = 1;

    pos = ArraySearch(ghttpCertList, &certInfo, ghiCompareCertUrl, 0 ,gsi_true);
    if (pos == NOT_FOUND)
    {
        ArrayInsertSorted(ghttpCertList, &certInfo,ghiCompareCertUrl );
    }
    else
    {
        // free the allocated memory
        gsifree(certInfo.mUrl);

        // It already is in the List so just Increment the use counter
        ((GHTTPCertificateInfo *)ArrayNth(ghttpCertList, pos))->mInUseCounter++;
    }

    return result;
}


GHTTPBool ghiEncryptorCleanupTwlRootCA(char *url)
{
    GHTTPBool result = GHTTPTrue;
    GHTTPCertificateInfo certInfo;
    int pos = NOT_FOUND;

    if (url == NULL || ghttpCertList == NULL || strcmp(url, "")==0)
    {
        // cert list is empty
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "%s(@%s:%d):SSL - Null pointer or empty url string; url=0x%X, ghttpCertList=0x%X\n", 
            __FILE__, __FUNCTION__, __LINE__, url, ghttpCertList);

        result = GHTTPFalse;
        return result;
    }

    // find the element in the cert list with given URL
    certInfo.mUrl = ghiGetServerAddressFromUrl(url);

    pos = ArraySearch(ghttpCertList, &certInfo, ghiCompareCertUrl, 0 ,gsi_true);
    if (pos == NOT_FOUND)
    {
        // cert list is empty
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_HotError,
            "%s(@%s:%d):SSL - Could not find Url in the Cert List. url=%s\n", 
            __FILE__, __FUNCTION__, __LINE__, url);

        result = GHTTPFalse;
    }
    else
    {
        GHTTPCertificateInfo *certInfoToUpdate = NULL;

        // It already is in the List so just Increment the use counter
        certInfoToUpdate = (GHTTPCertificateInfo *)ArrayNth(ghttpCertList, pos);
        -- certInfoToUpdate->mInUseCounter;
        if (certInfoToUpdate->mInUseCounter <=0)
        {
            // remove this entry from the list
            ArrayDeleteAt(ghttpCertList, pos);
            if (ArrayLength(ghttpCertList) == 0)
            {
                // no more entries remove the list
                ArrayFree(ghttpCertList);
                ghttpCertList = NULL;
            }
        }
    }
    // Free the memory allocated for the search
    gsifree(certInfo.mUrl);
    return result;
}
    
static int SslAuthCallback(int result, SOCSslConnection* con, int level)
{
    gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Notice,
		"%s(@%s:%d):SSL - RECEIVED CERTFICATE\n",
		__FILE__, __FUNCTION__, __LINE__);
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Notice,	
	    "l:%d\n\ts:%s\n\tCN=%s\n", 
	    level, con->subject,con->cn);     
    
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Notice,	
	    "\ti:%s\n\tsn: %s\n", 
	    con->issuer, con->server_name );           
    
    
    if (result & SOC_CERT_OUTOFDATE)
    {
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
	        "%s(@%s:%d): Certificate is out-of-date - Ignoring\r\n", 
	        __FILE__, __FUNCTION__, __LINE__);
        result &= ~SOC_CERT_OUTOFDATE;
    }

    if (result & SOC_CERT_BADSERVER)
    {
       gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
	        "%s(@%s:%d): SSL: Server name does not match - (DO NOT IGNORE)\r\n", 
	        __FILE__, __FUNCTION__, __LINE__);
    }

    switch (result & SOC_CERT_ERRMASK)
    {
    case SOC_CERT_NOROOTCA:
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
	        "%s(@%s:%d): SSL: No root CA installed.(DO NOT IGNORE)\r\n", 
	        __FILE__, __FUNCTION__, __LINE__);
        break;

    case SOC_CERT_BADSIGNATURE:
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
	        "%s(@%s:%d): SSL: Bad signature.(DO NOT IGNORE)\n", 
	        __FILE__, __FUNCTION__, __LINE__);
        break;

    case SOC_CERT_UNKNOWN_SIGALGORITHM:
       gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
	        "%s(@%s:%d):SSL: Unknown signature algorithm\n",
	        __FILE__, __FUNCTION__, __LINE__);
        break;

    case SOC_CERT_UNKNOWN_PUBKEYALGORITHM:
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_WarmError,
	        "%s(@%s:%d):SSL: Unknown public key alrorithm\n",
	        __FILE__, __FUNCTION__, __LINE__);
        break;
    }

	GSI_UNUSED(con);
	GSI_UNUSED(level);

    return result; 
}

static SOCSslConnection* ghiTwlSSLInit(char *url)
{
	SOCSslConnection *sslCon = NULL;
    GHTTPCertificateInfo* certInfo = ghiEncryptorGetCertInfo(url);
	
    static u32  ssl_seed[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)];
    
    if ((certInfo != NULL) && (certInfo->mCaList != NULL))
    {
        if ((certInfo->mCaList->mCaInfo != NULL) && 
            (certInfo->mCaList->mCaBuiltins >0))
        {   
            OS_GetLowEntropyData(ssl_seed);
            SOC_AddRandomSeed(ssl_seed, sizeof(ssl_seed));

            // Check if the root ca list is initialized
    
	        sslCon = (SOCSslConnection *) gsimalloc(sizeof(SOCSslConnection));
            if (sslCon != NULL)
            {
                memset(sslCon, 0, sizeof( SOCSslConnection));
                sslCon->server_name   = certInfo->mUrl;
                sslCon->ca_info 	  = certInfo->mCaList->mCaInfo;
                sslCon->ca_builtins   = certInfo->mCaList->mCaBuiltins;
	            sslCon->auth_callback = SslAuthCallback;
            }
            else
            {
                // memory allocation failed
		        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			        "%s(@%s:%d): TWL Failed to allocate memory for ssl connection\r\n", 
			        __FILE__, __FUNCTION__, __LINE__);
            }
        }   
        else
        {
            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			    "%s(@%s:%d): TWL Uninitialized certificate list\r\n", 
			    __FILE__, __FUNCTION__, __LINE__);
        }
    }
    else
    {
		gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"%s(@%s:%d): TWL Uninitialized certificate list\r\n", 
			__FILE__, __FUNCTION__, __LINE__);
    }
    return sslCon;
    
}

GHIEncryptionResult ghiEncryptorSslInitFunc(struct GHIConnection * connection,
											struct GHIEncryptor  * theEncryptor)
{
	int i=0;
	gsTwlSslInterface* sslInterface = NULL;
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d)\r\n",
		__FILE__, __FUNCTION__, __LINE__ );

	// There is only one place where this function should be called,
	//  and it should check if the engine has been initialized
	GS_ASSERT(theEncryptor->mInitialized == GHTTPFalse);
	GS_ASSERT(theEncryptor->mInterface == NULL);

	// allocate the interface (need one per connection)
	theEncryptor->mInterface = gsimalloc(sizeof(gsTwlSslInterface));
	if (theEncryptor->mInterface == NULL)
	{
		// memory allocation failed
		gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"%s(@%s:%d):Failed to allocate SSL interface (out of memory: %d bytes)\r\n", 
			__FILE__, __FUNCTION__, __LINE__, sizeof(gsTwlSslInterface));
		return GHIEncryptionResult_Error;
	}
	memset(theEncryptor->mInterface, 0, sizeof(gsTwlSslInterface));
	sslInterface = (gsTwlSslInterface*)theEncryptor->mInterface;
	sslInterface->mId = ghiTwlSSLInit(connection->URL);
	sslInterface->mConnected = GHTTPFalse;
    if (sslInterface->mId == NULL)
    {
        // The certificate data is uninitialized
        gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		    "%s(@%s:%d):GameSpy SSL (TWL) certificate is uninitialized\r\n",
		    __FILE__, __FUNCTION__, __LINE__);
        return GHIEncryptionResult_Error;
    }
	theEncryptor->mInitialized = GHTTPTrue;
	theEncryptor->mSessionStarted = GHTTPFalse;
	theEncryptor->mSessionEstablished = GHTTPFalse;
	theEncryptor->mEncryptOnBuffer = GHTTPFalse;
	theEncryptor->mEncryptOnSend = GHTTPTrue;
	theEncryptor->mLibSendsHandshakeMessages = GHTTPTrue;

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d):GameSpy SSL (TWL engine) initialized\r\n",
		__FILE__, __FUNCTION__, __LINE__);

    GSI_UNUSED(connection);
	return GHIEncryptionResult_Success;
}

GHIEncryptionResult ghiEncryptorSslStartFunc(struct GHIConnection * connection,
											struct GHIEncryptor  * theEncryptor)
{
	gsTwlSslInterface* sslInterface = (gsTwlSslInterface*)theEncryptor->mInterface;
	int result = 0;
	
	GS_ASSERT(theEncryptor->mSessionStarted == GHTTPFalse);
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d)\r\n",
		__FILE__, __FUNCTION__, __LINE__ );

    	
	if (!sslInterface->mConnected)
	{
	    result = SOC_EnableSsl(connection->socket, sslInterface->mId);
		if (result < 0) // 0 or higher is successful
		{
			switch(result)
			{
				case SOC_EINVAL:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"%s(@%s:%d):GameSpy SSL (TWL) Invalid processing. Socket config is invalid\r\n",
						__FILE__, __FUNCTION__, __LINE__);
					break;
				case SOC_EMFILE:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"%s(@%s:%d):GameSpy SSL (TWL) Cannot create any more socket descriptors.\r\n"
						__FILE__, __FUNCTION__, __LINE__);
					break;
				
				case SOC_ENETRESET:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"%s(@%s:%d):GameSpy SSL (TWL) Socket library is not initialized.\r\n",
						__FILE__, __FUNCTION__, __LINE__);
					break;
				case SOC_ENOBUFS:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"%s(@%s:%d):GameSpy SSL (TWL)Insufficient resources.\r\n",
						__FILE__, __FUNCTION__, __LINE__);
					break;

				default:
					gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
						"%s(@%s:%d):GameSpy SSL (TWL) SOC_EnableSsl failed (Unknown Error = %d)\r\n", 
						__FILE__, __FUNCTION__, __LINE__, result);
					break;
			}
			connection->completed = GHTTPTrue;
		    connection->result = GHTTPConnectFailed;
		    connection->socketError = result;
			return GHIEncryptionResult_Error;
		}
		else
		{
	        // we successfully enabled SSL 
			// connect here
			SOCKADDR_IN address;
			
			memset(&address, 0, sizeof(SOCKADDR_IN));
			address.len = sizeof(SOCKADDR_IN);
		    address.sin_family = AF_INET;
		    
		    if (connection->proxyOverrideServer)
			    address.sin_port = htons(connection->proxyOverridePort);
		    else if(ghiProxyAddress)
			    address.sin_port = htons(ghiProxyPort);
		    else
			    address.sin_port = htons(connection->serverPort);
		    address.sin_addr.s_addr = connection->serverIP;
			
			// Set the socket to blocking.
		    //////////////////////////////////
		    if (!SetSockBlocking(connection->socket, 1))
		    {
			    connection->completed = GHTTPTrue;
			    connection->result = GHTTPSocketFailed;
			    connection->socketError = GOAGetLastError(connection->socket);
			   	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
	                    "%s(@%s:%d)return socket error %d\r\n",
	                    __FILE__, __FUNCTION__, __LINE__ ,connection->socketError );

                return GHIEncryptionResult_Error;
	    	};
		    
			result = connect(connection->socket, (SOCKADDR *)&address, sizeof(address));
		    if(gsiSocketIsError(result))
		    {
			  connection->completed = GHTTPTrue;
			  connection->result = GHTTPConnectFailed;
			  connection->socketError = GOAGetLastError(connection->socket);

  			  gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
                    "%s(@%s:%d)return socket error %d\r\n",
                    __FILE__, __FUNCTION__, __LINE__ ,connection->socketError );

			  return GHIEncryptionResult_Error;
		    }
		    

	        {
	        	int writeFlag;
	            int exceptFlag;
	            // Check if the connect has completed.
	            //////////////////////////////////////
	            result = GSISocketSelect(   connection->socket, 
	                                        NULL, 
	                                        &writeFlag, 
	                                        &exceptFlag);
	            if((gsiSocketIsError(result)) || ((result == 1) && exceptFlag))
	            {
		            connection->completed = GHTTPTrue;
		            connection->result = GHTTPConnectFailed;
		            if(gsiSocketIsError(result))
			            connection->socketError = GOAGetLastError(connection->socket);
		            else
			            connection->socketError = 0;
		            gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
	                    "%s(@%s:%d)return socket error %d\r\n",
	                    __FILE__, __FUNCTION__, __LINE__ ,connection->socketError );

		            return GHIEncryptionResult_Error;
	            }
	        }
		}
		sslInterface->mConnected = GHTTPTrue;
	}

	GS_ASSERT(sslInterface->mConnected == GHTTPTrue);
    
	// Success	
	theEncryptor->mSessionStarted = GHTTPTrue;
	theEncryptor->mSessionEstablished = GHTTPTrue;
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
	"%s(@%s:%d)return success\r\n",
	__FILE__, __FUNCTION__, __LINE__ );

	return GHIEncryptionResult_Success;
}

// Encrypt and send some data
GHIEncryptionResult ghiEncryptorSslEncryptSend(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * thePlainTextBuffer,
												 int          thePlainTextLength,
												 int *        theBytesSentOut)
{
	gsTwlSslInterface* sslInterface = (gsTwlSslInterface*)theEncryptor->mInterface;
	int result = 0;
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d)\r\n",
		__FILE__, __FUNCTION__, __LINE__ );
    gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,"Sending: %s\r\n",thePlainTextBuffer );
	result = send((int)connection->socket, thePlainTextBuffer, thePlainTextLength, 0);
    if(gsiSocketIsError(result))
	{
	    int sockErr = GOAGetLastError(connection->socket);
	    *theBytesSentOut = 0;
		switch(sockErr)
		{
			case SOC_EINVAL:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Invalid processing, socket error %d\r\n",
					__FILE__, __FUNCTION__, __LINE__, sockErr);
				break;
			case SOC_EMSGSIZE:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) The size is too large to be sent, socket error %d\r\n",
					__FILE__, __FUNCTION__, __LINE__, sockErr );
				break;
			case SOC_ENETRESET:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Socket library is not initialized, socket error %d\r\n", 
					__FILE__, __FUNCTION__, __LINE__, sockErr);
				break;
			case SOC_ENOTCONN:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Not connected, socket error %d\r\n", 
					__FILE__, __FUNCTION__, __LINE__, sockErr);
				break;
			case SOC_EWOULDBLOCK:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Cannot execute until the requested operation is blocked, %d\r\n", 
					__FILE__, __FUNCTION__, __LINE__, sockErr);
				break;
			case 0:
				// send 0 is fatal in write
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) No bytes sent, %d \r\n", 
					__FILE__, __FUNCTION__, __LINE__, sockErr);
				break;
			default:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Unknown Error, soctket error %d\r\n", 
					__FILE__, __FUNCTION__, __LINE__, sockErr);
				break;
		}

	    //connection->socketError = sockErr;
		return GHIEncryptionResult_Error;
	}
	else
	{
		GS_ASSERT(result > 0);
		*theBytesSentOut = result;	
	}
	GSI_UNUSED(connection);
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
	"%s(@%s:%d)return success\r\n",
	__FILE__, __FUNCTION__, __LINE__ );

	return GHIEncryptionResult_Success;
}

// Receive and decrypt some data
GHIEncryptionResult ghiEncryptorSslDecryptRecv(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 char * theDecryptedBuffer,
												 int *  theDecryptedLength)
{
	gsTwlSslInterface* sslInterface = (gsTwlSslInterface*)theEncryptor->mInterface;
	int result = 0;
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d)\r\n",
		__FILE__, __FUNCTION__, __LINE__ );
	
	result = recv(connection->socket, theDecryptedBuffer, *theDecryptedLength, 0);
    gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,"Result % d \n\tReceived: %s\r\n",result, theDecryptedBuffer );
	if (result == 0)
	{    
		// receive 0 is not fatal
		*theDecryptedLength = 0;
	}
	else if(gsiSocketIsError(result))
	{
	    int sockErr = GOAGetLastError(connection->socket);
	    *theDecryptedLength = 0;
		switch(sockErr)
		{
			case SOC_EINVAL:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Invalid processing, socket error %d\r\n", 
					__FILE__, __FUNCTION__, __LINE__, sockErr);
				break;
			case SOC_ENETRESET:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Socket library is not initialized, socket error %d\r\n", sockErr);
				break;
			case SOC_ENOTCONN:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Not connected, socket error %d\r\n", sockErr);
				break;
			case SOC_EWOULDBLOCK:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Cannot execute until the requested operation is blocked, %d\r\n", sockErr);
				break;
			default:
				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"%s(@%s:%d):GameSpy SSL (TWL) Unknown Error, soctket error %d\r\n", sockErr);
				break;
		}

	    //connection->socketError = sockErr;
		return GHIEncryptionResult_Error;
	}
	else
	{
		GS_ASSERT(result > 0);
		*theDecryptedLength = result;
	}
	GSI_UNUSED(connection);
		gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d)return success\r\n",
		__FILE__, __FUNCTION__, __LINE__ );

	return GHIEncryptionResult_Success;
}

GHIEncryptionResult ghiEncryptorSslEncryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * thePlainTextBuffer,
												 int          thePlainTextLength,
												 char *       theEncryptedBuffer,
												 int *        theEncryptedLength)
{
	GS_FAIL(); // Should never call this for TWL SSL!  It uses encrypt on send

	GSI_UNUSED(connection);
	GSI_UNUSED(theEncryptor);
	GSI_UNUSED(thePlainTextBuffer);
	GSI_UNUSED(thePlainTextLength);
	GSI_UNUSED(theEncryptedBuffer);
	GSI_UNUSED(theEncryptedLength);
	
	return GHIEncryptionResult_Error;
}

GHIEncryptionResult ghiEncryptorSslDecryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * theEncryptedBuffer,
												 int *        theEncryptedLength,
												 char *       theDecryptedBuffer,
												 int *        theDecryptedLength)
{
	GS_FAIL(); // Should never call this for TWL SSL!  It uses decrypt on recv

	GSI_UNUSED(connection);
	GSI_UNUSED(theEncryptor);
	GSI_UNUSED(theEncryptedBuffer);
	GSI_UNUSED(theEncryptedLength);
	GSI_UNUSED(theDecryptedBuffer);
	GSI_UNUSED(theDecryptedLength);

	return GHIEncryptionResult_Error;
}

GHIEncryptionResult ghiEncryptorSslCleanupFunc(struct GHIConnection * connection,
											   struct GHIEncryptor  * theEncryptor)
{
	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d):GameSpy SSL (Twl) engine cleanup called\r\n",
		__FILE__, __FUNCTION__, __LINE__ );
	if (theEncryptor != NULL)
	{
		gsTwlSslInterface* sslInterface = (gsTwlSslInterface *)theEncryptor->mInterface;
		if (sslInterface != NULL)
		{
            SOC_EnableSsl(connection->socket, NULL);
            if (sslInterface->mId != NULL)
            {
                gsifree(sslInterface->mId);
            }
			gsifree(sslInterface);
			theEncryptor->mInterface = NULL;
		}
		theEncryptor->mInitialized = GHTTPFalse;
		theEncryptor->mSessionStarted = GHTTPFalse;
		theEncryptor->mSessionEstablished = GHTTPFalse;
	}

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"%s(@%s:%d):GameSpy SSL (Twl) engine cleanup called\r\n", 
		__FILE__, __FUNCTION__, __LINE__);
	
	GSI_UNUSED(connection);

	return GHIEncryptionResult_Success;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// ***********************  GS SSL ENCRYPTION ENGINE  ********************** //
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#else

#include "../common/gsSSL.h"
#include "../common/gsSHA1.h"
#include "../common/gsRC4.h"
#include "../common/md5.h"


// Processor for SSL state messages (transparent to application)
static GHIEncryptionResult ghiEncryptorProcessSSLHandshake(struct GHIConnection * connection,
													  struct GHIEncryptor * encryptor,
													  GHIBuffer * data);

// SSL-ASN.1 lengths are variable length NBO integers
//    we use this utility to make data packing easier
//    example: 61 little-endian(intel) = 61 00 00 00
//                big-endian(network)  = 00 00 00 61
static void ghiEncryptorWriteNBOLength(unsigned char* buf, int value, int size)
{
	int NBO = (int)htonl(value);
	unsigned char* NBOData = (unsigned char*)&NBO;

	GS_ASSERT(size <= (int)sizeof(NBO));
	if (size > (int)sizeof(NBO))
		return; // can't write more than 4 bytes!

	// this won't work if NBO ever changes from big-endian
	memcpy(buf, NBOData+(sizeof(int)-size), (size_t)size);
}
static GHTTPBool ghiEncryptorReadNBOLength(GHIBuffer * data, int* value, int size)
{
	GS_ASSERT(size <= (int)sizeof(*value));
	if (size > (int)sizeof(*value))
		return GHTTPFalse;
	if (GHTTPFalse == ghiReadDataFromBufferFixed(data, ((char*)value)+(sizeof(int)-size), size))
		return GHTTPFalse;

	*value = (int)htonl(*value);
	return GHTTPTrue;
}

static GHTTPBool ghiEncryptorParseASN1Sequence(GHIBuffer * data, int* lenOut)
{
	char tempChar = '\0';

	if (GHTTPFalse == ghiReadDataFromBufferFixed(data, &tempChar, 1))
		return GHTTPFalse;
	if (tempChar != 0x30) // sequence start byte
		return GHTTPFalse;

	if (GHTTPFalse == ghiReadDataFromBufferFixed(data, &tempChar, 1))
		return GHTTPFalse;
	if ((tempChar & 0x80) == 0x80)
	{
		int tempInt = 0;

		// length is stored in next (tempChar^0x80) bytes
		tempChar ^= 0x80;
		if (GHTTPFalse == ghiEncryptorReadNBOLength(data, &tempInt, tempChar))
			return GHTTPFalse;
		if (tempInt > (data->len - data->pos))
			return GHTTPFalse;

		*lenOut = tempInt;
		return GHTTPTrue;
	}
	else
	{
		if ((int)tempChar > (data->len - data->pos))
			return GHTTPFalse;

		*lenOut = tempChar;
		return GHTTPTrue;
	}
}

static void ghiEncryptorGenerateEncryptionKeys(gsSSL* sslInterface)
{
	// Use the server random, client random and pre master secret
	// to compute the encryption key.  
	
	// SSLv3 style
	//  master_secret = {
	//		MD5(pre_master_secret + SHA1("A"+pre_master_secret+client_random+server_random)) +
	//		MD5(pre_master_secret + SHA1("BB"+pre_master_secret+client_random+server_random)) +
	//		MD5(pre_master_secret + SHA1("CCC"+pre_master_secret+client_random+server_random))
	//  }
	//  key_block = {
	//		MD5(master_secret + SHA1("A"+master_secret+server_random+client_random)) +
	//		MD5(master_secret + SHA1("BB"+master_secret+server_random+client_random)) +
	//		MD5(master_secret + SHA1("CCC"+master_secret+server_random+client_random))

	GSSHA1Context sha1;
	GSMD5_CTX md5;
	unsigned char temp[GSSHA1HashSize];

	unsigned int randomSize = 32;
	unsigned char keyblock[64]; // todo: support different key sizes

	// master_secret "A"
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const unsigned char*)"A", 1);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->premastersecret, GS_SSL_MASTERSECRET_LEN);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->clientRandom, randomSize);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->serverRandom, randomSize);
	GSSHA1Result(&sha1, temp);
	GSMD5Init(&md5);
	GSMD5Update(&md5, (unsigned char*)sslInterface->premastersecret, GS_SSL_MASTERSECRET_LEN);
	GSMD5Update(&md5, temp, GS_CRYPT_SHA1_HASHSIZE);
	GSMD5Final((unsigned char*)&sslInterface->mastersecret[0*GS_CRYPT_MD5_HASHSIZE], &md5);

	// master_secret "BB"
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const unsigned char*)"BB", 2);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->premastersecret, GS_SSL_MASTERSECRET_LEN);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->clientRandom, randomSize);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->serverRandom, randomSize);
	GSSHA1Result(&sha1, temp);
	GSMD5Init(&md5);
	GSMD5Update(&md5, (unsigned char*)sslInterface->premastersecret, GS_SSL_MASTERSECRET_LEN);
	GSMD5Update(&md5, temp, GS_CRYPT_SHA1_HASHSIZE);
	GSMD5Final((unsigned char*)&sslInterface->mastersecret[1*GS_CRYPT_MD5_HASHSIZE], &md5);

	// master_secret "CCC"
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const unsigned char*)"CCC", 3);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->premastersecret, GS_SSL_MASTERSECRET_LEN);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->clientRandom, randomSize);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->serverRandom, randomSize);
	GSSHA1Result(&sha1, temp);
	GSMD5Init(&md5);
	GSMD5Update(&md5, (unsigned char*)sslInterface->premastersecret, GS_SSL_MASTERSECRET_LEN);
	GSMD5Update(&md5, temp, GS_CRYPT_SHA1_HASHSIZE);
	GSMD5Final((unsigned char*)&sslInterface->mastersecret[2*GS_CRYPT_MD5_HASHSIZE], &md5);

	// key_block "A"
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const unsigned char*)"A", 1);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->serverRandom, randomSize);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->clientRandom, randomSize);
	GSSHA1Result(&sha1, temp);
	GSMD5Init(&md5);
	GSMD5Update(&md5, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSMD5Update(&md5, temp, GS_CRYPT_SHA1_HASHSIZE);
	GSMD5Final(&keyblock[0*GS_CRYPT_MD5_HASHSIZE], &md5);

	// key_block "BB"
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const unsigned char*)"BB", 2);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->serverRandom, randomSize);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->clientRandom, randomSize);
	GSSHA1Result(&sha1, temp);
	GSMD5Init(&md5);
	GSMD5Update(&md5, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSMD5Update(&md5, temp, GS_CRYPT_SHA1_HASHSIZE);
	GSMD5Final(&keyblock[1*GS_CRYPT_MD5_HASHSIZE], &md5);

	// key_block "CCC"
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const unsigned char*)"CCC", 3);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->serverRandom, randomSize);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->clientRandom, randomSize);
	GSSHA1Result(&sha1, temp);
	GSMD5Init(&md5);
	GSMD5Update(&md5, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSMD5Update(&md5, temp, GS_CRYPT_SHA1_HASHSIZE);
	GSMD5Final(&keyblock[2*GS_CRYPT_MD5_HASHSIZE], &md5);

	// key_block "DDDD"
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const unsigned char*)"DDDD", 4);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->serverRandom, randomSize);
	GSSHA1Input(&sha1, (const unsigned char*)sslInterface->clientRandom, randomSize);
	GSSHA1Result(&sha1, temp);
	GSMD5Init(&md5);
	GSMD5Update(&md5, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
	GSMD5Update(&md5, temp, GS_CRYPT_SHA1_HASHSIZE);
	GSMD5Final(&keyblock[3*GS_CRYPT_MD5_HASHSIZE], &md5);

	// key_block "EEEEE"
	// key_block "FFFFFF"
	// ... continue if more key material is needed

	// todo: support different key sizes
	// KEYBLOCK
	//    writemac[16], readmac[16], writekey[16], readkey[16], writeIV[0], readIV[0]
	memcpy(sslInterface->clientWriteMACSecret, &keyblock[0],  16);
	memcpy(sslInterface->clientReadMACSecret,  &keyblock[16], 16);
	memcpy(sslInterface->clientWriteKey,       &keyblock[32], 16);
	memcpy(sslInterface->clientReadKey,        &keyblock[48], 16);

	sslInterface->clientWriteMACLen = 16;
	sslInterface->clientReadMACLen = 16;
	sslInterface->clientWriteKeyLen = 16;
	sslInterface->clientReadKeyLen = 16;

	// Init the stream cipher
	RC4Init(&sslInterface->sendRC4, (const unsigned char*)sslInterface->clientWriteKey, sslInterface->clientWriteKeyLen);
	RC4Init(&sslInterface->recvRC4, (const unsigned char*)sslInterface->clientReadKey, sslInterface->clientReadKeyLen);

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"Generated SSL session keys\r\n");
}


// Init the engine
GHIEncryptionResult ghiEncryptorSslInitFunc(struct GHIConnection * connection,
											  struct GHIEncryptor  * theEncryptor)
{
	gsSSL* sslInterface = NULL;

	// There is only one place where this function should be called,
	//  and it should check if the engine has been initialized
	GS_ASSERT(theEncryptor->mInitialized == GHTTPFalse);
	GS_ASSERT(theEncryptor->mInterface == NULL);

	// Make sure the send buffer is large enough for the SSL handshake (handshake is <1k)
    while (connection->sendBuffer.size - connection->sendBuffer.len < (int)sizeof(gsSSLClientHelloMsg))
    {
        if ( GHTTPTrue != 
            ghiResizeBuffer(&connection->sendBuffer, connection->sendBuffer.sizeIncrement) )
        {
            return GHIEncryptionResult_BufferTooSmall;
        }
    }

	// allocate the interface (need one per connection)
	theEncryptor->mInterface = gsimalloc(sizeof(gsSSL));
	if (theEncryptor->mInterface == NULL)
	{
		// memory allocation failed
		gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
			"Failed to allocate SSL interface (out of memory: %d bytes)\r\n", sizeof(gsSSL));
		return GHIEncryptionResult_Error;
	}
	memset(theEncryptor->mInterface, 0, sizeof(gsSSL));
	sslInterface = (gsSSL*)theEncryptor->mInterface;

	theEncryptor->mInitialized = GHTTPTrue;
	theEncryptor->mSessionStarted = GHTTPFalse;
	theEncryptor->mSessionEstablished = GHTTPFalse;
	theEncryptor->mEncryptOnBuffer = GHTTPTrue;
	theEncryptor->mEncryptOnSend = GHTTPFalse;
	theEncryptor->mLibSendsHandshakeMessages = GHTTPFalse;
	GSMD5Init(&sslInterface->finishHashMD5);
	GSSHA1Reset(&sslInterface->finishHashSHA1);

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"GameSpy SSL engine initialized\r\n");

	return GHIEncryptionResult_Success;
}

// Destroy the engine
GHIEncryptionResult ghiEncryptorSslCleanupFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor)
{
	if (theEncryptor != NULL)
	{
		gsSSL* sslInterface = (gsSSL*)theEncryptor->mInterface;
		if (sslInterface != NULL)
		{
			gsifree(sslInterface);
			theEncryptor->mInterface = NULL;
		}
		theEncryptor->mInitialized = GHTTPFalse;
		theEncryptor->mSessionStarted = GHTTPFalse;
		theEncryptor->mSessionEstablished = GHTTPFalse;
	}

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"GameSpy SSL engine cleanup called\r\n");

	GSI_UNUSED(connection);

	return GHIEncryptionResult_Success;
}

// Init the engine
GHIEncryptionResult ghiEncryptorSslStartFunc(struct GHIConnection * connection,
											  struct GHIEncryptor  * theEncryptor)
{
	gsSSL* sslInterface = (gsSSL*)theEncryptor->mInterface;
	gsSSLClientHelloMsg helloMsg;
	int i=0;

	// prepare the client hello
	//    1) 
	helloMsg.header.contentType  = GS_SSL_CONTENT_HANDSHAKE;
	helloMsg.header.versionMajor = GS_SSL_VERSION_MAJOR;
	helloMsg.header.versionMinor = GS_SSL_VERSION_MINOR;

	// Set the length of the client hello message (2-byte NBO int, not including record header)
	ghiEncryptorWriteNBOLength(helloMsg.header.lengthNBO, sizeof(gsSSLClientHelloMsg)-sizeof(gsSSLRecordHeaderMsg), 2);

	helloMsg.handshakeType = GS_SSL_HANDSHAKE_CLIENTHELLO;
	helloMsg.versionMajor  = GS_SSL_VERSION_MAJOR;
	helloMsg.versionMinor  = GS_SSL_VERSION_MINOR;

	// Set the length of the client hello data (3 byte NBO int)
	//    This is the total message length MINUS the SSL record header MINUS four additional header bytes 
	ghiEncryptorWriteNBOLength(helloMsg.lengthNBO, sizeof(gsSSLClientHelloMsg)-sizeof(gsSSLRecordHeaderMsg)-4, 3);
	//ghttpEncryptorSetNBOBytesFromHBOInt(helloMsg.time, (gsi_u32)current_time(), 4); // 4 byte time value (for randomness)
	ghiEncryptorWriteNBOLength(helloMsg.time, 0, 4); // test code: no randomness

	// fill in the [rest of the] random
	//   Security Note: If a hacker is able to discern the current_time() they may be able to
	//   recreate the random bytes and recover the session key.
	Util_RandSeed(current_time());
	for (i=0; i<28; i++)
	{
		#if defined(GS_CRYPT_NO_RANDOM)
			#pragma message ("!!!WARNING: SSL Random disable for debug purposes.  SSL not secured!!!")
			helloMsg.random[i] = 0; // test code: no randomness
		#else
			helloMsg.random[i] = (unsigned char)Util_RandInt(0, 0xff);
		#endif
	}

	// store a copy of the random (used later for key generation)
	memcpy(&sslInterface->clientRandom[0], helloMsg.time, 4);
	memcpy(&sslInterface->clientRandom[4], helloMsg.random, 28);

	// todo: session resumption
	helloMsg.sessionIdLen = 0;

	// fill in cipher suite IDs
	helloMsg.cipherSuitesLength = htons(sizeof(gsi_u16)*GS_SSL_NUM_CIPHER_SUITES);
	for (i=0; i < GS_SSL_NUM_CIPHER_SUITES; i++)
		helloMsg.cipherSuites[i] = htons((unsigned short)gsSSLCipherSuites[i].mSuiteID);

	// there are no standard SSL compression methods
	helloMsg.compressionMethodLen = 1;
	helloMsg.compressionMethodList = 0;

	// We need to compute a hash of all the handshake messages
	//    Add this message to the hash (both MD5 hash and SHA1 hash)
	GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)&helloMsg+sizeof(gsSSLRecordHeaderMsg), sizeof(gsSSLClientHelloMsg)-sizeof(gsSSLRecordHeaderMsg));
	GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)&helloMsg+sizeof(gsSSLRecordHeaderMsg), sizeof(gsSSLClientHelloMsg)-sizeof(gsSSLRecordHeaderMsg));

	// Now send it (we already verified the length, so this should not fail)
	if (GHTTPFalse == ghiAppendDataToBuffer(&connection->sendBuffer, (const char*)&helloMsg, sizeof(gsSSLClientHelloMsg)))
	{
		// assert or just return?
		return GHIEncryptionResult_BufferTooSmall;
	}

	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
		"GameSpy SSL engine handshake started\r\n");

	theEncryptor->mSessionStarted = GHTTPTrue;
	return GHIEncryptionResult_Success;

}

// Encrypt some data
//    -  theEncryptedLength is reduced by the length of data written to theEncryptedBuffer
// So if the encrypted buffer is 255 bytes long and we write 50 additional bytes, we'll return 205.
GHIEncryptionResult ghiEncryptorSslEncryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * thePlainTextBuffer,
												 int          thePlainTextLength,
												 char *       theEncryptedBuffer,
												 int *        theEncryptedLength)
{
	if (theEncryptor != NULL)
	{
		gsSSL* sslInterface = (gsSSL*)theEncryptor->mInterface;
		if (sslInterface == NULL || theEncryptor->mSessionEstablished == GHTTPFalse)
		{
			// not secured yet, send as plain text
			if (thePlainTextLength > *theEncryptedLength)
				return GHIEncryptionResult_BufferTooSmall;
			memcpy(theEncryptedBuffer, thePlainTextBuffer, (size_t)thePlainTextLength);
			*theEncryptedLength += thePlainTextLength; // number of bytes written
		}
		else
		{
			// Create an SSL encrypted record
			//    The order of operations below is very important.
			//    The MAC must be computed before ciphering the plain text because
			//    theEncryptedBuffer may be the same memory location as thePlainTextBuffer

			gsSSL* sslInterface2 = (gsSSL*)theEncryptor->mInterface;
			gsSSLRecordHeaderMsg* header = NULL;
			GSMD5_CTX md5;
			int pos = 0;
			unsigned short lengthNBO = htons((unsigned short)thePlainTextLength);
			unsigned char MAC[GS_CRYPT_MD5_HASHSIZE];
            int requiredEncryptedLength = thePlainTextLength +
                (int)sizeof(gsSSLRecordHeaderMsg) +
                GS_CRYPT_MD5_HASHSIZE;

			// The SSL record adds a little overhead
            if (*theEncryptedLength < requiredEncryptedLength )
				return GHIEncryptionResult_BufferTooSmall;

			// write the SSL header
			header = (gsSSLRecordHeaderMsg*)theEncryptedBuffer;
			header->contentType = GS_SSL_CONTENT_APPLICATIONDATA;
			header->versionMajor = GS_SSL_VERSION_MAJOR;
			header->versionMinor = GS_SSL_VERSION_MINOR;
			pos += sizeof(gsSSLRecordHeaderMsg);

			// calculate the MAC
			GSMD5Init(&md5);
			GSMD5Update(&md5, sslInterface2->clientWriteMACSecret, (unsigned int)sslInterface2->clientWriteMACLen);
			GSMD5Update(&md5, (unsigned char*)GS_SSL_PAD_ONE, GS_SSL_MD5_PAD_LEN);
			GSMD5Update(&md5, sslInterface2->sendSeqNBO, sizeof(sslInterface2->sendSeqNBO));
			GSMD5Update(&md5, (unsigned char*)"\x17", 1); // content type application data
			GSMD5Update(&md5,(unsigned char*)&lengthNBO, sizeof(lengthNBO));
			GSMD5Update(&md5, (unsigned char*)thePlainTextBuffer, (unsigned int)thePlainTextLength); // **cast-away const**
			GSMD5Final(MAC, &md5); // first half of MAC

			GSMD5Init(&md5);
			GSMD5Update(&md5, sslInterface2->clientWriteMACSecret, (unsigned int)sslInterface2->clientWriteMACLen);
			GSMD5Update(&md5, (unsigned char*)GS_SSL_PAD_TWO, GS_SSL_MD5_PAD_LEN);
			GSMD5Update(&md5, MAC, GS_CRYPT_MD5_HASHSIZE);
			GSMD5Final(MAC, &md5); // complete MAC

			// apply stream cipher to data + MAC
			RC4Encrypt(&sslInterface2->sendRC4, (const unsigned char*)thePlainTextBuffer, (unsigned char*)&theEncryptedBuffer[pos], thePlainTextLength);
			pos += thePlainTextLength;
			RC4Encrypt(&sslInterface2->sendRC4, MAC, (unsigned char*)&theEncryptedBuffer[pos], GS_CRYPT_MD5_HASHSIZE);
			pos += GS_CRYPT_MD5_HASHSIZE;

			// Now that we know the final length (data+mac+pad), write it into the header
			ghiEncryptorWriteNBOLength(header->lengthNBO, (int)(pos - sizeof(gsSSLRecordHeaderMsg)), 2); 

			// adjust encrypted length
			*theEncryptedLength -= pos;

			// Update the sequence number for the next message (8-byte, NBO)
			pos = 7; // **changing the semantic of variable "pos"
			do 
			{
				//int carry = 0;
				if (sslInterface2->sendSeqNBO[pos] == 0xFF) // wraparound means carry
				{
					//carry = 1;
					sslInterface2->sendSeqNBO[pos] = 0;
					pos -= 1;
				}
				else
				{
					sslInterface2->sendSeqNBO[pos] += 1;
					pos = 0; // end addition
				}
			} while(pos >= 0);
		}
	}

	GSI_UNUSED(connection);
	return GHIEncryptionResult_Success;
}

// Decrypt some data
//    -  During the handshaking process, this may result in data being appended to the send buffer
//    -  Data may be left in the encrypted buffer
//    -  theEncryptedLength becomes the length of data read from theEncryptedBuffer
//    -  theDecryptedLength becomes the length of data written to theDecryptedBuffer
GHIEncryptionResult ghiEncryptorSslDecryptFunc(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * theEncryptedBuffer,
												 int *        theEncryptedLength,
												 char *       theDecryptedBuffer,
												 int *        theDecryptedLength)
{
	gsSSL* sslInterface = NULL;
	int readPos = 0;
	int writePos = 0;

	// Make sure we have a valid encryptor
	GS_ASSERT(theEncryptor != NULL);
	GS_ASSERT(theEncryptor->mInterface != NULL);
	if (theEncryptor == NULL || theEncryptor->mInterface == NULL)
	{
		// no encryption set? copy as plain text
		memcpy(theDecryptedBuffer, theEncryptedBuffer, (size_t)(*theEncryptedLength));
		*theDecryptedLength = *theEncryptedLength;
		*theEncryptedLength = 0; // no bytes remaining
		return GHIEncryptionResult_Success;
	}

	sslInterface = (gsSSL*)theEncryptor->mInterface;
	if (sslInterface == NULL)
		return GHIEncryptionResult_Error;

	// Read each SSL message from the stream (leave partial messages)
	while(readPos < *theEncryptedLength)
	{
		gsSSLRecordHeaderMsg* header = NULL;
		unsigned short length = 0;
		GHIEncryptionResult result;

		// make sure we have the complete record header
		if ((*theEncryptedLength-readPos) < (int)sizeof(gsSSLRecordHeaderMsg))
			break;
		header = (gsSSLRecordHeaderMsg*)&theEncryptedBuffer[readPos];

		// make sure we have the complete record data
		// Warning:  Convert the length in two steps to avoid issues with byte order
		//    BAD!! -> length = ntohs((header->lengthNBO[0] | (header->lengthNBO[1] << 8)));
		//length = (unsigned short)(header->lengthNBO[0] | (header->lengthNBO[1] << 8));
		memcpy(&length, &header->lengthNBO[0], sizeof(length));
		length = ntohs(length);
		if ( *theEncryptedLength < (readPos + length +(int)sizeof(gsSSLRecordHeaderMsg)))
			break; // wait for more data

		// if we have to decrypt, make sure there is room in the decrypt buffer
		if (connection->encryptor.mSessionEstablished)
		{
			if ((*theDecryptedLength-writePos) < length)
			{
				*theEncryptedLength = readPos; // bytes read *NOT* bytes remaining
				*theDecryptedLength = writePos; // bytes written

				if (*theDecryptedLength>0)
					return GHIEncryptionResult_Success;
				else
					return GHIEncryptionResult_BufferTooSmall;
			}
		}

		//readPos += sizeof(gsSSLRecordHeaderMsg);

		// process the record data
		switch(header->contentType)
		{
		case GS_SSL_CONTENT_HANDSHAKE:
			{
				GHIBuffer data;

				// Apply stream cipher if the session has been established
				readPos += sizeof(gsSSLRecordHeaderMsg);
				if (connection->encryptor.mSessionEstablished)
					RC4Encrypt(&sslInterface->recvRC4, (const unsigned char*)&theEncryptedBuffer[readPos], (unsigned char*)&theEncryptedBuffer[readPos], length);

				ghiInitReadOnlyBuffer(connection, &data, &theEncryptedBuffer[readPos], length);

				gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
					"SSL handshake message received\r\n");

				result = ghiEncryptorProcessSSLHandshake(connection, theEncryptor, &data);
				if (result != GHIEncryptionResult_Success)
					return result; // error!

				break;
			}
		case GS_SSL_CONTENT_APPLICATIONDATA:
			{
				// make sure there is enough room to receive this data
				if ( (*theDecryptedLength-writePos) < length)
				{
				}

				// Apply stream cipher if the session has been established
				readPos += sizeof(gsSSLRecordHeaderMsg);
				if (connection->encryptor.mSessionEstablished)
					RC4Encrypt(&sslInterface->recvRC4, (const unsigned char*)&theEncryptedBuffer[readPos], (unsigned char*)&theEncryptedBuffer[readPos], length);

				// verify MAC and pad
				// verifyMAC();

				// copy to decrypted buffer so HTTP layer can process
				memcpy(theDecryptedBuffer+writePos, &theEncryptedBuffer[readPos], (size_t)(length - GS_CRYPT_MD5_HASHSIZE));
				writePos += length - GS_CRYPT_MD5_HASHSIZE;
				break;
			}

		case GS_SSL_CONTENT_CHANGECIPHERSPEC:
			readPos += sizeof(gsSSLRecordHeaderMsg);
			//if(readPos > *theEncryptedLength)
			//	_asm int 3;
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Misc, GSIDebugLevel_Debug,
				"SSL change cipher spec message received\r\n");

			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_Notice,
				"SSL: Incoming traffic now encrypted\r\n");
			connection->encryptor.mSessionEstablished = GHTTPTrue;
			break;

		case GS_SSL_CONTENT_ALERT:
			readPos += sizeof(gsSSLRecordHeaderMsg);
			//if(readPos > *theEncryptedLength)
			//	_asm int 3;

			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_WarmError,
				"SSL received unhandled ALERT\r\n");
			// server alert
			break;

		default:
			readPos += sizeof(gsSSLRecordHeaderMsg);
			return GHIEncryptionResult_Error; // unhandled content type
		};

		readPos += length;
	};

	// remove read bytes from the stream
	*theEncryptedLength = readPos; // bytes read *NOT* bytes remaining
	*theDecryptedLength = writePos; // bytes written

	if (*theEncryptedLength < 0)
		return GHIEncryptionResult_Error;
	else
		return GHIEncryptionResult_Success;
}

static GHTTPBool ghiCertificateChainIsValid(gsSSL* sslInterface)
{
	GSI_UNUSED(sslInterface);

	return GHTTPTrue;
}

#define CHECK(a) { if (GHTTPFalse == a) return GHIEncryptionResult_Error; }


// Programmer note: 
//    The structure of these SSL handshake messages may seem a bit cryptic, 
//    due to their variable length data items.  Refer to the ASN1/DER encoding guide
//    for tag specifics.
GHIEncryptionResult ghiEncryptorProcessSSLHandshake(struct GHIConnection * connection,
													  struct GHIEncryptor * encryptor,
													  GHIBuffer * data)
{
	// There may be multiple messages within the handshake message
	//  length must be completely used, otherwise it's a protocol error
	gsSSL* sslInterface = (gsSSL*)encryptor->mInterface;

	while(data->pos < data->len)
	{
		// Parse each SSL handshake message (there may be multiple)
		int  messageStart = data->pos;
		char messageType = 0;
		CHECK(ghiReadDataFromBufferFixed(data, &messageType, 1));

		if (messageType == GS_SSL_HANDSHAKE_SERVERHELLO)
		{
			int totalMsgLen = 0; // length of header + data
			int msgDataLen = 0;  // length of data
			int tempInt = 0;
			char tempChar = '\0';

			// make sure we don't have a session already (e.g. dupe hello message)
			if (sslInterface->sessionLen != 0)
				return GHIEncryptionResult_Error; // abort connection

			CHECK(ghiEncryptorReadNBOLength(data, &msgDataLen, 3));

			// check reported size against the actual bytes remaining
			if (msgDataLen > (data->len - data->pos))
				return GHIEncryptionResult_Error; // abort connection

			// skip SSL version
			//    (length check not required because we did that above)
			data->pos += 2;

			// store server random (used for key generation)
			CHECK(ghiReadDataFromBufferFixed(data, (char*)&sslInterface->serverRandom[0], 32));

			// store session information (length followed by data)
			CHECK(ghiReadDataFromBufferFixed(data, &tempChar, 1));
			CHECK(ghiReadDataFromBufferFixed(data, (char*)sslInterface->sessionData, tempChar));
			sslInterface->sessionLen = (int)tempChar;

			// store cipher suite
			CHECK(ghiEncryptorReadNBOLength(data, &tempInt, 2));
			sslInterface->cipherSuite = (unsigned short)tempInt;

			// skip compression algorithms (should always be 0x00 since we don't support any!)
			CHECK(ghiReadDataFromBufferFixed(data, &tempChar, 1));
			if (tempChar != 0x00)
				return GHIEncryptionResult_Error;

			// add it to the running handshake hash
			totalMsgLen = data->pos - messageStart;
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)&data->data[messageStart], (unsigned int)totalMsgLen);
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)&data->data[messageStart], (unsigned int)totalMsgLen);
		}
		else if (messageType == GS_SSL_HANDSHAKE_CERTIFICATE)
		{
			int msgLength = 0;    // combined length of the message (size in SSL message header)
			int certListLen = 0;  // combined length of all certificates
			int totalMsgLen = 0;  // our calculated msg length (for handshake hashing)

			int certCount = 0;
			int certListEndPos = 0;

			CHECK(ghiEncryptorReadNBOLength(data, &msgLength, 3));
			CHECK(ghiEncryptorReadNBOLength(data, &certListLen, 3));
			if (msgLength != certListLen + 3)
				return GHIEncryptionResult_Error;

			// make sure we don't have the certificates already (e.g. dupe message)
			//if (sslInterface->certificateArray != NULL)
			//	return GHIEncryptionResult_Error; // abort connection

			// make sure we have enough data to cover the certificate list 
			certListEndPos = data->pos + certListLen;
			if (certListLen > (data->len - data->pos))
				return GHIEncryptionResult_Error;

			// read the certificates
			while(data->pos < certListEndPos)
			{
				int certLength = 0;
				int certStartPos = 0;

				int temp = 0;
				int version = 0;

				// Must start with a 3 byte length
				CHECK(ghiEncryptorReadNBOLength(data, &certLength, 3));

				// Make sure we have enough data to cover this certificate
				if (certLength > (data->len - data->pos))
					return GHIEncryptionResult_Error; // certificate too big

				// 0xFFFF is max message size in SSL v3.0, we don't currently support
				// split messages
				if (certLength > 0xFFFF) 
					return GHIEncryptionResult_Error;

				certStartPos = data->pos; // remember this for a shortcut later
				certCount++;

				// make a copy of the certificate data
				//certCopy = gsimalloc(certLength);
				//if (certCopy == NULL)
				//{
				//	gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Memory, GSIDebugLevel_WarmError,
				//		"SSL failed to allocate certificate #%d (out of memory)\r\n", certCount);
				//	return GHIEncryptionResult_Error;
				//}
				//memcpy(certCopy, &data[browsePos], certLength);
				//ArrayAppend(sslInterface->certificateArray, &certCopy);

				// The first certificate holds the server's public key
				if (certCount == 1)
				{
					// X.509 format is rather convoluted.  Since we only support
					// one variation anyways, I'm hardcoding the specific values
					// we require.  Anything else is a protocol error.
					//    0x30 marks the start of a sequence.  next byte is a length field size
					//    0x82 is a length tag, meaning the next two bytes contain the length
					//    0x81 is the same thing, only the next one byte contains the length
					//    The other values usually denote required types

					// Certificate SEQUENCE
					int seqLen = 0;
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					// todo: verify reported length of this sequence

					// TBSCertificate SEQUENCE
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					// todo: verify reported length of this sequence

					// EXPLICIT Version (must be one of: 0x03,0x02,0x01)
					if (5 > (data->len - data->pos)) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0xa0) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0x03) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0x02) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0x01) return GHIEncryptionResult_Error;
					version = (unsigned char)data->data[data->pos++];

					// Serial Number (variable length, with 2-byte length field)
					if ((unsigned char)data->data[data->pos++] != 0x02) return GHIEncryptionResult_Error;
					temp = (unsigned char)data->data[data->pos++]; // len of serial number
					if (data->pos + temp > certListEndPos) return GHIEncryptionResult_Error;
					data->pos += temp; // skip the serial number

					// Signature algorithm identifier SEQUENCE 
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					data->pos += seqLen; // skip algorithm ID (todo: verify signatures)

					// Issuer Name
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					data->pos += seqLen; // skip the issuer name sequence

					// Validity
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					data->pos += seqLen; // skip the validity sequence

					// Subject Name
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					data->pos += seqLen; // skip the subject name sequence

					// Subject Public Key Info
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					//     AlgorithmIdentifier
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));
					if (seqLen != 0x0d) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0x06) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0x09) return GHIEncryptionResult_Error;
					if (0 != memcmp(&data->data[data->pos], gsSslRsaOid, sizeof(gsSslRsaOid)))
						return GHIEncryptionResult_Error; // only RSA certs are supported
					data->pos += sizeof(gsSslRsaOid);
					if ((unsigned char)data->data[data->pos++] != 0x05) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0x00) return GHIEncryptionResult_Error;

					//     Bitstring (subject public key)
					if (2 > (certListEndPos - data->pos)) return GHIEncryptionResult_Error;
					if ((unsigned char)data->data[data->pos++] != 0x03) return GHIEncryptionResult_Error; // bitstring
					if ((unsigned char)data->data[data->pos++] != 0x81) return GHIEncryptionResult_Error; // 1 byte len field
					if (temp > (certListEndPos - data->pos)) return GHIEncryptionResult_Error;
					temp = (unsigned char)data->data[data->pos++]; // remaining data size (check or ignore)

					if ((unsigned char)data->data[data->pos++] != 0x00) return GHIEncryptionResult_Error;

					//     Start of the public key modulus
					CHECK(ghiEncryptorParseASN1Sequence(data, &seqLen));

					// Read out the public key modulus
					if (data->data[data->pos++] != 0x02) return GHIEncryptionResult_Error; // integer tag
					if ((data->data[data->pos]&0x80)==0x80) // ASN1 variable length field
					{
						int lensize = data->data[data->pos++]&0x7f;
						if (lensize > 4)
							return GHIEncryptionResult_Error;
						temp = 0;
						while(lensize-- > 0)
							temp = (temp << 8) | (unsigned char)data->data[data->pos++];
					}
					else
					{
						temp = (unsigned char)data->data[data->pos++];
					}
					if (data->pos + temp > certListEndPos) return GHIEncryptionResult_Error;
					if (data->data[data->pos++] != 0x00) return GHIEncryptionResult_Error; // ignore bits must be 0x00
					if (temp-1 > (int)(GS_LARGEINT_BINARY_SIZE/sizeof(char)))
						return GHIEncryptionResult_Error;
					sslInterface->serverpub.modulus.mLength = (unsigned int)((temp-1)/GS_LARGEINT_DIGIT_SIZE_BYTES); //-1 to subtract the previous 0x00 byte
					gsLargeIntSetFromMemoryStream(&sslInterface->serverpub.modulus, (const gsi_u8*)&data->data[data->pos], (gsi_u32)temp-1);
					data->pos += temp-1;

					// Read out the public key exponent
					if (data->data[data->pos++] != 0x02) return GHIEncryptionResult_Error; // integer
					if ((data->data[data->pos]&0x80)==0x80)
					{
						int lensize = data->data[data->pos++]&0x7f;
						if (lensize > 4)
							return GHIEncryptionResult_Error;
						temp = 0;
						while(lensize-- > 0)
							temp = (temp << 8) | (unsigned char)data->data[data->pos++];
					}
					else
					{
						temp = (unsigned char)data->data[data->pos++];
					}
					if (data->pos + temp > certListEndPos) return GHIEncryptionResult_Error;
					if (temp == 0) return GHIEncryptionResult_Error; // no exponent?
					if (temp > (int)(GS_LARGEINT_BINARY_SIZE/sizeof(char)))
						return GHIEncryptionResult_Error;
					sslInterface->serverpub.exponent.mLength = (unsigned int)(((temp-1)/GS_LARGEINT_DIGIT_SIZE_BYTES)+1); // ceiling of temp/4
					gsLargeIntSetFromMemoryStream(&sslInterface->serverpub.exponent, (const gsi_u8*)&data->data[data->pos], (gsi_u32)temp);
					data->pos += temp;
				}

				// update the position
				data->pos = certStartPos + certLength;

				GSI_UNUSED(version); 
			}
			if (data->pos != certListEndPos)
				return GHIEncryptionResult_Error; // bytes hanging off the end!

			// todo: verify certificate chain
			//       first certificate is the server's, the rest likely belong to CA
			if  (GHTTPFalse == ghiCertificateChainIsValid(sslInterface))
				return GHIEncryptionResult_Error;

			// add it to the running handshake hash
			totalMsgLen = data->pos - messageStart;
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)&data->data[messageStart], (unsigned int)totalMsgLen);
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)&data->data[messageStart], (unsigned int)totalMsgLen);
		}
		else if (messageType == GS_SSL_HANDSHAKE_SERVERHELLODONE)
		{
			// Process the hello done
			// Respond with 3 messages
			//    ClientKeyExchange
			//    ChangeCipherSpec
			//    Finished (final handshake)
			int i=0;

			gsSSLClientKeyExchangeMsg* clientKeyExchange = NULL;
			gsSSLRecordHeaderMsg* changeCipherSpec = NULL;
			gsSSLRecordHeaderMsg* finalHandshake = NULL;

			unsigned char temp[7];
			unsigned char hashTempMD5[GS_CRYPT_MD5_HASHSIZE];
			unsigned char hashTempSHA1[GS_CRYPT_SHA1_HASHSIZE];
			int tempInt = 0;

			// ServerHelloDone has a zero length data field
			CHECK(ghiEncryptorReadNBOLength(data, &tempInt, 3));
			if (tempInt != 0x00) return GHIEncryptionResult_Error;

			// add it to the running handshake hash
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)&data->data[messageStart], (unsigned int)(data->pos - messageStart));
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)&data->data[messageStart], (unsigned int)(data->pos - messageStart));

			// Make sure there is room in the send buffer for the response messages
            tempInt = (int)(sizeof(gsSSLClientKeyExchangeMsg)
                + sslInterface->serverpub.modulus.mLength*GS_LARGEINT_DIGIT_SIZE_BYTES
                + sizeof(gsSSLRecordHeaderMsg)
                + 1
                + sizeof(gsSSLRecordHeaderMsg)
                + 1
                + 3
                + GS_CRYPT_MD5_HASHSIZE
                + GS_CRYPT_SHA1_HASHSIZE
                + GS_CRYPT_MD5_HASHSIZE);
            while (connection->sendBuffer.size - connection->sendBuffer.len < tempInt)
			{
				// not enough room in send buffer, try to grow it
				if (GHTTPFalse == ghiResizeBuffer(&connection->sendBuffer, connection->sendBuffer.sizeIncrement))
					return GHIEncryptionResult_Error; 
			}

			// 1) Client key exchange,
			//    create the pre-master-secret
			sslInterface->premastersecret[0] = GS_SSL_VERSION_MAJOR;
			sslInterface->premastersecret[1] = GS_SSL_VERSION_MINOR;
			for (i=2; i<GS_SSL_MASTERSECRET_LEN; i++)
			{
				#if defined(GS_CRYPT_NO_RANDOM)
					#pragma message ("!!!WARNING: SSL Random disable for debug purposes.  SSL not secured!!!")
					// Use zero as the random so we can packet sniff for debug
					//   warning! : this compromises the SSL security
					sslInterface->premastersecret[i] = 0; // rand()
				#else
					Util_RandSeed(current_time());
					sslInterface->premastersecret[i] = (unsigned char)(Util_RandInt(0, 0x0100)); // range = [0...FF]
				#endif
			}

			clientKeyExchange = (gsSSLClientKeyExchangeMsg*)&connection->sendBuffer.data[connection->sendBuffer.len];
			connection->sendBuffer.len += sizeof(gsSSLClientKeyExchangeMsg);
			clientKeyExchange->header.contentType = GS_SSL_CONTENT_HANDSHAKE;
			clientKeyExchange->header.versionMajor = GS_SSL_VERSION_MAJOR;
			clientKeyExchange->header.versionMinor = GS_SSL_VERSION_MINOR;
			ghiEncryptorWriteNBOLength(clientKeyExchange->header.lengthNBO, (int)(sslInterface->serverpub.modulus.mLength*GS_LARGEINT_DIGIT_SIZE_BYTES+4), 2);
			clientKeyExchange->handshakeType = GS_SSL_HANDSHAKE_CLIENTKEYEXCHANGE;
			ghiEncryptorWriteNBOLength(clientKeyExchange->lengthNBO, (int)(sslInterface->serverpub.modulus.mLength*GS_LARGEINT_DIGIT_SIZE_BYTES), 3);
			//    encrypt the preMasterSecret using the server's public key (store result in sendbuffer)
			gsCryptRSAEncryptBuffer(&sslInterface->serverpub, sslInterface->premastersecret, 
				GS_SSL_MASTERSECRET_LEN, (unsigned char*)&connection->sendBuffer.data[connection->sendBuffer.len]);
			connection->sendBuffer.len += sslInterface->serverpub.modulus.mLength*GS_LARGEINT_DIGIT_SIZE_BYTES;

			// add it to the running handshake hash
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)clientKeyExchange+sizeof(gsSSLRecordHeaderMsg), 
				sizeof(gsSSLClientKeyExchangeMsg) - sizeof(gsSSLRecordHeaderMsg) + 
				sslInterface->serverpub.modulus.mLength*GS_LARGEINT_DIGIT_SIZE_BYTES);
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)clientKeyExchange+sizeof(gsSSLRecordHeaderMsg), 
				sizeof(gsSSLClientKeyExchangeMsg) - sizeof(gsSSLRecordHeaderMsg) +
				sslInterface->serverpub.modulus.mLength*GS_LARGEINT_DIGIT_SIZE_BYTES);


			// 2) change cipher spec
			changeCipherSpec = (gsSSLRecordHeaderMsg*)&connection->sendBuffer.data[connection->sendBuffer.len];
			changeCipherSpec->contentType = GS_SSL_CONTENT_CHANGECIPHERSPEC;
			changeCipherSpec->versionMajor = GS_SSL_VERSION_MAJOR;
			changeCipherSpec->versionMinor = GS_SSL_VERSION_MINOR;
			changeCipherSpec->lengthNBO[0] = 0;
			changeCipherSpec->lengthNBO[1] = 1; // always one byte length 
			connection->sendBuffer.len += sizeof(gsSSLRecordHeaderMsg);
			connection->sendBuffer.data[connection->sendBuffer.len++] = 0x01; // always set to 0x01
			// DO NOT add it to the running handshake hash (its content is not GS_SSL_CONTENT_HANDSHAKE)

			// Calculate the encryption keys
			ghiEncryptorGenerateEncryptionKeys(sslInterface);

			// 3) final handshake message (encrypted)
			finalHandshake = (gsSSLRecordHeaderMsg*)&connection->sendBuffer.data[connection->sendBuffer.len];
			finalHandshake->contentType = GS_SSL_CONTENT_HANDSHAKE;
			finalHandshake->versionMajor = GS_SSL_VERSION_MAJOR;
			finalHandshake->versionMinor = GS_SSL_VERSION_MINOR;
			finalHandshake->lengthNBO[0] = 0;
			finalHandshake->lengthNBO[1] = 56; // handshake type(1)+handshake lenNBO(3)+SHA1(20)+MD5(16)+MAC(16)
			connection->sendBuffer.len += sizeof(gsSSLRecordHeaderMsg);
			connection->sendBuffer.data[connection->sendBuffer.len++] = GS_SSL_HANDSHAKE_FINISHED;
			ghiEncryptorWriteNBOLength((unsigned char*)&connection->sendBuffer.data[connection->sendBuffer.len], 36, 3);
			connection->sendBuffer.len += 3;


			// MD5(master_secret + pad2 + MD5(handshake_messages+"CLNT"+master_secret+pad1))
			// SHA1(master_secret + pad2 + SHA1(handshake_messages+"CLNT"+master_secret+pad1))
			// prepare the final hashes (inner hashes)
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)GS_SSL_CLIENT_FINISH_VALUE, 4);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)GS_SSL_PAD_ONE, GS_SSL_MD5_PAD_LEN);
			GSMD5Final(hashTempMD5, &sslInterface->finishHashMD5);

			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)GS_SSL_CLIENT_FINISH_VALUE, 4);
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)GS_SSL_PAD_ONE, GS_SSL_SHA1_PAD_LEN);
			GSSHA1Result(&sslInterface->finishHashSHA1, hashTempSHA1);

			// prepare the final hashes (outer hashes)
			GSMD5Init(&sslInterface->finishHashMD5);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)GS_SSL_PAD_TWO, GS_SSL_MD5_PAD_LEN);
			GSMD5Update(&sslInterface->finishHashMD5, hashTempMD5, GS_CRYPT_MD5_HASHSIZE);
			GSMD5Final(hashTempMD5, &sslInterface->finishHashMD5);

			GSSHA1Reset(&sslInterface->finishHashSHA1);
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)sslInterface->mastersecret, GS_SSL_MASTERSECRET_LEN);
			GSSHA1Input(&sslInterface->finishHashSHA1, (unsigned char*)GS_SSL_PAD_TWO, GS_SSL_SHA1_PAD_LEN);
			GSSHA1Input(&sslInterface->finishHashSHA1, hashTempSHA1, GS_CRYPT_SHA1_HASHSIZE);
			GSSHA1Result(&sslInterface->finishHashSHA1, hashTempSHA1);

			// copy results into the sendbuffer
			memcpy(&connection->sendBuffer.data[connection->sendBuffer.len], hashTempMD5, GS_CRYPT_MD5_HASHSIZE);
			connection->sendBuffer.len += GS_CRYPT_MD5_HASHSIZE;
			memcpy(&connection->sendBuffer.data[connection->sendBuffer.len], hashTempSHA1, GS_CRYPT_SHA1_HASHSIZE);
			connection->sendBuffer.len += GS_CRYPT_SHA1_HASHSIZE;

			// output the message MAC  (hash(MAC_write_secret+pad_2+ hash(MAC_write_secret+pad_1+seq_num+length+content)));
			// Re-using the finishHashMD5 since it has already been allocated
			GSMD5Init(&sslInterface->finishHashMD5);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)sslInterface->clientWriteMACSecret, GS_CRYPT_MD5_HASHSIZE);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)GS_SSL_PAD_ONE, GS_SSL_MD5_PAD_LEN);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)sslInterface->sendSeqNBO, 8);
			temp[0] = 0x16;
			temp[1] = (unsigned char)((GS_CRYPT_MD5_HASHSIZE+GS_CRYPT_SHA1_HASHSIZE+4)>>8);
			temp[2] = (unsigned char)((GS_CRYPT_MD5_HASHSIZE+GS_CRYPT_SHA1_HASHSIZE+4));
			//temp[1] = (unsigned char)(htons(GS_CRYPT_MD5_HASHSIZE+GS_CRYPT_SHA1_HASHSIZE));
			//temp[2] = (unsigned char)(htons(GS_CRYPT_MD5_HASHSIZE+GS_CRYPT_SHA1_HASHSIZE+4)>>8);
			temp[3] = 0x14; // 20-bytes of data (MD5+SHA1)
			temp[4] = 0x00; // 3-byte length NBO
			temp[5] = 0x00; // ..
			temp[6] = 0x24; // ..
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)&temp, 7);
			GSMD5Update(&sslInterface->finishHashMD5, hashTempMD5, GS_CRYPT_MD5_HASHSIZE);   // content part 1
			GSMD5Update(&sslInterface->finishHashMD5, hashTempSHA1, GS_CRYPT_SHA1_HASHSIZE); // content part 2
			GSMD5Final(hashTempMD5, &sslInterface->finishHashMD5);
			GSMD5Init(&sslInterface->finishHashMD5); // reset for outer hash
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)sslInterface->clientWriteMACSecret, GS_CRYPT_MD5_HASHSIZE);
			GSMD5Update(&sslInterface->finishHashMD5, (unsigned char*)GS_SSL_PAD_TWO, GS_SSL_MD5_PAD_LEN);
			GSMD5Update(&sslInterface->finishHashMD5, hashTempMD5, GS_CRYPT_MD5_HASHSIZE);
			GSMD5Final(hashTempMD5, &sslInterface->finishHashMD5);

			memcpy(&connection->sendBuffer.data[connection->sendBuffer.len], hashTempMD5, GS_CRYPT_MD5_HASHSIZE);
			connection->sendBuffer.len += GS_CRYPT_MD5_HASHSIZE;

			// increment sequence each time we send a message
			//   ...assume NBO is bigendian for simplicity
			memset(sslInterface->sendSeqNBO, 0, sizeof(sslInterface->sendSeqNBO));
			ghiEncryptorWriteNBOLength(&sslInterface->sendSeqNBO[4], 1, 4);

			// now encrypt the message (not including record header)
			RC4Encrypt(&sslInterface->sendRC4, 
				((unsigned char*)finalHandshake)+sizeof(gsSSLRecordHeaderMsg), 
				((unsigned char*)finalHandshake)+sizeof(gsSSLRecordHeaderMsg),
				56);
		}
		else if (messageType == GS_SSL_HANDSHAKE_FINISHED)
		{
			// process server finished and verify hashes
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_Notice,
				"SSL: todo - verify server finished hash\r\n");
			data->pos = data->len; 
		}
		else
		{
			gsDebugFormat(GSIDebugCat_HTTP, GSIDebugType_Network, GSIDebugLevel_WarmError,
				"SSL received unexpected handshake message type: %d\r\n", messageType);
			return GHIEncryptionResult_Error; // abort connection
		}
	}

	GSI_UNUSED(connection);

	if (data->pos == data->len)
		return GHIEncryptionResult_Success;
	else
		return GHIEncryptionResult_Error; // too many or too few bytes, protocol error!
}

GHIEncryptionResult ghiEncryptorSslEncryptSend(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 const char * thePlainTextBuffer,
												 int          thePlainTextLength,
												 int *        theBytesSentOut)
{
	GS_FAIL(); // Should never call this for GameSpy SSL!  It uses encrypt on buffer

	GSI_UNUSED(connection);
	GSI_UNUSED(theEncryptor);
	GSI_UNUSED(thePlainTextBuffer);
	GSI_UNUSED(thePlainTextLength);
	GSI_UNUSED(theBytesSentOut);

	return GHIEncryptionResult_Error;
}

GHIEncryptionResult ghiEncryptorSslDecryptRecv(struct GHIConnection * connection,
												 struct GHIEncryptor  * theEncryptor,
												 char * theDecryptedBuffer,
												 int *  theDecryptedLength)
{
	GS_FAIL(); // Should never call this for GameSpy SSL!  It uses decrypt on buffer

	GSI_UNUSED(connection);
	GSI_UNUSED(theEncryptor);
	GSI_UNUSED(theDecryptedBuffer);
	GSI_UNUSED(theDecryptedLength);

	return GHIEncryptionResult_Error;	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // encryption engine switch

