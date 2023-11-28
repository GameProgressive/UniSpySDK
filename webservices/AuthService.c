///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "AuthService.h"
#include "../common/gsXML.h"
#include "../common/gsAvailable.h"
#include "../common/md5.h"
///////////////////////////////////////////////////////////////////////////////
// File:	AuthService.c
// SDK:		GameSpy Authentication Service SDK
//
// Copyright Notice: This file is part of the GameSpy SDK designed and 
// developed by GameSpy Industries. Copyright (c) 2008-2009 GameSpy Industries, Inc.

#include "../common/gsSHA1.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////// Old login system (no game id)
#define WS_AUTHSERVICE_LOGINPROFILE_OLD		 "LoginProfile"
#define WS_AUTHSERVICE_LOGINUNIQUE_OLD		 "LoginUniqueNick"
#define WS_AUTHSERVICE_LOGINREMOTEAUTH_OLD   "LoginRemoteAuth"
#define WS_AUTHSERVICE_LOGINPS3CERT_OLD		 "LoginPs3Cert"
#define WS_AUTHSERVICE_LOGINPROFILE_SOAP_OLD "SOAPAction: \"http://gamespy.net/AuthService/LoginProfile\""
#define WS_AUTHSERVICE_LOGINUNIQUE_SOAP_OLD  "SOAPAction: \"http://gamespy.net/AuthService/LoginUniqueNick\""
#define WS_AUTHSERVICE_LOGINREMOTEAUTH_SOAP_OLD "SOAPAction: \"http://gamespy.net/AuthService/LoginRemoteAuth\""
#define WS_AUTHSERVICE_LOGINPS3CERT_SOAP_OLD "SOAPAction: \"http://gamespy.net/AuthService/LoginPs3Cert\""

#define WS_AUTHSERVICE_LOGINPROFILE		 "LoginProfileWithGameId"
#define WS_AUTHSERVICE_LOGINUNIQUE		 "LoginUniqueNickWithGameId"
#define WS_AUTHSERVICE_LOGINREMOTEAUTH   "LoginRemoteAuthWithGameId"
#define WS_AUTHSERVICE_LOGINPS3CERT      "LoginPs3CertWithGameId"
#define WS_AUTHSERVICE_CREATEUSER        "CreateUserAccount"
#define WS_AUTHSERVICE_LOGINFACEBOOK	 "LoginFacebook"

#define WS_BUDDYSERVICE_GETLIST          "GetBuddyListWithNames"
#define WS_BUDDYSERVICE_SYNCFACEBOOK	 "SyncFacebook"

#define WS_AUTHSERVICE_LOGINPROFILE_SOAP    "SOAPAction: \"http://gamespy.net/AuthService/LoginProfileWithGameId\""
#define WS_AUTHSERVICE_LOGINUNIQUE_SOAP     "SOAPAction: \"http://gamespy.net/AuthService/LoginUniqueNick\""
#define WS_AUTHSERVICE_LOGINREMOTEAUTH_SOAP "SOAPAction: \"http://gamespy.net/AuthService/LoginRemoteAuthWithGameId\""
#define WS_AUTHSERVICE_LOGINPS3CERT_SOAP    "SOAPAction: \"http://gamespy.net/AuthService/LoginPs3CertWithGameId\""
#define WS_AUTHSERVICE_CREATEUSER_SOAP      "SOAPAction: \"http://gamespy.net/AuthService/CreateUserAccount\""
#define WS_AUTHSERVICE_LOGINFACEBOOK_SOAP	"SOAPAction: \"http://gamespy.net/AuthService/LoginFacebook\""

#define WS_BUDDYSERVICE_GETLIST_SOAP        "SOAPAction: \"http://gamespy.net/BuddyService/GetBuddyListWithName\""
#define WS_BUDDYSERVICE_SYNCFACEBOOK_SOAP   "SOAPAction: \"http://gamespy.net/BuddyService/SyncFacebook\""

#define WS_AUTHSERVICE_PROTOVERSION      1

#define WS_AUTHSERVICE_NAMESPACE         "ns1"

#define WS_AUTHSERVICE_NAME				"AuthService.asmx"
#define WS_BUDDYSERVICE_NAME			"BuddyService.asmx"

#define WS_AUTHSERVICE_NAMESPACE_COUNT  1
const char * WS_AUTHSERVICE_NAMESPACES[WS_AUTHSERVICE_NAMESPACE_COUNT] =
{
	WS_AUTHSERVICE_NAMESPACE "=\"http://gamespy.net/AuthService/\""
};

#define WS_BUDDYSERVICE_NAMESPACE_COUNT 1
const char * WS_BUDDYSERVICE_NAMESPACES[WS_BUDDYSERVICE_NAMESPACE_COUNT] =
{
	WS_AUTHSERVICE_NAMESPACE "=\"http://gamespy.net/AuthService/\""
};

const char WS_AUTHSERVICE_SIGNATURE_KEY[] = 
	"BF05D63E93751AD4A59A4A7389CF0BE8"
	"A22CCDEEA1E7F12C062D6E194472EFDA"
	"5184CCECEB4FBADF5EB1D7ABFE911814"
	"53972AA971F624AF9BA8F0F82E2869FB"
	"7D44BDE8D56EE50977898F3FEE758696"
	"22C4981F07506248BD3D092E8EA05C12"
	"B2FA37881176084C8F8B8756C4722CDC"
	"57D2AD28ACD3AD85934FB48D6B2D2027";

/*	2007 RSA key (left for legacy)
 	"D589F4433FAB2855F85A4EB40E6311F0"
	"284C7882A72380938FD0C55CC1D65F7C"
	"6EE79EDEF06C1AE5EDE139BDBBAB4219"
	"B7D2A3F0AC3D3B23F59F580E0E89B9EC"
	"787F2DD5A49788C633D5D3CE79438934"
	"861EA68AE545D5BBCAAAD917CE9F5C7C"
	"7D1452D9214F989861A7511097C35E60"
	"A7273DECEA71CB5F8251653B26ACE781";
*/


#ifdef UNISPY_NO_RSA_CERT
const char WS_AUTHSERVICE_SIGNATURE_EXP[] = "000001";
#else
const char WS_AUTHSERVICE_SIGNATURE_EXP[] = "010001";
#endif

// This is declared as an extern so it can be overriden when testing

// Old API
//#define WS_LOGIN_SERVICE_URL_FORMAT   GSI_HTTP_PROTOCOL_URL "%s.auth.pubsvs." GSI_DOMAIN_NAME "/AuthService/WS_AUTHSERVICE_NAME"

#ifdef UNISPY_FORCE_IP
#define WS_LOGIN_SERVICE_URL_FORMAT GSI_HTTP_PROTOCOL_URL "%s/AuthService/%s"
#else
#define WS_LOGIN_SERVICE_URL_FORMAT GSI_HTTP_PROTOCOL_URL GSI_OPEN_DOMAIN_NAME "/AuthService/%s"
#endif

char wsAuthServiceURL[WS_LOGIN_MAX_URL_LEN] = "";
char authCreds[92];

gsi_bool __isAuthenticated = gsi_false; // new 2012!

typedef struct WSIRequestData
{
	union 
	{
		WSSyncFacebookCallback mSyncFacebookCallback;
		WSLoginFacebookCallback mLoginFacebookCallback;
		WSGetBuddyListCallback mBuddyListCallback;
		WSCreateUserAccountCallback mCreateUserAccountCallback;
		WSLoginCallback mLoginCallback;
		WSLoginSonyCertCallback mLoginSonyCertCallback;
	} mUserCallback;
	void * mUserData;
	GSSoapTask* mTask;
} WSIRequestData;

//MACROS for debug printing
#define GSAUTH_DP(t,l,m, ...)              gsDebugFormat(GSIDebugCat_AuthService, t, l, m, __VA_ARGS__) 
#define GSAUTH_DP_FILE_FUNCTION_LINE(t, l) GSAUTH_DP(t, l, "%s(%d): In %s:\n", __FILE__, __LINE__, __FUNCTION__)
// m is concatenated to format string
#define GSAUTH_DEBUG_LOG(t, l, m, ... )    GSAUTH_DP(t, l, "%s(%d): In %s: "m"\n" , __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsiLoginEncryptPassword(const gsi_char * password, gsi_u8 ciphertext[GS_CRYPT_RSA_BYTE_SIZE]);
static gsi_bool wsiServiceAvailable(const char* serviceName);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Checks to make sure the availability check has been performed prior
// to using any AuthService Login, and sets the service URL if it has
static gsi_bool wsiServiceAvailable(const char* serviceName)
{
	if (__GSIACResult == GSIACAvailable)
	{
		if (wsAuthServiceURL[0] == '\0')
#ifndef UNISPY_FORCE_IP
			snprintf(wsAuthServiceURL, WS_LOGIN_MAX_URL_LEN, WS_LOGIN_SERVICE_URL_FORMAT, __GSIACGamename, serviceName);
#else
			snprintf(wsAuthServiceURL, WS_LOGIN_MAX_URL_LEN, WS_LOGIN_SERVICE_URL_FORMAT, UNISPY_FORCE_IP, serviceName);
#endif

		return gsi_true;
	}
	else
		return gsi_false;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Converts an authentication error to webservice error
static void wsiAuthErrorToLoginResponse(GSAuthErrorCode authError, WSLoginResponse* response)
{
	if (authError == GSAuthErrorCode_InvalidGameID)
	{
		response->mLoginResult = WSLogin_InvalidGameCredentials;
		response->mResponseCode = WSLogin_InvalidGameID;
	}
	else if (authError == GSAuthErrorCode_InvalidAccessKey)
	{
		response->mLoginResult = WSLogin_InvalidGameCredentials;
		response->mResponseCode = WSLogin_InvalidAccessKey;
	}
	else
	{
		response->mLoginResult = WSLogin_InvalidGameCredentials;
		response->mResponseCode = WSLogin_UnknownError;
	}

	gsiCoreSetAuthError("");
	gsiCoreSetAuthErrorCode(GSAuthErrorCode_None);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Converts an authentication error to webservice error (for Sony)
static void wsiAuthErrorToLoginSonyCertResponse(GSAuthErrorCode authError, WSLoginSonyCertResponse* theResponse)
{
	if (authError == GSAuthErrorCode_InvalidGameID)
	{
		theResponse->mLoginResult = WSLogin_InvalidGameCredentials;
		theResponse->mResponseCode = WSLogin_InvalidGameID;
	}
	else if (authError == GSAuthErrorCode_InvalidAccessKey)
	{
		theResponse->mLoginResult = WSLogin_InvalidGameCredentials;
		theResponse->mResponseCode = WSLogin_InvalidAccessKey;
	}
	else
	{
		theResponse->mLoginResult = WSLogin_InvalidGameCredentials;
		theResponse->mResponseCode = WSLogin_UnknownError;
	}

	gsiCoreSetAuthError("");
	gsiCoreSetAuthErrorCode(GSAuthErrorCode_None);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsiLoginProfileCallback(GHTTPResult theResult, 
								   GSXmlStreamWriter theRequestXml,
								   GSXmlStreamReader theResponseXml,
								   void * theRequestData)
{
	GHTTPResult translatedResult = GHTTPSuccess;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;

	WSLoginResponse response;
	GSLoginCertificate * cert = &response.mCertificate; // for convenience

	// initialize local variables
	cert->mIsValid = gsi_false;
	memset(&response, 0, sizeof(response));
	
	if (theResult == GHTTPRequestCancelled)
	{
		response.mLoginResult = WSLogin_Cancelled;
	}
	else if (theResult != GHTTPSuccess)
	{
		response.mLoginResult = WSLogin_HttpError;
	}
	else if (gsiCoreGetAuthErrorCode())
	{
		GSAuthErrorCode err = gsiCoreGetAuthErrorCode();
		wsiAuthErrorToLoginResponse(err, &response);
	}
	if (theResult == GHTTPSuccess)
	{
		// try to parse the soap
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "LoginProfileWithGameIdResult")))
		{
			response.mLoginResult = WSLogin_ParseError;
		}
		else
		{
			// prepare response structure
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode)))
			{
				// could not parse login response code
				response.mLoginResult = WSLogin_ParseError;
			}
			else if (response.mResponseCode != WSLogin_Success)
			{
				// server reported an error into reponseCode
				response.mLoginResult = WSLogin_ServerError;
			}
			else if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "certificate")) ||
				gsi_is_false(wsLoginCertReadXML(cert, theResponseXml)) ||
				gsi_is_false(gsXmlMoveToParent(theResponseXml)) ||
				gsi_is_false(gsXmlReadChildAsLargeInt(theResponseXml, "peerkeyprivate", 
				                 &response.mPrivateData.mPeerPrivateKey.exponent))
				)
			{
				response.mLoginResult = WSLogin_ParseError;
			}
			else
			{
				GSMD5_CTX md5;
				char buffer[20];

				// peer privatekey modulus is same as peer public key modulus
				memcpy(&response.mPrivateData.mPeerPrivateKey, &cert->mPeerPublicKey, sizeof(cert->mPeerPublicKey));

				// hash the private key
				GSMD5Init(&md5);
				//gsLargeIntAddToMD5(&response.mPrivateData.mPeerPrivateKey.modulus, &md5);
				gsLargeIntAddToMD5(&response.mPrivateData.mPeerPrivateKey.exponent, &md5);
				GSMD5Final((unsigned char*)response.mPrivateData.mKeyHash, &md5);

				// verify certificate
				cert->mIsValid = wsLoginCertIsValid(cert);
				if (gsi_is_false(cert->mIsValid))
				{
					response.mLoginResult = WSLogin_InvalidCertificate;
				}
				else
				{
					__isAuthenticated = gsi_true;
				}

				sprintf(buffer, "%u", cert->mProfileId);
				gsiCoreSetProfileId(buffer);
			}
		}
	}

	// trigger the user callback
	if (requestData->mUserCallback.mLoginCallback != NULL)
	{
		WSLoginCallback userCallback = (WSLoginCallback)(requestData->mUserCallback.mLoginCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WSLoginValue wsLoginProfile(int gameId,
					   int partnerCode,
					   int namespaceId,
					   const gsi_char * profileNick, 
					   const gsi_char * email, 
					   const gsi_char * password, 
					   const gsi_char * cdkey, 
					   WSLoginCallback callback, 
					   void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;
	gsi_u8 encryptedPassword[GS_CRYPT_RSA_BYTE_SIZE];

	if (!wsiServiceAvailable(WS_AUTHSERVICE_NAME))
		return WSLogin_NoAvailabilityCheck;

	GS_ASSERT(gameId >= 0);
	GS_ASSERT(partnerCode >= 0);
	GS_ASSERT(profileNick != NULL);
	GS_ASSERT(email       != NULL);
	GS_ASSERT(password    != NULL);

	if (_tcslen(profileNick) >= WS_LOGIN_NICK_LEN)
		return WSLogin_InvalidParameters;
	if (_tcslen(email) >= WS_LOGIN_EMAIL_LEN)
		return WSLogin_InvalidParameters;
	if (_tcslen(password) >= WS_LOGIN_PASSWORD_LEN)
		return WSLogin_InvalidParameters;
	if (cdkey != NULL && (_tcslen(cdkey) > WS_LOGIN_CDKEY_LEN))
		return WSLogin_InvalidParameters;

	// make a copy of the request callback and user param
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSLogin_OutOfMemory;
	requestData->mUserCallback.mLoginCallback = callback;
	requestData->mUserData     = userData;
	requestData->mTask = NULL;

	// encrypt the password (includes safety padding and hash)
	wsiLoginEncryptPassword(password, encryptedPassword);

	writer = gsXmlCreateStreamWriter(WS_AUTHSERVICE_NAMESPACES, WS_AUTHSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		char buffer[1024];

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINPROFILE)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "gameid", gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "partnercode", (gsi_u32)partnerCode)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "namespaceid", (gsi_u32)namespaceId)) ||
			gsi_is_false(gsXmlWriteAsciiStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "email", email)) ||
			gsi_is_false(gsXmlWriteAsciiStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "profilenick", profileNick)) ||
			gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, "password")) ||
			gsi_is_false(gsXmlWriteHexBinaryElement(writer, WS_AUTHSERVICE_NAMESPACE, "Value", encryptedPassword, GS_CRYPT_RSA_BYTE_SIZE)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, "password")) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINPROFILE)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSLogin_OutOfMemory;
		}
		
		strcpy(buffer, WS_AUTHSERVICE_LOGINPROFILE_SOAP);
		strcat(buffer, "\r\nAccessKey: ");
		strcat(buffer, authCreds);

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, buffer,
			        writer, wsiLoginProfileCallback, (void*)requestData);
		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSLogin_OutOfMemory;
		}
	}
	else
	{
		gsifree(requestData);
		return WSLogin_OutOfMemory;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsLoginUniqueCallback(GHTTPResult theResult, 
								  GSXmlStreamWriter theRequestXml,
								  GSXmlStreamReader theResponseXml,
								  void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;
	
	WSLoginResponse response;
	GSLoginCertificate * cert = &response.mCertificate; // for convenience
	cert->mIsValid = gsi_false;

	memset(&response, 0, sizeof(response));

	if (theResult == GHTTPRequestCancelled)
	{
	response.mLoginResult = WSLogin_Cancelled;
	}
	else if (theResult != GHTTPSuccess)
	{
	response.mLoginResult = WSLogin_HttpError;
	}
	else if (gsiCoreGetAuthErrorCode())
	{
		GSAuthErrorCode err = gsiCoreGetAuthErrorCode();
		wsiAuthErrorToLoginResponse(err, &response);
	}
	else
	{
		// try to parse the soap
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "LoginUniqueNickWithGameIdResult")))
		{
			response.mLoginResult = WSLogin_ParseError;
		}
		else
		{
			// prepare response structure
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode)))
			{
				// could not parse login response code
				response.mLoginResult = WSLogin_ParseError;
			}
			else if (response.mResponseCode != WSLogin_Success)
			{
				// server reported an error into reponseCode
				response.mLoginResult = WSLogin_ServerError;
			}
			else if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "certificate")) ||
				gsi_is_false(wsLoginCertReadXML(cert, theResponseXml)) ||
				gsi_is_false(gsXmlMoveToParent(theResponseXml)) ||
				gsi_is_false(gsXmlReadChildAsLargeInt(theResponseXml, "peerkeyprivate", 
				                 &response.mPrivateData.mPeerPrivateKey.exponent))
				)
			{
				response.mLoginResult = WSLogin_ParseError;
			}
			else
			{
				GSMD5_CTX md5;
				char buffer[20];

				// peer privatekey modulus is same as peer public key modulus
				memcpy(&response.mPrivateData.mPeerPrivateKey.modulus, &cert->mPeerPublicKey.modulus, sizeof(cert->mPeerPublicKey.modulus));

				// hash the private key
				//   -- we use the has like a password for simple authentication
				GSMD5Init(&md5);
				//gsLargeIntAddToMD5(&response.mPrivateData.mPeerPrivateKey.modulus, &md5);
				gsLargeIntAddToMD5(&response.mPrivateData.mPeerPrivateKey.exponent, &md5);
				GSMD5Final((unsigned char*)response.mPrivateData.mKeyHash, &md5);

				// verify certificate
				cert->mIsValid = wsLoginCertIsValid(cert);
				if (gsi_is_false(cert->mIsValid))
				{
					response.mLoginResult = WSLogin_InvalidCertificate;
				}
				else
				{
					__isAuthenticated = gsi_true;
				}

				sprintf(buffer, "%u", cert->mProfileId);
				gsiCoreSetProfileId(buffer);
			}
		}
	}

	// trigger the user callback
	if (requestData->mUserCallback.mLoginCallback != NULL)
	{
		WSLoginCallback userCallback = (WSLoginCallback)(requestData->mUserCallback.mLoginCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//gsi_u32 wsLoginUnique(WSLoginUniqueRequest * request, WSLoginCallback callback)
WSLoginValue wsLoginUnique(int gameId,
					  int partnerCode,
					  int namespaceId,
					  const gsi_char * uniqueNick, 
					  const gsi_char * password, 
					  const gsi_char * cdkey, 
					  WSLoginCallback userCallback, 
					  void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;
	gsi_u8 encryptedPassword[GS_CRYPT_RSA_BYTE_SIZE];

	if (!wsiServiceAvailable(WS_AUTHSERVICE_NAME))
		return WSLogin_NoAvailabilityCheck;

	GS_ASSERT(partnerCode >= 0);
	GS_ASSERT(uniqueNick != NULL);
	GS_ASSERT(password != NULL);
	GS_ASSERT(gameId >= 0);

	if (_tcslen(uniqueNick) >= WS_LOGIN_UNIQUENICK_LEN)
		return WSLogin_InvalidParameters;
	if (_tcslen(password) >= WS_LOGIN_PASSWORD_LEN)
		return WSLogin_InvalidParameters;
	if (cdkey != NULL && (_tcslen(cdkey) >= WS_LOGIN_CDKEY_LEN))
		return WSLogin_InvalidParameters;

	// allocate the request values
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSLogin_OutOfMemory;
	requestData->mUserCallback.mLoginCallback = userCallback;
	requestData->mUserData     = userData;
	requestData->mTask = NULL;

	// encrypt the password (includes safety padding and hash)
	wsiLoginEncryptPassword(password, encryptedPassword);

	// create the xml request
	writer = gsXmlCreateStreamWriter(WS_AUTHSERVICE_NAMESPACES, WS_AUTHSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		char buffer[1024];

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINUNIQUE)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "gameid", gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "partnercode", (gsi_u32)partnerCode)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "namespaceid", (gsi_u32)namespaceId)) ||
			gsi_is_false(gsXmlWriteAsciiStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "uniquenick", uniqueNick)) ||
			gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, "password")) ||
			gsi_is_false(gsXmlWriteHexBinaryElement(writer, WS_AUTHSERVICE_NAMESPACE, "Value", encryptedPassword, GS_CRYPT_RSA_BYTE_SIZE)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, "password")) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINUNIQUE)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSLogin_OutOfMemory;
		}
		
		strcpy(buffer, WS_AUTHSERVICE_LOGINUNIQUE_SOAP);
		strcat(buffer, "\r\nAccessKey: ");
		strcat(buffer, authCreds);

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, buffer,
			        writer, wsLoginUniqueCallback, (void*)requestData);
		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSLogin_OutOfMemory;
		}
	}
	else
	{
		gsifree(requestData);
		return WSLogin_OutOfMemory;
	}

	return 0;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsLoginRemoteAuthCallback(GHTTPResult theResult, 
									  GSXmlStreamWriter theRequestXml,
									  GSXmlStreamReader theResponseXml,
									  void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;
	
	WSLoginResponse response;
	GSLoginCertificate * cert = &response.mCertificate; // for convenience
	cert->mIsValid = gsi_false;

	memset(&response, 0, sizeof(response));

	if (theResult == GHTTPRequestCancelled)
	{
		response.mLoginResult = WSLogin_Cancelled;
	}
	else if (theResult != GHTTPSuccess)
	{
		response.mLoginResult = WSLogin_HttpError;
	}
	else if (gsiCoreGetAuthErrorCode())
	{
		GSAuthErrorCode err = gsiCoreGetAuthErrorCode();
		wsiAuthErrorToLoginResponse(err, &response);
	}
	else
	{
		// try to parse the soap
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "LoginRemoteAuthWithGameIdResult")))
		{
			response.mLoginResult = WSLogin_ParseError;
		}
		else
		{
			// prepare response structure
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode)))
			{
				// could not parse login response code
				response.mLoginResult = WSLogin_ParseError;
			}
			else if (response.mResponseCode != WSLogin_Success)
			{
				// server reported an error into reponseCode
				response.mLoginResult = WSLogin_ServerError;
			}
			else if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "certificate")) ||
				gsi_is_false(wsLoginCertReadXML(cert, theResponseXml)) ||
				gsi_is_false(gsXmlMoveToParent(theResponseXml)) ||
				gsi_is_false(gsXmlReadChildAsLargeInt(theResponseXml, "peerkeyprivate", 
				                 &response.mPrivateData.mPeerPrivateKey.exponent))
				)
			{
				response.mLoginResult = WSLogin_ParseError;
			}
			else
			{
				GSMD5_CTX md5;
				char buffer[20];

				// peer privatekey modulus is same as peer public key modulus
				memcpy(&response.mPrivateData.mPeerPrivateKey.modulus, &cert->mPeerPublicKey.modulus, sizeof(cert->mPeerPublicKey.modulus));

				// hash the private key
				//   -- we use the has like a password for simple authentication
				GSMD5Init(&md5);
				//gsLargeIntAddToMD5(&response.mPrivateData.mPeerPrivateKey.modulus, &md5);
				gsLargeIntAddToMD5(&response.mPrivateData.mPeerPrivateKey.exponent, &md5);
				GSMD5Final((unsigned char*)response.mPrivateData.mKeyHash, &md5);

				// verify certificate
				cert->mIsValid = wsLoginCertIsValid(cert);
				if (gsi_is_false(cert->mIsValid))
				{
					response.mLoginResult = WSLogin_InvalidCertificate;
				}
				else
				{
					__isAuthenticated = gsi_true;
				}

				sprintf(buffer, "%u", cert->mProfileId);
			}
		}
	}

	// trigger the user callback
	if (requestData->mUserCallback.mLoginCallback != NULL)
	{
		WSLoginCallback userCallback = (WSLoginCallback)(requestData->mUserCallback.mLoginCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WSLoginValue wsLoginRemoteAuth(int gameId,
						  int partnerCode, 
						  int namespaceId, 
						  const gsi_char authtoken[WS_LOGIN_AUTHTOKEN_LEN], 
						  const gsi_char partnerChallenge[WS_LOGIN_PARTNERCHALLENGE_LEN], 
						  WSLoginCallback userCallback, 
						  void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;
	//gsi_u8 encryptedChallenge[GS_CRYPT_RSA_BYTE_SIZE];

	if (!wsiServiceAvailable(WS_AUTHSERVICE_NAME))
		return WSLogin_NoAvailabilityCheck;

	GS_ASSERT(gameId >= 0);
	GS_ASSERT(partnerCode >= 0);

	// allocate the request values
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSLogin_OutOfMemory;

	requestData->mUserCallback.mLoginCallback = userCallback;
	requestData->mUserData     = userData;
	requestData->mTask = NULL;

	// encrypt the password (includes safety padding and hash)
	//wsiLoginEncryptPassword(partnerChallenge, encryptedChallenge);

	// create the xml request
	writer = gsXmlCreateStreamWriter(WS_AUTHSERVICE_NAMESPACES, WS_AUTHSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		char buffer[1024];

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINREMOTEAUTH)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "gameid", gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "partnercode", (gsi_u32)partnerCode)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "namespaceid", (gsi_u32)namespaceId)) ||
			gsi_is_false(gsXmlWriteTStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "authtoken", authtoken)) ||
			gsi_is_false(gsXmlWriteTStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "challenge", partnerChallenge)) ||
			//gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, "challenge")) ||
			//gsi_is_false(gsXmlWriteHexBinaryElement(writer, WS_AUTHSERVICE_NAMESPACE, "Value", encryptedChallenge, GS_CRYPT_RSA_BYTE_SIZE)) ||
			//gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, "challenge")) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINREMOTEAUTH)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSLogin_OutOfMemory;
		}
		
		strcpy(buffer, WS_AUTHSERVICE_LOGINREMOTEAUTH_SOAP);
		strcat(buffer, "\r\nAccessKey: ");
		strcat(buffer, authCreds);

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, buffer,
			        writer, wsLoginRemoteAuthCallback, (void*)requestData);
		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSLogin_OutOfMemory;
		}
	}
	else
	{
		gsifree(requestData);
		return WSLogin_OutOfMemory;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsLoginSonyCertCallback(GHTTPResult theResult, 
								  GSXmlStreamWriter theRequestXml,
								  GSXmlStreamReader theResponseXml,
								  void * theRequestData)
{

	GHTTPResult translatedResult = theResult;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;
	
	WSLoginSonyCertResponse response;
	memset(&response, 0, sizeof(response));

	if (theResult == GHTTPRequestCancelled)
	{
	response.mLoginResult = WSLogin_Cancelled;
	}
	else if (theResult != GHTTPSuccess)
	{
	response.mLoginResult = WSLogin_HttpError;
	}
	else if (gsiCoreGetAuthErrorCode())
	{
		GSAuthErrorCode err = gsiCoreGetAuthErrorCode();
		wsiAuthErrorToLoginSonyCertResponse(err, &response);
	}
	else
	{
		// try to parse the soap
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "LoginPs3CertWithGameIdResult")))
		{
			response.mLoginResult = WSLogin_ParseError;
		}
		else
		{
			// prepare response structure
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode)))
			{
				// could not parse login response code
				response.mLoginResult = WSLogin_ParseError;
			}
			else if (response.mResponseCode != WSLogin_Success)
			{
				// server reported an error into reponseCode
				response.mLoginResult = WSLogin_ServerError;
			}
			else 
			{
				const char * tokenStr = NULL;
				const char * challengeStr = NULL;
				int tokenLen = 0;
				int challengeLen = 0;

				// Check length of token+challenge, then read into memory
				if ( //gsi_is_false(gsXmlReadChildAsBase64Binary(theResponseXml, "authToken", NULL, &tokenLen)) ||
				     //gsi_is_false(gsXmlReadChildAsBase64Binary(theResponseXml, "partnerChallenge", NULL, &challengeLen)) ||
					 //(tokenLen > WS_LOGIN_AUTHTOKEN_LEN || challengeLen > WS_LOGIN_PARTNERCHALLENGE_LEN) ||
					gsi_is_false(gsXmlReadChildAsString(theResponseXml, "authToken", &tokenStr, &tokenLen)) ||
					gsi_is_false(gsXmlReadChildAsString(theResponseXml, "partnerChallenge", &challengeStr, &challengeLen)) ||
					(tokenLen >= WS_LOGIN_AUTHTOKEN_LEN || challengeLen >= WS_LOGIN_PARTNERCHALLENGE_LEN) )
				{
					response.mLoginResult = WSLogin_ParseError;
				}
				else
				{
					memcpy(response.mRemoteAuthToken, tokenStr, (gsi_u32)tokenLen);
					memcpy(response.mPartnerChallenge, challengeStr, (gsi_u32)challengeLen);
					response.mRemoteAuthToken[tokenLen] = '\0';
					response.mPartnerChallenge[challengeLen] = '\0';
					__isAuthenticated = gsi_true;
				}
			}
		}
	}

	// trigger the user callback
	if (requestData->mUserCallback.mLoginSonyCertCallback != NULL)
	{
		WSLoginSonyCertCallback userCallback = (WSLoginSonyCertCallback)(requestData->mUserCallback.mLoginSonyCertCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}
	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Compatibility with old AuthService SDK
WSLoginValue wsLoginPs3Cert(int gameId, int partnerCode, int namespaceId, const gsi_u8* ps3cert, int certLen, WSLoginPs3CertCallback callback, void* userData)
{
	return wsLoginSonyCert(gameId, partnerCode, namespaceId, ps3cert, certLen, (WSLoginSonyCertCallback)callback, userData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//gsi_u32 wsLoginUnique(WSLoginUniqueRequest * request, WSLoginCallback callback)
WSLoginValue wsLoginSonyCert(int gameId,
					   int partnerCode,
					   int namespaceId,
					   const gsi_u8 * ps3cert,
					   int certLen,
					   WSLoginSonyCertCallback userCallback, 
					   void * userData)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable(WS_AUTHSERVICE_NAME))
		return WSLogin_NoAvailabilityCheck;

	GS_ASSERT(gameId >= 0);
	GS_ASSERT(partnerCode >= 0);
	GS_ASSERT(ps3cert != NULL);
	
	
	// Version check, todo: use something better than length
	//if (certLen != 248)
	//	return WSLogin_InvalidParameters;

	// allocate the request values
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSLogin_OutOfMemory;

	requestData->mUserCallback.mLoginSonyCertCallback = userCallback;
	requestData->mUserData     = userData;
	requestData->mTask = NULL;
	
	// create the xml request
	writer = gsXmlCreateStreamWriter(WS_AUTHSERVICE_NAMESPACES, WS_AUTHSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		char buffer[1024];

		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINPS3CERT)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "gameid", (gsi_u32)gameId)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "partnercode", (gsi_u32)partnerCode)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "namespaceid", (gsi_u32)namespaceId)) ||
			gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, "npticket")) ||
			gsi_is_false(gsXmlWriteBase64BinaryElement(writer, WS_AUTHSERVICE_NAMESPACE, "Value", ps3cert, certLen)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, "npticket")) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINPS3CERT)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSLogin_OutOfMemory;
		}
		
		strcpy(buffer, WS_AUTHSERVICE_LOGINPS3CERT_SOAP);
		strcat(buffer, "\r\nAccessKey: ");
		strcat(buffer, authCreds);

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, buffer,
			        writer, wsLoginSonyCertCallback, (void*)requestData);
		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSLogin_OutOfMemory;
		}
	}
	else
	{
		gsifree(requestData);
		return WSLogin_OutOfMemory;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void wsiLoginEncryptPassword(const gsi_char * password, gsi_u8 ciphertext[GS_CRYPT_RSA_BYTE_SIZE])
{
	gsCryptRSAKey sigkeypub;
	size_t passwordLen;


#ifdef GSI_UNICODE
	char password_A[WS_LOGIN_PASSWORD_LEN];
		// strip password into ascii to encrypt
	UCS2ToAsciiString(password, password_A);
#endif

	gsLargeIntSetFromHexString(&sigkeypub.modulus, WS_AUTHSERVICE_SIGNATURE_KEY);
	gsLargeIntSetFromHexString(&sigkeypub.exponent, WS_AUTHSERVICE_SIGNATURE_EXP);

#ifdef GSI_UNICODE
	passwordLen = _tcslen(password);

	GS_ASSERT(passwordLen <= UINT_MAX);

	gsCryptRSAEncryptBuffer(&sigkeypub, (const gsi_u8*)password_A, (unsigned int)passwordLen, ciphertext);
#else
	passwordLen = strlen(password);

	GS_ASSERT(passwordLen <= UINT_MAX);

	gsCryptRSAEncryptBuffer(&sigkeypub, (const gsi_u8*)password, (unsigned int)passwordLen, ciphertext);
#endif
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static gsi_u32 wsiMakeLittleEndian32(gsi_u32 num)
{
#if defined(GSI_BIG_ENDIAN)
	num = gsiByteOrderSwap32(num);
#endif
	return num;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool wsLoginCertIsValid(const GSLoginCertificate * cert)
{
	// Verify the signature
	gsCryptRSAKey sigkeypub;
	GSMD5_CTX md5;
	gsi_u8 hash[16];
	gsi_i32 cryptResult = 0;
	gsi_u32 temp;

	// hash certificate data
	GSMD5Init(&md5);
	temp = wsiMakeLittleEndian32(cert->mLength);
	GSMD5Update(&md5, (unsigned char*)&temp, 4);
	temp = wsiMakeLittleEndian32(cert->mVersion);
	GSMD5Update(&md5, (unsigned char*)&temp, 4);
	temp = wsiMakeLittleEndian32(cert->mPartnerCode);
	GSMD5Update(&md5, (unsigned char*)&temp, 4);
	temp = wsiMakeLittleEndian32(cert->mNamespaceId);
	GSMD5Update(&md5, (unsigned char*)&temp, 4);
	temp = wsiMakeLittleEndian32(cert->mUserId);
	GSMD5Update(&md5, (unsigned char*)&temp, 4);
	temp = wsiMakeLittleEndian32(cert->mProfileId);
	GSMD5Update(&md5, (unsigned char*)&temp, 4);
	temp = wsiMakeLittleEndian32(cert->mExpireTime);
	GSMD5Update(&md5, (unsigned char*)&temp, 4);

#if defined(GSI_UNICODE)
	{
		char profile_A[WS_LOGIN_NICK_LEN];
		char uniquenick_A[WS_LOGIN_UNIQUENICK_LEN];
		char keyhash_A[WS_LOGIN_KEYHASH_LEN];

		UCS2ToAsciiString(cert->mProfileNick, profile_A);
		UCS2ToAsciiString(cert->mUniqueNick, uniquenick_A);
		UCS2ToAsciiString(cert->mCdKeyHash, keyhash_A);

		GSMD5Update(&md5, (unsigned char*)profile_A, strlen(profile_A));        //FIX for unicode
		GSMD5Update(&md5, (unsigned char*)uniquenick_A, strlen(uniquenick_A));  //FIX for unicode
		GSMD5Update(&md5, (unsigned char*)keyhash_A, strlen(keyhash_A));        //FIX for unicode
	}
#else
	GSMD5Update(&md5, (unsigned char*)&cert->mProfileNick, (unsigned int)strlen(cert->mProfileNick)); 
	GSMD5Update(&md5, (unsigned char*)&cert->mUniqueNick, (unsigned int)strlen(cert->mUniqueNick));
	GSMD5Update(&md5, (unsigned char*)&cert->mCdKeyHash, (unsigned int)strlen(cert->mCdKeyHash));
#endif

	// must be hashed in big endian byte order
	//    skip leading zeroes
	gsLargeIntAddToMD5(&cert->mPeerPublicKey.modulus, &md5);
	gsLargeIntAddToMD5(&cert->mPeerPublicKey.exponent, &md5);

	GSMD5Update(&md5, (unsigned char*)&cert->mServerData, WS_LOGIN_SERVERDATA_LEN);
	GSMD5Final(hash, &md5);

	gsLargeIntSetFromHexString(&sigkeypub.modulus, WS_AUTHSERVICE_SIGNATURE_KEY);
	gsLargeIntSetFromHexString(&sigkeypub.exponent, WS_AUTHSERVICE_SIGNATURE_EXP);
	cryptResult = gsCryptRSAVerifySignedHash(&sigkeypub, hash, 16, cert->mSignature, WS_LOGIN_SIGNATURE_LEN);
	if (cryptResult == 0)
		return gsi_true;

    return gsi_false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Utility to write a certificate in binary form, does it really belong in this file?
#define WRITE_NETWORK_INT(a)  { \
                                  intNB = (gsi_i32)htonl(a); \
                                  if(lenoutSoFar + sizeof(intNB) > maxlen) \
                                      return gsi_false; \
                                  memcpy(bufout+lenoutSoFar, &intNB, sizeof(intNB)); \
								  lenoutSoFar += sizeof(intNB); }
#define WRITE_NTS(a)  { \
                                  if(lenoutSoFar + _tcslen(a) > maxlen) \
                                      return gsi_false; \
								  strcpy(bufout+lenoutSoFar, a); \
								  lenoutSoFar += (gsi_u32)_tcslen(a) + 1; }

#define WRITE_BINARY(a,l)  { \
                                  if(lenoutSoFar + l > maxlen) \
                                      return gsi_false; \
								  memcpy(bufout+lenoutSoFar, a, l); \
								  lenoutSoFar += l; }

#define WRITE_REV_BINARY(a,l)  { \
	                              int i=(gsi_i32)l; \
                                  const char * readPos = ((char*)a)+i-1; \
								  if (lenoutSoFar + l > maxlen) \
								      return gsi_false; \
                                  while(i > 0) \
                                  { \
								      *(bufout+lenoutSoFar) = *readPos; \
									  readPos--; lenoutSoFar++; i--; \
								  }; \
                               }

gsi_bool wsLoginCertWriteBinary(const GSLoginCertificate * cert, char * bufout, unsigned int maxlen, unsigned int * lenout)
{
	gsi_i32 intNB; // network byte order int
	gsi_i32 lenoutSoFar = 0; // tracks bytes written to bufout
	gsi_i32 lenTemp = 0; // for temp length of large ints
	gsi_i32 skipBytes = 0; // used to adjust for different host byte order on PS3 

	WRITE_NETWORK_INT(cert->mLength);
	WRITE_NETWORK_INT(cert->mVersion);
	WRITE_NETWORK_INT(cert->mPartnerCode);
	WRITE_NETWORK_INT(cert->mNamespaceId);
	WRITE_NETWORK_INT(cert->mUserId);
	WRITE_NETWORK_INT(cert->mProfileId);
	WRITE_NETWORK_INT(cert->mExpireTime);

#ifndef GSI_UNICODE
	WRITE_NTS(cert->mProfileNick);
	WRITE_NTS(cert->mUniqueNick);
	WRITE_NTS(cert->mCdKeyHash);
#else
	if((lenoutSoFar + _tcslen(cert->mProfileNick) + _tcslen(cert->mUniqueNick) + _tcslen(cert->mCdKeyHash)) > maxlen)
		return gsi_false;
	// strip unicode to Ascii before writing this to the buffer
	lenoutSoFar += UCS2ToAsciiString(cert->mProfileNick, bufout+lenoutSoFar)+1;
	lenoutSoFar += UCS2ToAsciiString(cert->mUniqueNick, bufout+lenoutSoFar)+1;
	lenoutSoFar += UCS2ToAsciiString(cert->mCdKeyHash, bufout+lenoutSoFar)+1;
#endif

	lenTemp = (gsi_i32)gsLargeIntGetByteLength(&cert->mPeerPublicKey.modulus); // size in bytes, no leading zeroes
#ifdef _PS3
	skipBytes = (lenTemp%4)==0 ? 0 : 4-(lenTemp%4);
#endif
	WRITE_NETWORK_INT(lenTemp); 
#ifdef _PS3
	WRITE_REV_BINARY( ((char*)cert->mPeerPublicKey.modulus.mData)+skipBytes, (unsigned int)lenTemp);
#else
	WRITE_REV_BINARY(cert->mPeerPublicKey.modulus.mData, (unsigned int)lenTemp);
#endif

	lenTemp = (gsi_i32)gsLargeIntGetByteLength(&cert->mPeerPublicKey.exponent); // size in bytes, no leading zeroes
#ifdef _PS3
	skipBytes = (lenTemp%4)==0 ? 0 : 4-(lenTemp%4);
#endif
	WRITE_NETWORK_INT(lenTemp);
#ifdef _PS3
	WRITE_REV_BINARY( ((char*)cert->mPeerPublicKey.exponent.mData)+skipBytes, (unsigned int)lenTemp);
#else
	WRITE_REV_BINARY(cert->mPeerPublicKey.exponent.mData, (unsigned int)lenTemp);
#endif

	WRITE_NETWORK_INT(WS_LOGIN_SERVERDATA_LEN); 
	WRITE_BINARY(cert->mServerData, (unsigned int)WS_LOGIN_SERVERDATA_LEN); 

	WRITE_NETWORK_INT(GS_CRYPT_RSA_BYTE_SIZE); 
	WRITE_BINARY(cert->mSignature, (unsigned int)WS_LOGIN_SERVERDATA_LEN); 

	*lenout = (gsi_u32)lenoutSoFar;
	GSI_UNUSED(skipBytes);
	return gsi_true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Utility to read the binary certificate

// each read macro first clears the buffer (a) before writing
#define READ_NTOH_INT(a)  { \
								  gsi_i32 temp; \
								  memset(&a, 0, sizeof(a)); \
								  memcpy(&temp, bufin, sizeof(gsi_i32)); \
								  intHB = (gsi_u32)ntohl(temp); \
                                  memcpy(&a, &intHB, sizeof(intHB)); \
								  if(lenoutSoFar + sizeof(intHB) > maxlen) \
                                      return gsi_false; \
								  bufin += sizeof(gsi_u32); \
								  lenoutSoFar += sizeof(intHB); }
#define READ_NTS(a,l)  { \
								  memset(&a, 0, sizeof(a)); \
								  _tcsncpy(a, bufin, l); \
								  if(lenoutSoFar + _tcslen(a)+1 > maxlen) \
                                      return gsi_false; \
								  bufin += _tcslen(a)+1; \
								  lenoutSoFar += (gsi_u32)_tcslen(a)+1; }

#define READ_ASCII(a,l)  { \
								  char temp[l]; \
								  memset(&a, 0, sizeof(a)); \
								  strncpy(temp, bufin, l); \
								  AsciiToUCS2String(temp, a); \
								  if(lenoutSoFar + _tcslen(a)+1 > maxlen) \
                                      return gsi_false; \
								  bufin += _tcslen(a)+1; \
								  lenoutSoFar += _tcslen(a)+1; }


#define READ_BINARY(a,l)  { \
								  int index = 0; \
								  memset(&a, 0, sizeof(a)); \
								  if(lenoutSoFar + l > maxlen) \
                                      return gsi_false; \
								  memcpy(a+index, bufin, l); \
								  lenoutSoFar += l; \
							 	  bufin += l; }

// seperate conversion to account for different host byte order on PS3 
#ifdef _PS3 
#define READ_REV_BINARY_TO_INT(a,l) { \
									int i=(gsi_i32)l; \
									int index = 0; \
									char temp[4]; \
									int skipBytes = (i%4==0) ? 0 : 4-(i%4); \
									const char * readPos = bufin+i-1; \
									memset(&a, 0, sizeof(a)); \
									if (lenoutSoFar + l > maxlen) \
									return gsi_false; \
									while(i > 0) \
									  { \
										  temp[index%4] = *readPos; \
										  if (index%4 == 3) \
										  memcpy(a+(index/4), temp, 4); \
										  else if (index == (gsi_i32)l-1) \
										  memcpy(((char*)a+(index/4))+skipBytes, temp, l); \
										  readPos--; index++; lenoutSoFar++; i--; \
									}; \
									bufin += l; }
#else
#define READ_REV_BINARY_TO_INT(a,l) { \
									int i=(gsi_i32)l; \
									int index = 0; \
									char temp[4]; \
									const char * readPos = bufin+i-1; \
									memset(&a, 0, sizeof(a)); \
									if (lenoutSoFar + l > maxlen) \
									return gsi_false; \
									while(i > 0) \
									{ \
										temp[index%4] = *readPos; \
										if (index%4 == 3) \
										memcpy(a+(index/4), temp, 4); \
										else if (index == (gsi_i32)l-1) \
										memcpy(a+(index/4), temp, l); \
										readPos--; index++; lenoutSoFar++; i--; \
									}; \
									bufin += l; }
#endif


gsi_bool wsLoginCertReadBinary(GSLoginCertificate * certOut, char * bufin, unsigned int maxlen)
{
	gsi_u32 intHB; // host byte order int
	gsi_u32 lenoutSoFar = 0; // tracks bufin index to make sure we dont exceed the maxlen
	gsi_u32 lenTemp = 0; // for temp length of large ints

	
	READ_NTOH_INT(certOut->mLength);
	READ_NTOH_INT(certOut->mVersion);
	READ_NTOH_INT(certOut->mPartnerCode);
	READ_NTOH_INT(certOut->mNamespaceId);
	READ_NTOH_INT(certOut->mUserId);
	READ_NTOH_INT(certOut->mProfileId);
	READ_NTOH_INT(certOut->mExpireTime);

#ifndef GSI_UNICODE

	READ_NTS(certOut->mProfileNick, WS_LOGIN_NICK_LEN);
	READ_NTS(certOut->mUniqueNick, WS_LOGIN_UNIQUENICK_LEN);
	READ_NTS(certOut->mCdKeyHash, WS_LOGIN_KEYHASH_LEN);
#else

	// parses ascii to unicode before writing into the buffer
	READ_ASCII(certOut->mProfileNick, WS_LOGIN_NICK_LEN);
	READ_ASCII(certOut->mUniqueNick, WS_LOGIN_UNIQUENICK_LEN);
	READ_ASCII(certOut->mCdKeyHash, WS_LOGIN_CDKEY_LEN);
#endif

	READ_NTOH_INT(lenTemp); //size of the modulus data in bytes	
	// now calculate the length in int's
	certOut->mPeerPublicKey.modulus.mLength = (lenTemp / GS_LARGEINT_DIGIT_SIZE_BYTES);
	if (lenTemp % GS_LARGEINT_DIGIT_SIZE_BYTES != 0)
		certOut->mPeerPublicKey.modulus.mLength++;

	READ_REV_BINARY_TO_INT(certOut->mPeerPublicKey.modulus.mData, lenTemp);

	READ_NTOH_INT(lenTemp); //size of the exponent data in bytes
	// now calculate the length in int's
	certOut->mPeerPublicKey.exponent.mLength = (lenTemp / GS_LARGEINT_DIGIT_SIZE_BYTES);
	if (lenTemp % GS_LARGEINT_DIGIT_SIZE_BYTES != 0)
		certOut->mPeerPublicKey.exponent.mLength++;

	READ_REV_BINARY_TO_INT(certOut->mPeerPublicKey.exponent.mData, lenTemp);

	bufin += sizeof(gsi_u32); //skip the prepended length
	READ_BINARY(certOut->mServerData, (unsigned int)WS_LOGIN_SERVERDATA_LEN); 

	bufin += sizeof(gsi_u32); //skip the prepended length
	READ_BINARY(certOut->mSignature, (unsigned int)WS_LOGIN_SERVERDATA_LEN); 

	return gsi_true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool wsLoginCertWriteXML(const GSLoginCertificate * cert, const char * aNamespace, GSXmlStreamWriter writer)
{
	GS_ASSERT(cert != NULL);
	GS_ASSERT(writer != NULL);

	if (gsi_is_false(gsXmlWriteIntElement(writer, aNamespace, "length",      cert->mLength))       ||
		gsi_is_false(gsXmlWriteIntElement(writer, aNamespace, "version",     cert->mVersion))      ||
		gsi_is_false(gsXmlWriteIntElement(writer, aNamespace, "partnercode", cert->mPartnerCode))  ||
		gsi_is_false(gsXmlWriteIntElement(writer, aNamespace, "namespaceid", cert->mNamespaceId))  ||
		gsi_is_false(gsXmlWriteIntElement(writer, aNamespace, "userid",      cert->mUserId))       ||
		gsi_is_false(gsXmlWriteIntElement(writer, aNamespace, "profileid",   cert->mProfileId))    ||
		gsi_is_false(gsXmlWriteIntElement(writer, aNamespace, "expiretime",  cert->mExpireTime))   ||
		gsi_is_false(gsXmlWriteAsciiStringElement(writer, aNamespace, "profilenick", cert->mProfileNick))||
		gsi_is_false(gsXmlWriteAsciiStringElement(writer, aNamespace, "uniquenick",  cert->mUniqueNick)) ||
		gsi_is_false(gsXmlWriteAsciiStringElement(writer, aNamespace, "cdkeyhash",   cert->mCdKeyHash))  ||
		gsi_is_false(gsXmlWriteLargeIntElement(writer, aNamespace, "peerkeymodulus", &cert->mPeerPublicKey.modulus)) ||
 		gsi_is_false(gsXmlWriteLargeIntElement(writer, aNamespace, "peerkeyexponent", &cert->mPeerPublicKey.exponent)) ||	
		gsi_is_false(gsXmlWriteHexBinaryElement(writer, aNamespace, "serverdata", cert->mServerData, WS_LOGIN_SERVERDATA_LEN)) ||
		gsi_is_false(gsXmlWriteHexBinaryElement(writer, aNamespace, "signature", cert->mSignature, WS_LOGIN_SIGNATURE_LEN))
		)
	{
		//gsLargeIntReverseBytes(&cert->mPeerPublicKey.modulus);
		//gsLargeIntReverseBytes(&cert->mPeerPublicKey.exponent);
		return gsi_false;
	}
		
	//gsLargeIntReverseBytes(&cert->mPeerPublicKey.modulus);
	//gsLargeIntReverseBytes(&cert->mPeerPublicKey.exponent);
	return gsi_true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
gsi_bool wsLoginCertReadXML(GSLoginCertificate * cert, GSXmlStreamReader reader)
{
	char hexstr[GS_CRYPT_RSA_BYTE_SIZE*2 +1]; // temp storage for key hex strings
	int hexlen;

	GS_ASSERT(cert != NULL);
	GS_ASSERT(reader != NULL);

	if (gsi_is_false(gsXmlReadChildAsInt      (reader, "length",     (int*)&cert->mLength))      ||
		gsi_is_false(gsXmlReadChildAsInt      (reader, "version",    (int*)&cert->mVersion))     ||
		gsi_is_false(gsXmlReadChildAsInt      (reader, "partnercode",(int*)&cert->mPartnerCode)) ||
		gsi_is_false(gsXmlReadChildAsInt      (reader, "namespaceid",(int*)&cert->mNamespaceId)) ||
		gsi_is_false(gsXmlReadChildAsInt      (reader, "userid",     (int*)&cert->mUserId))      ||
		gsi_is_false(gsXmlReadChildAsInt      (reader, "profileid",  (int*)&cert->mProfileId))   ||
		gsi_is_false(gsXmlReadChildAsInt      (reader, "expiretime", (int*)&cert->mExpireTime))  ||
		gsi_is_false(gsXmlReadChildAsTStringNT (reader, "profilenick", cert->mProfileNick, WS_LOGIN_NICK_LEN))       ||
		gsi_is_false(gsXmlReadChildAsTStringNT (reader, "uniquenick",  cert->mUniqueNick,  WS_LOGIN_UNIQUENICK_LEN)) ||
		gsi_is_false(gsXmlReadChildAsTStringNT (reader, "cdkeyhash",   cert->mCdKeyHash,   WS_LOGIN_KEYHASH_LEN))    ||

		gsi_is_false(gsXmlReadChildAsStringNT (reader, "peerkeymodulus", hexstr, GS_CRYPT_RSA_BYTE_SIZE*2 +1)) ||
		gsi_is_false(gsLargeIntSetFromHexString(&cert->mPeerPublicKey.modulus, hexstr)) ||

		gsi_is_false(gsXmlReadChildAsStringNT (reader, "peerkeyexponent", hexstr, GS_CRYPT_RSA_BYTE_SIZE*2 +1)) ||
		gsi_is_false(gsLargeIntSetFromHexString(&cert->mPeerPublicKey.exponent, hexstr)) ||

		gsi_is_false(gsXmlReadChildAsHexBinary(reader, "serverdata", cert->mServerData, WS_LOGIN_SERVERDATA_LEN, &hexlen)) ||

		gsi_is_false(gsXmlReadChildAsHexBinary(reader, "signature", cert->mSignature, WS_LOGIN_SIGNATURE_LEN, &hexlen))
		)
	{
		return gsi_false;
	}
	return gsi_true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// New functions from GameSpy Open SDK
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char* wsGetServiceUrl()
{
	if (__GSIACResult == GSIACAvailable)
	{
		if (!wsAuthServiceURL[0])
			snprintf(wsAuthServiceURL, sizeof(wsAuthServiceURL),
				WS_LOGIN_SERVICE_URL_FORMAT,
				__GSIACGamename, WS_AUTHSERVICE_NAME);

		return wsAuthServiceURL;
	}

	gsDebugFormat(GSIDebugCat_App, GSIDebugType_State, GSIDebugLevel_HotError, "No availability\r\n");
	return NULL;
}

const char* wsLoginValueString(int loginValue)
{
	switch (loginValue)
	{
	case WSLogin_Success:
		return "Login succeeded.";
	case WSLogin_ServerInitFailed:
		return "Server init failed.";
	case WSLogin_UserNotFound:
		return "User not found.";
	case WSLogin_InvalidPassword:
		return "Invalid password.";
	case WSLogin_InvalidProfile:
		return "Invalid profile.";
	case WSLogin_UniqueNickExpired:
		return "Unique nick expired.";
	case WSLogin_DBError:
		return "Database error.";
	case WSLogin_ServerError:
		return "Server error.";

	case WSLogin_HttpError:
		return "HTTP error.";
	case WSLogin_ParseError:
		return "Couldn't parse HTTP response.";
	case WSLogin_InvalidCertificate:
		return "Login succeeded, but an invalid certificate was returned.";
	case WSLogin_LoginFailed:
		return "Login failed.";
	case WSLogin_OutOfMemory:
		return "Out of memory.";
	case WSLogin_InvalidParameters:
		return "Invalid parameters.";
	case WSLogin_NoAvailabilityCheck:
		return "Availability check not yet performed.";
	case WSLogin_Cancelled:
		return "Login cancelled.";

	default:
		break;
	}

	return "An unknown error occurred."; // WSLogin_UnknownError
}

void wsSetGameCredentials(const char* accessKey, const int gameId, const char* secretKey)
{
	char buffer[20];
	GSSHA1Context sha1;
	const size_t accessKeyLen = strlen(accessKey), secretKeyLen = strlen(secretKey);

	GS_ASSERT(accessKeyLen <= UINT_MAX);
	GS_ASSERT(secretKeyLen <= UINT_MAX);

	strcpy(authCreds, accessKey);
	GSSHA1Reset(&sha1);
	GSSHA1Input(&sha1, (const uint8_t*)accessKey, (unsigned int)accessKeyLen);
	GSSHA1Input(&sha1, (const uint8_t*)secretKey, (unsigned int)secretKeyLen);
	GSSHA1Result(&sha1, (uint8_t*)&authCreds[33]);

	for (int i = 0; i < 20; ++i)
		sprintf(&authCreds[2 * i + 53], "%02x", (gsi_u8)authCreds[i + 33]);

	sprintf(buffer, "%d", gameId);
	gsiCoreSetGameId(buffer);
}

/////////////////////////////////////////////////////////////
// GameSpy 2012 SDK functions

#if _IPHONE
static void wsSyncFacebookCallback(GHTTPResult theResult,
	GSXmlStreamWriter theRequestXml,
	GSXmlStreamReader theResponseXml,
	void* theRequestData)
{
	WSIRequestData* requestData = (WSIRequestData*)theRequestData;

	WSSyncFacebookResponse response;
	int i;

	memset(&response, 0, sizeof(response));

	if (theResult == GHTTPRequestCancelled)
	{
		response.mResult = WSLogin_Cancelled;
		response.mErrorMsg = goastrdup("Login Request Cancelled");
	}
	else if (theResult != GHTTPSuccess)
	{
		response.mResult = WSLogin_HttpError;
		response.mErrorMsg = goastrdup("HTTP Error");
	}
	else
	{
		// try to parse the soap
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "SyncFacebookResult")))
		{
			response.mResult = WSLogin_ParseError;
			response.mErrorMsg = goastrdup("Parse Error, couldn't read SyncFacebookResult");
		}
		else
		{
			int responseMsgLen = 0;
			const char* responseMsg = NULL;

			// prepare response structure
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResult)) ||
				gsi_is_false(gsXmlReadAttributeAsString(theResponseXml, "responseMsg", &responseMsg, &responseMsgLen)))
			{
				// could not parse login response code
				response.mResult = WSLogin_ParseError;
				response.mErrorMsg = goastrdup("Parse Error, couldn't read login response code");
			}
			else if (response.mResult != WSLogin_Success)
			{
				// server reported an error into reponseCode
				response.mErrorMsg = (char*)gsimalloc(responseMsgLen + 1);

				if (response.mErrorMsg)
				{
					gsiSafeStrcpyA(response.mErrorMsg, responseMsg, responseMsgLen);
				}
				else
				{
					response.mErrorMsg = goastrdup("Out of memory");
				}
			}
			else if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "numBuddies", &response.mNumBuddies)))
			{
				response.mResult = WSLogin_ParseError;
				response.mErrorMsg = goastrdup("Parse Error, couldn't read number of buddies returned");
			}
			else
			{
				if (response.mNumBuddies > 0)
				{
					if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "buddies")))
					{
						response.mResult = WSLogin_ParseError;
						response.mErrorMsg = goastrdup("Parse Error, couldn't read number of buddies returned");
					}
					else
					{
						response.mBuddyList = (FacebookBuddy*)gsimalloc(sizeof(FacebookBuddy) * response.mNumBuddies);

						if (!response.mBuddyList) // Added by UniSpy! not present in the original sdk
						{
							response.mResult = WSLogin_OutOfMemory;
							response.mErrorMsg = goastrdup("Out of memory");
						}
						else
						{
							const char* buddyName = NULL;
							int buddyNameLen = 0;

							for (i = 0; i < response.mNumBuddies; i++)
							{
								if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "FacebookBuddy")) ||
									gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "Profileid", &response.mBuddyList[i].mProfileid)) ||
									gsi_is_false(gsXmlReadChildAsString(theResponseXml, "Name", &buddyName, &buddyNameLen)) ||
									gsi_is_false(gsXmlReadChildAsInt64(theResponseXml, "Uid", (gsi_i64*) & response.mBuddyList[i].mUid)))
								{
									response.mResult = WSLogin_ParseError;
									response.mErrorMsg = goastrdup("Parse Error, couldn't parse info from a FacebookBuddy returne");
									break;
								}

								response.mBuddyList[i].mName = gsimalloc(buddyNameLen + 1);

								if (!response.mBuddyList[i].mName)  // Added by UniSpy! not present in the original sdk
								{
									response.mResult = WSLogin_OutOfMemory;
									response.mErrorMsg = goastrdup("Out of memory");
									break;
								}

								gsiSafeStrcpyA(response.mBuddyList[i].mName, buddyName, buddyNameLen);
							}

							response.mErrorMsg = goastrdup("Success");
						}
					}
				}
				else
				{
					response.mErrorMsg = goastrdup("Success");
				}
			}
		}
	}

	// trigger the user callback
	if (requestData->mUserCallback.mSyncFacebookCallback != NULL)
	{
		requestData->mUserCallback.mSyncFacebookCallback(theResult, &response, requestData->mUserData);
	}

	gsifree(requestData);
	gsifree(response.mErrorMsg);

	if (response.mNumBuddies)
	{
		for (i = 0; i < response.mNumBuddies; i++)
			gsifree(response.mBuddyList[i].mName);

		gsifree(response.mBuddyList);
	}

	GSI_UNUSED(theRequestXml);
}


GSTask* wsSyncFacebook(
	int gameId,
	const GSLoginCertificate* certificate,
	const GSLoginPrivateData* privData,
	const char* apiKey,
	const char* sessionKey,
	const char* sessionSecret,
	WSSyncFacebookCallback callback,
	void* userData
)
{
	GSXmlStreamWriter writer;
	WSIRequestData* requestData = NULL;

	if (!wsiServiceAvailable(WS_BUDDYSERVICE_NAME))
		return NULL;

	GS_ASSERT(gameId >= 0);
	GS_ASSERT(certificate != NULL);
	GS_ASSERT(privData != NULL);
	GS_ASSERT(callback != NULL);
	GS_ASSERT(apiKey != NULL);
	GS_ASSERT(sessionKey != NULL);
	GS_ASSERT(sessionSecret != NULL);

	if (!callback)
		return NULL;

	// allocate the request values
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return NULL;

	requestData->mUserCallback.mSyncFacebookCallback = callback;
	requestData->mUserData = userData;
	requestData->mTask = NULL;

	// create the xml request
	writer = gsXmlCreateStreamWriter(WS_BUDDYSERVICE_NAMESPACES, WS_BUDDYSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		if (gsi_is_false(gsXmlWriteOpenTag(writer, WS_AUTHSERVICE_NAMESPACE, WS_BUDDYSERVICE_SYNCFACEBOOK)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, WS_AUTHSERVICE_NAMESPACE, "gameid", gameId)) ||
			gsi_is_false(gsXmlWriteOpenTag(writer, "ns1", "certificate")) ||
			gsi_is_false(wsLoginCertWriteXML(certificate, "ns1", writer)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, "ns1", "certificate")) ||
			gsi_is_false(gsXmlWriteHexBinaryElement(writer, "ns1", "proof", (gsi_u8*)privData->mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "apiKey", apiKey)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "sessionKey", sessionKey)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "sessionSecret", sessionSecret)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, WS_AUTHSERVICE_NAMESPACE, WS_BUDDYSERVICE_SYNCFACEBOOK)) ||
			gsi_is_false(gsXmlCloseWriter(writer))
			)
		{
			gsXmlFreeWriter(writer);
			return NULL;
		}

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, WS_BUDDYSERVICE_SYNCFACEBOOK_SOAP,
			writer, wsSyncFacebookCallback, (void*)requestData);
		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return NULL;
		}

	}
	else
	{
		gsifree(requestData);
		return NULL;
	}

	return requestData->mTask->mCoreTask;
}

static void wsLoginFacebookCallback(GHTTPResult theResult,
	GSXmlStreamWriter theRequestXml,
	GSXmlStreamReader theResponseXml,
	void* theRequestData)
{
	WSIRequestData* requestData = (WSIRequestData*)theRequestData;

	WSLoginFacebookResponse response;
	memset(&response, 0, sizeof(response));

	if (theResult == GHTTPRequestCancelled)
	{
		response.mLoginResult = WSLogin_Cancelled;
		response.mErrorMsg = goastrdup("Login Request Cancelled");
	}
	else if (theResult != GHTTPSuccess)
	{
		response.mLoginResult = WSLogin_HttpError;
		response.mErrorMsg = goastrdup("HTTP Error");
	}
	else
	{
		// try to parse the soap
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "LoginFacebookResult")))
		{
			response.mLoginResult = WSLogin_ParseError;
			response.mErrorMsg = goastrdup("Parse Error, couldn't read LoginFacebookResult");
		}
		else
		{
			int responseMsgLen = 0;
			const char* responseMsg = NULL;

			// prepare response structure
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mLoginResult)) ||
				gsi_is_false(gsXmlReadAttributeAsString(theResponseXml, "responseMsg", &responseMsg, &responseMsgLen)))
			{
				// could not parse login response code
				response.mLoginResult = WSLogin_ParseError;
				response.mErrorMsg = goastrdup("Parse Error, couldn't read login response code");
			}
			else if (response.mLoginResult != WSLogin_Success)
			{
				// server reported an error into reponseCode
				response.mErrorMsg = (char*)gsimalloc(responseMsgLen + 1);

				if (response.mErrorMsg)
				{
					gsiSafeStrcpyA(response.mErrorMsg, responseMsg, responseMsgLen);
				}
				else
				{
					response.mErrorMsg = goastrdup("Out of memory");
				}
			}
			else
			{
				const char* tokenStr = NULL;
				const char* challengeStr = NULL;
				int tokenLen = 0;
				int challengeLen = 0;

				// Check length of token+challenge, then read into memory
				if (gsi_is_false(gsXmlReadChildAsString(theResponseXml, "authToken", &tokenStr, &tokenLen)) ||
					gsi_is_false(gsXmlReadChildAsString(theResponseXml, "partnerChallenge", &challengeStr, &challengeLen)) ||
					(tokenLen >= WS_LOGIN_AUTHTOKEN_LEN || challengeLen >= WS_LOGIN_PARTNERCHALLENGE_LEN))
				{
					response.mLoginResult = WSLogin_ParseError;
					response.mErrorMsg = goastrdup("Parse Error, couldn't read token + challenge");
				}
				else
				{
					memcpy(response.mRemoteAuthToken, tokenStr, (gsi_u32)tokenLen);
					memcpy(response.mPartnerChallenge, challengeStr, (gsi_u32)challengeLen);
					response.mRemoteAuthToken[tokenLen] = '\0';
					response.mPartnerChallenge[challengeLen] = '\0';
					response.mErrorMsg = goastrdup("Success");
					__isAuthenticated = gsi_true;
				}
			}
		}
	}

	// trigger the user callback
	if (requestData->mUserCallback.mLoginFacebookCallback != NULL)
	{
		requestData->mUserCallback.mLoginFacebookCallback(theResult, &response, requestData->mUserData);
	}

	gsifree(requestData);
	gsifree(response.mErrorMsg);
	GSI_UNUSED(theRequestXml);
}

GSTask* wsLoginFacebook(
	int gameId,
	const gsi_u64 uid,
	const char* apiKey,
	const char* sessionKey,
	const char* sessionSecret,
	WSLoginFacebookCallback callback,
	void* userData
)
{
	GSXmlStreamWriter writer;
	WSIRequestData* requestData = NULL;

	if (!wsiServiceAvailable(WS_AUTHSERVICE_NAME))
	{
		GSAUTH_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_WarmError, "called without doing availability check.", NULL);
		return NULL;
	}

	if (!callback)
	{
		GSAUTH_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "user defined callback is NULL.", NULL);
		return NULL;
	}

	// allocate the request values
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
	{
		GSAUTH_DEBUG_LOG(GSIDebugType_Memory, GSIDebugLevel_HotError, "unable to allocate memory for request.", NULL);
		return NULL;
	}

	requestData->mUserCallback.mLoginFacebookCallback = callback;
	requestData->mUserData = userData;
	requestData->mTask = NULL;

	// create the xml request
	writer = gsXmlCreateStreamWriter(WS_AUTHSERVICE_NAMESPACES, WS_AUTHSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		if (gsi_is_false(gsXmlWriteOpenTag(writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINFACEBOOK)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement(writer, WS_AUTHSERVICE_NAMESPACE, "gameid", gameId)) ||
			gsi_is_false(gsXmlWriteInt64Element(writer, WS_AUTHSERVICE_NAMESPACE, "uid", uid)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "sessionKey", sessionKey)) ||
			gsi_is_false(gsXmlWriteStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "sessionSecret", sessionSecret)) ||
			gsi_is_false(gsXmlWriteCloseTag(writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_LOGINFACEBOOK)) ||
			gsi_is_false(gsXmlCloseWriter(writer))
			)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSAUTH_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "error creating soap request.", NULL);
			return NULL;
		}

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, WS_AUTHSERVICE_LOGINFACEBOOK_SOAP,
			writer, wsLoginFacebookCallback, (void*)requestData);
		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			GSAUTH_DEBUG_LOG(GSIDebugType_State, GSIDebugLevel_HotError, "error executing request.", NULL);
			return NULL;
		}
	}
	else
	{
		gsifree(requestData);
		return NULL;
	}

	return requestData->mTask->mCoreTask;

}

#endif

static void wsiCreateUserAccountCallback(GHTTPResult theResult,
								   GSXmlStreamWriter theRequestXml,
								   GSXmlStreamReader theResponseXml,
								   void * theRequestData)
{
	GHTTPResult translatedResult = theResult;
	WSIRequestData *requestData = (WSIRequestData *)theRequestData;

	WSCreateUserAccountResponse response;
	GSLoginCertificate *cert = &response.mCertificate;
	cert->mIsValid = gsi_false;

	memset(&response, 0, sizeof(response));

	if (theResult != GHTTPSuccess)
	{
		response.mCreateUserAccountResult = WSCreateUserAccount_HttpError;
	}
	else
	{
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			 gsi_is_false(gsXmlMoveToNext(theResponseXml, "CreateUserAccountResult")))
		{
			response.mCreateUserAccountResult = WSCreateUserAccount_ParseError;
		}
		else
		{
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResponseCode)))
			{
				response.mCreateUserAccountResult = WSCreateUserAccount_ParseError;
			}
			else if (response.mResponseCode != WSCreateUserAccount_Success)
			{
				response.mCreateUserAccountResult = WSCreateUserAccount_ServerError;
			}
			else if (gsi_is_false(gsXmlMoveToChild(theResponseXml, "certificate")) ||
				gsi_is_false(wsLoginCertReadXML(cert, theResponseXml)) ||
				gsi_is_false(gsXmlMoveToParent(theResponseXml)) ||
				gsi_is_false(gsXmlReadChildAsLargeInt(theResponseXml, "peerkeyprivate",
								&response.mPrivateData.mPeerPrivateKey.exponent))
					)
			{
				response.mCreateUserAccountResult = WSCreateUserAccount_ParseError;
			}
			else
			{
				GSMD5_CTX md5;

				// peer privatekey modulus is same as peer public key modulus
				memcpy(&response.mPrivateData.mPeerPrivateKey.modulus, &cert->mPeerPublicKey.modulus, sizeof(cert->mPeerPublicKey.modulus));

				// hash the private key
				//   -- we use the has like a password for simple authentication
				GSMD5Init(&md5);
				gsLargeIntAddToMD5(&response.mPrivateData.mPeerPrivateKey.exponent, &md5);
				GSMD5Final((unsigned char*)response.mPrivateData.mKeyHash, &md5);

				// verify certificate
				cert->mIsValid = wsLoginCertIsValid(cert);
				if (gsi_is_false(cert->mIsValid))
				{
					response.mCreateUserAccountResult = WSCreateUserAccount_InvalidCertificate;
				}
			}
		}
	}

	if (requestData->mUserCallback.mCreateUserAccountCallback != NULL)
	{
		WSCreateUserAccountCallback userCallback = (WSCreateUserAccountCallback)(requestData->mUserCallback.mCreateUserAccountCallback);
		(userCallback)(translatedResult, &response, requestData->mUserData);
	}

	gsifree(requestData);
	GSI_UNUSED(theRequestXml);
}

static void wsGetBuddyListCallback(GHTTPResult theResult, 
								   GSXmlStreamWriter theRequestXml,
								   GSXmlStreamReader theResponseXml,
								   void * theRequestData)
{

	WSGetBuddyListResponse response;
	WSIRequestData * requestData = (WSIRequestData*)theRequestData;
	int i;

	memset(&response, 0, sizeof(response));

	if (theResult == GHTTPRequestCancelled)
	{
		response.mResult = WSLogin_Cancelled;
		response.mErrorMsg = goastrdup("Login Request Cancelled");
	}
	else if (theResult != GHTTPSuccess)
	{
		response.mResult = WSLogin_HttpError;
		response.mErrorMsg = goastrdup("HTTP Error");
	}
	else if (theResult == GHTTPSuccess)
	{
		// try to parse the soap
		if (gsi_is_false(gsXmlMoveToStart(theResponseXml)) ||
			gsi_is_false(gsXmlMoveToNext(theResponseXml, "GetBuddyListWithNamesResult")))
		{
   			response.mResult = WSLogin_ParseError;
    		response.mErrorMsg = goastrdup("Parse Error, couldn't read GetBuddyListWithNamesResult");
		}
		else
		{
			int responseMsgLen = 0;
			const char* responseMsg = NULL;

			// prepare response structure
			if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "responseCode", (int*)&response.mResult)) || gsi_is_false(gsXmlReadChildAsString(theResponseXml, "responseMsg", &responseMsg, &responseMsgLen)))
			{
				response.mResult = WSLogin_ParseError;
				response.mErrorMsg = goastrdup("Parse Error, couldn't read buddy list response code");
			}
			else if (response.mResult != WSLogin_Success)
			{
				response.mErrorMsg = gsimalloc(responseMsgLen + 1);

				if (response.mErrorMsg)
					gsiSafeStrcpyA(response.mErrorMsg, responseMsg, responseMsgLen + 1);
				else
					response.mErrorMsg = goastrdup("Out of memory");
			}
			else if (gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "numBuddies", &response.mNumBuddies)))
			{
				response.mResult = WSLogin_ParseError;
				response.mErrorMsg = goastrdup("Parse Error, couldn't read number of buddies returned");
			}
			else
			{
				if (response.mNumBuddies > 0)
				{
					if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "buddies")))
					{
						response.mResult = WSLogin_ParseError;
						response.mErrorMsg = goastrdup("Parse Error, couldn't read number of buddies returned");
					}
					else
					{
						response.mBuddyList = (Buddy*)gsimalloc(sizeof(Buddy) * response.mNumBuddies);

						if (!response.mBuddyList) // Added by UniSpy! not present in the original sdk
						{
							response.mResult = WSLogin_OutOfMemory;
							response.mErrorMsg = goastrdup("Out of memory");
						}
						else
						{
							const char* buddyName = NULL;
							int buddyNameLen = 0;

							for (i = 0; i < response.mNumBuddies; i++)
							{
								if (gsi_is_false(gsXmlMoveToNext(theResponseXml, "Buddy")) ||
									gsi_is_false(gsXmlReadChildAsInt(theResponseXml, "Profileid", &response.mBuddyList[i].mProfileid)) ||
									gsi_is_false(gsXmlReadChildAsString(theResponseXml, "Name", &buddyName, &buddyNameLen)))
								{
									response.mResult = WSLogin_ParseError;
									response.mErrorMsg = goastrdup("Parse Error, couldn't parse info from a Buddy returned");
									break;
								}

								response.mBuddyList[i].mName = gsimalloc(buddyNameLen + 1);

								if (!response.mBuddyList[i].mName)  // Added by UniSpy! not present in the original sdk
								{
									response.mResult = WSLogin_OutOfMemory;
									response.mErrorMsg = goastrdup("Out of memory");
									break;
								}

								gsiSafeStrcpyA(response.mBuddyList[i].mName, buddyName, buddyNameLen);
							}

							response.mErrorMsg = goastrdup("Success");
						}
					}
				}
				else
				{
					response.mErrorMsg = goastrdup("Success");
				}
			}
		}
	}

	if (requestData->mUserCallback.mBuddyListCallback)
		requestData->mUserCallback.mBuddyListCallback(theResult, &response, requestData->mUserData);

	gsifree(requestData);
	gsifree(response.mErrorMsg);

	if (response.mNumBuddies)
	{
		for (i = 0; i < response.mNumBuddies; i++)
			gsifree(response.mBuddyList[i].mName);

		gsifree(response.mBuddyList);
	}
}

WSCreateUserAccountValue wsCreateUserAccount(
	int								partnerCode, 
	int								namespaceId, 
	const gsi_char					* email, 
	const gsi_char					* profileNick, 
	const gsi_char					* uniqueNick, 
	const gsi_char					* password, 
	WSCreateUserAccountCallback		userCallback, 
	void							* userData
)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;
	gsi_u8 encryptedPassword[GS_CRYPT_RSA_BYTE_SIZE];

	if (!wsiServiceAvailable(WS_AUTHSERVICE_NAME))
		return WSCreateUserAccount_NoAvailabilityCheck;

	GS_ASSERT(partnerCode >= 0);
	GS_ASSERT(email != NULL);
	GS_ASSERT(uniqueNick != NULL);
	GS_ASSERT(password != NULL);

	if (_tcslen(email) >= WS_LOGIN_EMAIL_LEN)
		return WSCreateUserAccount_InvalidParameters;

	if (_tcslen(profileNick) >= WS_LOGIN_NICK_LEN)
		return WSCreateUserAccount_InvalidParameters;

	if (_tcslen(uniqueNick) >= WS_LOGIN_UNIQUENICK_LEN)
		return WSCreateUserAccount_InvalidParameters;

	if (_tcslen(password) >= WS_LOGIN_PASSWORD_LEN)
		return WSCreateUserAccount_InvalidParameters;

	// allocate the request values
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return WSCreateUserAccount_OutOfMemory;

	requestData->mUserCallback.mCreateUserAccountCallback = userCallback;
	requestData->mUserData     = userData;
	requestData->mTask = NULL;

	// encrypt the password (includes safety padding and hash)
	wsiLoginEncryptPassword(password, encryptedPassword);

	// create the xml request
	writer = gsXmlCreateStreamWriter(WS_AUTHSERVICE_NAMESPACES, WS_AUTHSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_CREATEUSER)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "partnercode", (gsi_u32)partnerCode)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "namespaceid", (gsi_u32)namespaceId)) ||
			gsi_is_false(gsXmlWriteAsciiStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "email", email)) ||
			gsi_is_false(gsXmlWriteAsciiStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "profilenick", profileNick)) ||
			gsi_is_false(gsXmlWriteAsciiStringElement(writer, WS_AUTHSERVICE_NAMESPACE, "uniquenick", uniqueNick)) ||
			gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, "password")) ||
			gsi_is_false(gsXmlWriteHexBinaryElement(writer, WS_AUTHSERVICE_NAMESPACE, "Value", encryptedPassword, GS_CRYPT_RSA_BYTE_SIZE)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, "password")) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, WS_AUTHSERVICE_CREATEUSER)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return WSCreateUserAccount_OutOfMemory;
		}

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, WS_AUTHSERVICE_CREATEUSER_SOAP,
			        writer, wsiCreateUserAccountCallback, (void*)requestData);

		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return WSCreateUserAccount_OutOfMemory;
		}
	}
	else
	{
		gsifree(requestData);
		return WSCreateUserAccount_OutOfMemory;
	}

	return 0;
}


GSTask * wsGetBuddyList(
    int gameId, 
    const GSLoginCertificate * certificate,
    const GSLoginPrivateData * privData, 
    WSGetBuddyListCallback callback, 
    void * userData
)
{
	GSXmlStreamWriter writer;
	WSIRequestData * requestData = NULL;

	if (!wsiServiceAvailable(WS_BUDDYSERVICE_NAME))
		return NULL;
	
	GS_ASSERT(gameId >= 0);
	GS_ASSERT(certificate != NULL);
	GS_ASSERT(privData != NULL);
	GS_ASSERT(callback != NULL);
	
	if (!callback)
		return NULL;
	
	// allocate the request values
	requestData = (WSIRequestData*)gsimalloc(sizeof(WSIRequestData));
	if (requestData == NULL)
		return NULL;

	requestData->mUserCallback.mBuddyListCallback = callback;
	requestData->mUserData = userData;
	requestData->mTask = NULL;

	// create the xml request
	writer = gsXmlCreateStreamWriter(WS_BUDDYSERVICE_NAMESPACES, WS_BUDDYSERVICE_NAMESPACE_COUNT);
	if (writer != NULL)
	{
		if (gsi_is_false(gsXmlWriteOpenTag      (writer, WS_AUTHSERVICE_NAMESPACE, WS_BUDDYSERVICE_GETLIST)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "version", WS_AUTHSERVICE_PROTOVERSION)) ||
			gsi_is_false(gsXmlWriteIntElement   (writer, WS_AUTHSERVICE_NAMESPACE, "gameid", gameId)) ||
			gsi_is_false(gsXmlWriteOpenTag     (writer, "ns1", "certificate")) ||
            gsi_is_false(wsLoginCertWriteXML    (certificate, "ns1", writer)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, "ns1", "certificate")) ||
            gsi_is_false(gsXmlWriteHexBinaryElement(writer, "ns1", "proof", (gsi_u8*)privData->mKeyHash, GS_CRYPT_MD5_HASHSIZE)) ||
			gsi_is_false(gsXmlWriteCloseTag     (writer, WS_AUTHSERVICE_NAMESPACE, WS_BUDDYSERVICE_GETLIST)) ||
			gsi_is_false(gsXmlCloseWriter       (writer))
			)
		{
			gsXmlFreeWriter(writer);
			return NULL;
		}

		requestData->mTask = gsiExecuteSoap(wsAuthServiceURL, WS_BUDDYSERVICE_GETLIST_SOAP,
			        writer, wsGetBuddyListCallback, (void*)requestData);
		if (requestData->mTask == NULL)
		{
			gsXmlFreeWriter(writer);
			gsifree(requestData);
			return NULL;
		}

	}
	else
	{
		gsifree(requestData);
		return NULL;
	}

	return requestData->mTask->mCoreTask;
}
