#if defined(_PSP2)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsCommon.h"

#include <net.h>
#include <libnetctl.h>

#if defined(_PSP2) && defined(UNIQUEID)

	static const char* GetMAC(void)
	{
		static char Result[SCE_NET_ETHER_ADDR_LEN];
		SceNetCtlInfo EtherNetInfo;
		int Error = sceNetCtlInetGetInfo(SCE_NET_CTL_INFO_ETHER_ADDR, &EtherNetInfo);
		if
		(
			0 == Error
		&&	EtherNetInfo.bssid.data
		)
		{
			memcpy(Result, EtherNetInfo.bssid.data, SCE_NET_ETHER_ADDR_LEN);
		}
		return Result;
	}

	static const char * GOAGetUniqueID_Internal(void)
	{
		static char keyval[17];
		const char * MAC;

		// check if we already have the Unique ID
		if(keyval[0])
			return keyval;

		// get the MAC
		MAC = GetMAC();
		if(!MAC)
		{
			// error getting the MAC
			static char errorMAC[6] = { 1, 2, 3, 4, 5, 6 };
			MAC = errorMAC;
		}

		// format it
		sprintf(keyval, "%02X%02X%02X%02X%02X%02X0000",
			MAC[0] & 0xFF,
			MAC[1] & 0xFF,
			MAC[2] & 0xFF,
			MAC[3] & 0xFF,
			MAC[4] & 0xFF,
			MAC[5] & 0xFF);

		return keyval;
	}
#endif

gsi_i64 gsiStringToInt64(const char *theNumberStr)
{
	return atoll(theNumberStr);
}

void gsiInt64ToString(char theNumberStr[33], gsi_i64 theNumber)
{
	// you want to fit the number! 
	// give me a valid string!
	GS_ASSERT(theNumberStr != NULL);

	sprintf(theNumberStr, "%lld", theNumber);
}

double gsiWStringToDouble(const wchar_t *inputString)
{
	return wcstod(inputString, NULL);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // _PSP2 only
