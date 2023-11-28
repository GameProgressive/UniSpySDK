#if defined(_X360)

#include "../gsCommon.h"
#include "../gsMemory.h"
#include "../gsDebug.h"


#if GS_USE_REFLECTOR

gsi_u32 gsReflectorIP;
gsi_u16 gsReflectorPort;

// Make sure the ip and port are in network byte order for this function's parameters.
void gsSetReflectorAddress(gsi_u32 ip, gsi_u16 port)
{
	gsReflectorIP = ip;
	gsReflectorPort = port;
}

#endif GS_USE_REFLECTOR


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // _X360 only