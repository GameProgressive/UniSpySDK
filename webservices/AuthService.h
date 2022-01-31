///////////////////////////////////////////////////////////////////////////////
// File:	AuthService.h
// SDK:		GameSpy Authentication Service SDK
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.

#ifndef __AUTHSERVICE_H__
#define __AUTHSERVICE_H__


// ***** Authentication web services.
//
// ***** PUBLIC INTERFACE AT THE BOTTOM OF THE FILE

#include "../common/gsCore.h"
#include "../common/gsCrypt.h"
#include "../common/gsLargeInt.h"
#include "../ghttp/ghttpSoap.h"

#if defined(__cplusplus)
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern gsi_bool __isAuthenticated;

// URL for sc services.
#define WS_LOGIN_MAX_URL_LEN		  (128)
extern char wsAuthServiceURL[WS_LOGIN_MAX_URL_LEN];

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define	WSLogin_PARTNERCODE_GAMESPY        0
#define	WSLogin_NAMESPACE_SHARED_NONUNIQUE 0
#define	WSLogin_NAMESPACE_SHARED_UNIQUE    1

typedef enum WSCreateUserAccountValue {
	// Login response code (mResponseCode):
	//   -- GameSpy Devs: Must match server.
	WSCreateUserAccount_Success = 0,
	WSCreateUserAccount_ServerInitFailed,

	WSCreateUserAccount_InvalidPassword,
	WSCreateUserAccount_NicknameInvalid,
	WSCreateUserAccount_NicknameAlreadyExistsForUser,
	WSCreateUserAccount_UniqueNicknameAlreadyInUse,
	WSCreateUserAccount_UserFoundForEmailButNotSamePassword,

	WSCreateUserAccount_DBError,
	WSCreateUserAccount_ServerError,
	WSCreateUserAccount_FailureMax,         // Must be the last failure.

	// Login result (mLoginResult):
	WSCreateUserAccount_HttpError = 100,    // Ghttp reported an error, the response was ignored.
	WSCreateUserAccount_ParseError,         // Couldn't parse http response.
	WSCreateUserAccount_InvalidCertificate, // Login was successful, but the certificate was invalid!
	WSCreateUserAccount_LoginFailed,        // Failed login or other error condition.
	WSCreateUserAccount_OutOfMemory,        // Could not process due to insufficient memory.
	WSCreateUserAccount_InvalidParameters,  // Check the function arguments.
	WSCreateUserAccount_NoAvailabilityCheck,// No Availability Check was performed.
	WSCreateUserAccount_Cancelled,          // Login request was cancelled.
	WSCreateUserAccount_UnknownError,       // An error occured, but detailed information is unavailable.

	// Response codes dealing with errors in response headers:
	WSCreateUserAccount_InvalidGameID = 200, // Make sure the GameID is properly set with wsSetGameCredentials.
	WSCreateUserAccount_InvalidAccessKey,    // Make sure the Access Key is properly set with wsSetGameCredentials.

	// Login results dealing with errors in response headers:
	WSCreateUserAccount_InvalidGameCredentials // Check the parameters passed to wsSetGameCredentials.

} WSCreateUserAccountValue;

typedef enum WSLoginValue
{
	// Login response code (mResponseCode)
	//   -- GameSpy Devs: Must match server
	WSLogin_Success = 0,
	WSLogin_ServerInitFailed,

	WSLogin_UserNotFound,
	WSLogin_InvalidPassword,
	WSLogin_InvalidProfile,
	WSLogin_UniqueNickExpired,

	WSLogin_DBError,
	WSLogin_ServerError,
	WSLogin_FailureMax,         // must be the last failure

	// Login result (mLoginResult)
	WSLogin_HttpError = 100,    // ghttp reported an error, response ignored
	WSLogin_ParseError,         // couldn't parse http response
	WSLogin_InvalidCertificate, // login success but certificate was invalid!
	WSLogin_LoginFailed,        // failed login or other error condition
	WSLogin_OutOfMemory,        // could not process due to insufficient memory
	WSLogin_InvalidParameters,  // check the function arguments
	WSLogin_NoAvailabilityCheck,// No availability check was performed
	WSLogin_Cancelled,          // login request was cancelled
	WSLogin_UnknownError,       // error occured, but detailed information not available

	// response codes dealing with errors in response headers
	WSLogin_InvalidGameID = 200, // make sure GameID is properly set with wsSetGameCredentials
	WSLogin_InvalidAccessKey,       // make sure Access Key is properly set with wsSetGameCredentials

	// login results dealing with errors in response headers
	WSLogin_InvalidGameCredentials // check the parameters passed to wsSetGameCredentials

} WSLoginValue;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define WS_LOGIN_SIGKEY_LEN_BITS      (GS_CRYPT_RSA_BINARY_SIZE)
#define WS_LOGIN_PEERKEY_LEN_BITS     (GS_CRYPT_RSA_BINARY_SIZE)

#define WS_LOGIN_NICK_LEN             (30+1)
#define WS_LOGIN_EMAIL_LEN            (50+1)
#define WS_LOGIN_PASSWORD_LEN         (30+1)
#define WS_LOGIN_UNIQUENICK_LEN       (20+1)
#define WS_LOGIN_CDKEY_LEN            (64+1)
#define WS_LOGIN_TIMESTAMP_LEN        (64+1)
#define WS_LOGIN_PEERKEYMOD_LEN       (WS_LOGIN_PEERKEY_LEN_BITS/8)
#define WS_LOGIN_PEERKEYEXP_LEN       (WS_LOGIN_PEERKEY_LEN_BITS/8)
#define WS_LOGIN_PEERKEYPRV_LEN       (WS_LOGIN_PEERKEY_LEN_BITS/8)
#define WS_LOGIN_KEYHASH_LEN          (33) // 16 byte hash in hexstr +1 for NULL
#define WS_LOGIN_SIGNATURE_LEN        (WS_LOGIN_SIGKEY_LEN_BITS/8)
#define WS_LOGIN_SERVERDATA_LEN       (WS_LOGIN_PEERKEY_LEN_BITS/8)
#define WS_LOGIN_AUTHTOKEN_LEN        (256)
#define WS_LOGIN_PARTNERCHALLENGE_LEN (256)

// A user's login certificate, signed by the GameSpy AuthService
// The certificate is public and may be freely passed around
// Avoid use of pointer members so that structure may be easily copied
typedef struct GSLoginCertificate
{
	gsi_bool mIsValid;
	
	gsi_u32 mLength;
	gsi_u32 mVersion;
	gsi_u32 mPartnerCode; // aka Account space
	gsi_u32 mNamespaceId;
	gsi_u32 mUserId;
	gsi_u32 mProfileId;
	gsi_u32 mExpireTime;
	gsi_char mProfileNick[WS_LOGIN_NICK_LEN];
	gsi_char mUniqueNick[WS_LOGIN_UNIQUENICK_LEN];
	gsi_char mCdKeyHash[WS_LOGIN_KEYHASH_LEN];       // hexstr - bigendian
 	gsCryptRSAKey mPeerPublicKey;
	gsi_u8 mSignature[GS_CRYPT_RSA_BYTE_SIZE];   // binary - bigendian
	gsi_u8 mServerData[WS_LOGIN_SERVERDATA_LEN]; // binary - bigendian
	gsi_char mTimestamp[WS_LOGIN_TIMESTAMP_LEN];
} GSLoginCertificate;

// Private information for the owner of the certificate only
// -- careful! private key information must be kept secret --
typedef struct GSLoginCertificatePrivate
{
	gsCryptRSAKey mPeerPrivateKey;
	char mKeyHash[GS_CRYPT_MD5_HASHSIZE];
} GSLoginPrivateData;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Create User Account callback format:
typedef struct WSCreateUserAccountResponse {
	WSCreateUserAccountValue	mCreateUserAccountResult;	// SDK high-level result (e.g., LoginFailed).
	WSCreateUserAccountValue	mResponseCode;				// Server's result code (e.g., BadPassword).
	GSLoginCertificate			mCertificate;	// Show this to others (proves: "Bill is a valid user").
	GSLoginPrivateData			mPrivateData;	// Keep this secret (proves: "I am Bill")!
	void * mUserData;
} WSCreateUserAccountResponse;

typedef void (*WSCreateUserAccountCallback)(
	GHTTPResult						httpResult, 
	WSCreateUserAccountResponse		* response, 
	void							* userData
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CERTIFICATE login callback format 
typedef struct WSLoginResponse
{
	WSLoginValue mLoginResult;        // SDK high level result, e.g. LoginFailed
	WSLoginValue mResponseCode;       // server's result code,  e.g. BadPassword
	GSLoginCertificate mCertificate;  // Show this to others (proves: "Bill is a valid user")
	GSLoginPrivateData mPrivateData;  // Keep this secret!   (proves: "I am Bill")
	void * mUserData;
} WSLoginResponse;

typedef void (*WSLoginCallback)(GHTTPResult httpResult, WSLoginResponse * response, void * userData);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// PS3 login callback format 
typedef struct WSLoginPs3CertResponse
{
	WSLoginValue mLoginResult;   // SDK high level result, e.g. LoginFailed
	WSLoginValue mResponseCode;  // server's result code,  e.g. BadPassword
	char mRemoteAuthToken[WS_LOGIN_AUTHTOKEN_LEN];         // Show this to others
	char mPartnerChallenge[WS_LOGIN_PARTNERCHALLENGE_LEN]; // keep this secret! (It's a "password" for the token.)
	void * mUserData;
} WSLoginPs3CertResponse;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// PS3 login callback format 
typedef struct WSLoginSonyCertResponse
{
	WSLoginValue mLoginResult;   // SDK high level result, e.g. LoginFailed
	WSLoginValue mResponseCode;  // server's result code,  e.g. BadPassword
	char mRemoteAuthToken[WS_LOGIN_AUTHTOKEN_LEN];         // Show this to others
	char mPartnerChallenge[WS_LOGIN_PARTNERCHALLENGE_LEN]; // keep this secret! (It's a "password" for the token.)
	void * mUserData;
} WSLoginSonyCertResponse;

typedef void (*WSLoginPs3CertCallback)(GHTTPResult httpResult, WSLoginPs3CertResponse * response, void * userData);
typedef void (*WSLoginSonyCertCallback)(GHTTPResult httpResult, WSLoginSonyCertResponse * response, void * userData);

//wsLoginValueString
// Summary
//		Given a WSLoginValue, returns a meaningful string describing the login result.
// Parameters
//		loginValue		: [in] the login value
// Returns
//		A nul-byte terminated character string describing the login result corresponding to the value given.
// Notes
//		The returned string is read-only and must not be modified.
const char* wsLoginValueString(int loginValue);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Services to obtain a certificate

//wsSetGameCredentials
// Summary
//		Set your Access Key and Secret Key to which will be passed with every web service call for authentication and 
//      usage/metric tracking.
// Parameters
//		accessKey			: [in] the Access Key provided via the GameSpy apigee system
//		secretKey   	: [in] the Secret Key provided by GameSpy
// Returns
//
// Notes
//		This must be called prior to calling any wsLogin function
void wsSetGameCredentials(const char * accessKey, const int gameId, const char * secretKey);

//wsCreateUserAccount
// Summary
//		Login using the full GameSpy Presence login information, requiring 
//		email, password, and profile name.
// Parameters
//		partnerCode 	: [in] The partner code.
//		namespaceId		: [in] The namespace ID.
//		email			: [in] The email associated with user's GameSpy account. 
//		profileNick		: [in] The nickname associated with the user's GameSpy 
//								account.
//		uniqueNick		: [in] The uniquenick associated with the user's 
//								GameSpy account.
//		password		: [in] The password for the user's GameSpy account.
//		callback		: [in] Pointer to a function that will be called by 
//								the SDK to report the result of the 
//								authentication request.
//		userData		: [in] A pointer to data that will be supplied to the 
//								callback function.
// Returns
//		WSLoginValue: If successful, the value WSLogin_Success will be returned. 
//		Otherwise, a code specific to the error encountered will be returned.
// Notes
//		The GameSpy SDK Core must be initialized first using gsCoreInitialize 
//		before using this function.
WSCreateUserAccountValue wsCreateUserAccount(
	int								partnerCode, 
	int								namespaceId, 
	const gsi_char					* email, 
	const gsi_char					* profileNick, 
	const gsi_char					* uniqueNick, 
	const gsi_char					* password, 
	WSCreateUserAccountCallback		userCallback, 
	void							* userData
);

//wsLoginProfile
// Summary
//		Login using the full GameSpy Presence login information, requiring email, password, and profile name.
// Parameters
//		gameId			: [in] the game id
//		partnerCode 	: [in] the partner code 
//		namespaceId		: [in] the namespace ID
//		profileNick		: [in] the nickname associated with the user's GameSpy account
//		email			: [in] the email associated with user's GameSpy account 
//		password		: [in] the password for the user's GameSpy account
//		cdkeyhash		: [in] CD key hash
//		callback		: [in] pointer to a function which will be called by the SDK to report the result of the authentication request
//		userData		: [in] a pointer to data that will be supplied to the callback function
// Returns
//		WSLoginValue - 
//			If successful, the value WSLogin_Success will be returned. 
//			Otherwise, a code specific to the error encountered will be returned.
// Notes
//		The GameSpy core must be initialized first using gsCoreInitialize before using this function
WSLoginValue wsLoginProfile(int gameId, int partnerCode, int namespaceId, const gsi_char * profileNick, const gsi_char * email, const gsi_char * password, const gsi_char * cdkeyhash, WSLoginCallback callback, void * userData);

//wsLoginUnique
// Summary
//		Login using a subset of the GameSpy Presence login information, requiring only a unique nick and password.
// Parameters
//		gameId			: [in] the game id
//		partnerCode 	: [in] the partner code 
//		namespaceId		: [in] the namespace ID
//		uniqueNick 		: [in] the unique nickname associated with the user's GameSpy account
//		password		: [in] the password for the user's GameSpy account
//		cdkeyhash		: [in] CD key hash
//		callback		: [in] pointer to a function which will be called by the SDK to report the result of the authentication request
//		userData		: [in] a pointer to data that will be supplied to the callback function
// Returns
//		WSLoginValue - 
//			If successful, the value WSLogin_Success will be returned. 
//			Otherwise, a code specific to the error encountered will be returned.
// Notes
//		The GameSpy core must be initialized first using gsCoreInitialize before using this function
WSLoginValue wsLoginUnique(int gameId, int partnerCode, int namespaceId, const gsi_char * uniqueNick, const gsi_char * password, const gsi_char * cdkeyhash, WSLoginCallback callback, void * userData);

//wsLoginRemoteAuth
// Summary
//		Login by authenticating with a partner system and then use that authentication information with the GameSpy system.
// Parameters
//		gameId				: [in] the game id
//		partnerCode 		: [in] the partner code 
//		namespaceId			: [in] the namespace ID
//		authtoken 			: [in] the authentication token
//		partnerChallenge	: [in] the partner challenge
//		callback			: [in] pointer to a function which will be called by the SDK to report the result of the authentication request
//		userData			: [in] a pointer to data that will be supplied to the callback function
// Returns
//		WSLoginValue - 
//			If successful, the value WSLogin_Success will be returned. 
//			Otherwise, a code specific to the error encountered will be returned.
// Notes
//		The GameSpy core must be initialized first using gsCoreInitialize before using this function
WSLoginValue wsLoginRemoteAuth(int gameId, int partnerCode, int namespaceId, const gsi_char authtoken[WS_LOGIN_AUTHTOKEN_LEN], const gsi_char partnerChallenge[WS_LOGIN_PARTNERCHALLENGE_LEN], WSLoginCallback callback, void * userData);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//DOM-IGNORE-BEGIN 
// Facebook login callback format 
typedef struct WSLoginFacebookResponse
{
    WSLoginValue mLoginResult;      // Result of login attempt
    char * mErrorMsg;               // For failed logins, a clear text debug error message explaining why it failed
    char mRemoteAuthToken[WS_LOGIN_AUTHTOKEN_LEN];         // Show this to others
    char mPartnerChallenge[WS_LOGIN_PARTNERCHALLENGE_LEN]; // keep this secret! (It's a "password" for the token.)
    void * mUserData;
} WSLoginFacebookResponse;

typedef void (*WSLoginFacebookCallback)(GHTTPResult httpResult, WSLoginFacebookResponse * response, void * userData);

#ifdef _IPHONE
//wsLoginFacebook
// Summary
//		Generates authentication credentials to use with GameSpy Remote Authentication based on an active Facebook session.
// Parameters
//		gameId 		    : [in] the game id for your title 
//		uid			    : [in] the uid returned from a successful login via Facebook Connect
//		apiKey 			: [in] the apiKey associated with your Facebook application
//		sessionKey	    : [in] the sessionKey returned from a successful login via Facebook Connect
//		sessionSecret	: [in] the sessionSecret returned from a successful login via Facebook Connect
//		callback		: [in] pointer to a function which will be called by the SDK to report the result of the authentication request
//		userData		: [in] a pointer to data that will be supplied to the callback function
// Returns
//		WSRequest - 
//			If successful, the created GSTask will be returned, which can be used to cancel the request 
//          with gsiCoreCancelTask if necessary. 
//			Otherwise, NULL will be returned.
// Notes
//		The GameSpy core must be initialized first using gsCoreInitialize before using this function
//      This is an iPhone-specific function, and requires usage of the Facebook Connect Libraries for iPhone
// See Also
//		WSLoginFacebookCallback, gsiCoreCancelTask
GSTask *
wsLoginFacebook(
    int gameId, 
    const gsi_u64 uid, 
    const char * apiKey, 
    const char * sessionKey, 
    const char * sessionSecret, 
    WSLoginFacebookCallback callback, 
    void * userData
);
#endif //_IPHONE
//DOM-IGNORE-END 

// Services to obtain a remote auth token
WSLoginValue wsLoginPs3Cert(int gameId, int partnerCode, int namespaceId, const gsi_u8 * ps3cert, int certLen, WSLoginPs3CertCallback callback, void * userData);

//wsLoginSonyCert
// Summary
//		Login for the PS3 or PSP system - authenticates the NP account and creates a corresponding GP 'shadow account'.
// Parameters
//      gameId 		    : [in] the game id for your title 
//		partnerCode 	: [in] the partner code 
//		namespaceId		: [in] the namespace ID
//		ps3Cert 		: [in] the npTicket obtained from the Sony SDK
//		certLen			: [in] the length of the npTicket
//		callback		: [in] pointer to a function which will be called by the SDK to report the result of the authentication request
//		userData		: [in] a pointer to data that will be supplied to the callback function
// Returns
//		WSLoginValue - 
//			If successful, the value WSLogin_Success will be returned. 
//			Otherwise, a code specific to the error encountered will be returned.
// Notes
//		The GameSpy core must be initialized first using gsCoreInitialize before using this function
WSLoginValue wsLoginSonyCert(int gameId, int partnerCode, int namespaceId, const gsi_u8 * ps3cert, int certLen, WSLoginSonyCertCallback callback, void * userData);

// Certificate Utilities, for use after obtaining a certificate
gsi_bool wsLoginCertIsValid    (const GSLoginCertificate * cert);
gsi_bool wsLoginCertWriteXML   (const GSLoginCertificate * cert, const char * anamespace, GSXmlStreamWriter writer);
gsi_bool wsLoginCertWriteBinary(const GSLoginCertificate * cert, char * bufout, unsigned int maxlen, unsigned int * lenout);
gsi_bool wsLoginCertReadBinary (GSLoginCertificate * certOut, char * bufin, unsigned int maxlen);
gsi_bool wsLoginCertReadXML    (GSLoginCertificate * cert, GSXmlStreamReader reader);

char* wsGetServiceUrl();
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// BuddyService Functions
typedef struct Buddy
{
    int mProfileid;              // profileid associated with buddy
    char * mName;                // buddy name (uniquenick) for display
} Buddy;

// Get Buddy List callback format 
typedef struct WSGetBuddyListResponse
{
    WSLoginValue mResult;           // Result of buddy list request
    char * mErrorMsg;               // For failed requests, a clear text debug error message explaining why it failed
    Buddy * mBuddyList;             // Pointer to list of buddies, null if no buddies found
    int mNumBuddies;                // Count of total number of buddy profiles returned
    void * mUserData;
} WSGetBuddyListResponse;

typedef void (*WSGetBuddyListCallback)(GHTTPResult httpResult, WSGetBuddyListResponse * response, void * userData);

//wsGetBuddyList
// Summary
//		Grabs the names and profileids for a user's buddy list using an authenticated login certificate
// Parameters
//		gameId 		    : [in] the game id for your title 
//		certificate	    : [in] the user's authenticated login certificate
//		privData 		: [in] the private data returned with authentication
//		callback		: [in] pointer to a function which will be called by the SDK to report the result of the request
//		userData		: [in] a pointer to data that will be supplied to the callback function
// Returns
//		WSRequest - 
//			If successful, the created GSTask will be returned, which can be used to cancel the request 
//          with gsiCoreCancelTask if necessary. 
//			Otherwise, NULL will be returned.
// Notes
//		The GameSpy core must be initialized first using gsCoreInitialize before using this function
// See Also
//		WSGetBuddyListCallback, gsiCoreCancelTask
GSTask *
wsGetBuddyList(
    int gameId, 
    const GSLoginCertificate * certificate,
    const GSLoginPrivateData * privData, 
    WSGetBuddyListCallback callback, 
    void * userData
);

///////////////////////////////////////////////////////////////////////////////
//DOM-IGNORE-BEGIN
typedef struct FacebookBuddy
{
    int mProfileid;              // profileid associated with buddy
    char * mName;                // buddy name for display
    gsi_u64 mUid;                // Facebook UID associated with each buddy
} FacebookBuddy;

// Get Buddy List callback format 
typedef struct WSSyncFacebookResponse
{
    WSLoginValue mResult;           // Result of facebook sync request
    char * mErrorMsg;               // For failed requests, a clear text debug error message explaining why it failed
    FacebookBuddy * mBuddyList;     // Pointer to list of buddies, null if no buddies found
    int mNumBuddies;                // Count of total number of buddy profiles returned
    void * mUserData;
} WSSyncFacebookResponse;

typedef void (*WSSyncFacebookCallback)(GHTTPResult httpResult, WSSyncFacebookResponse * response, void * userData);

#ifdef _IPHONE
//wsSyncFacebook
// Summary
//		Grabs the names, profileids, and Facebook UIDs for a Facebook user's buddy list using an authenticated login certificate, 
//      after first syncing up the Facebook friends list w/ any registered Facebook accounts in the GP system.
// Parameters
//		gameId 		    : [in] the game id for your title 
//		certificate	    : [in] the user's authenticated login certificate
//		privData 		: [in] the private data returned with authentication
//		apiKey 			: [in] the apiKey associated with your Facebook application
//		sessionKey	    : [in] the sessionKey returned from a successful login via Facebook Connect
//		sessionSecret	: [in] the sessionSecret returned from a successful login via Facebook Connect
//		callback		: [in] pointer to a function which will be called by the SDK to report the result of the request
//		userData		: [in] a pointer to data that will be supplied to the callback function
// Returns
//		WSRequest - 
//			If successful, the created GSTask will be returned, which can be used to cancel the request 
//          with gsiCoreCancelTask if necessary. 
//			Otherwise, NULL will be returned.
// Notes
//		The GameSpy core must be initialized first using gsCoreInitialize before using this function
//      This is an iPhone-specific function, and requires usage of the Facebook Connect Libraries for iPhone
// See Also
//		WSSyncFacebookCallback, gsiCoreCancelTask
GSTask *
wsSyncFacebook(
    int gameId, 
    const GSLoginCertificate * certificate,
    const GSLoginPrivateData * privData, 
    const char * apiKey, 
    const char * sessionKey, 
    const char * sessionSecret, 
    WSSyncFacebookCallback callback, 
    void * userData
);
#endif //_IPHONE
//DOM-IGNORE-END 

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
} // extern "C"
#endif

#endif //__AUTHSERVICE_H__
