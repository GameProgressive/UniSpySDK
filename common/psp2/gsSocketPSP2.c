///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsCommon.h"
#include "../gsPlatformSocket.h"

#include <net.h>
#include <libnetctl.h>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MAX_IPS  5

gsDnsCache* GGSDnsCache = NULL;

void *GSSceNetResolverFunctionAllocate
(
	SceSize size,
	SceNetId rid,
	const char *name,
	void *user
)
{
	return gsimalloc(size);
}

void GSSceNetResolverFunctionFree
(
	void *ptr,
	SceNetId rid,
	const char *name,
	void *user
)
{
	gsifree(ptr);
}

struct hostent* gsSocketGetHostByName(const char* name)
{
	// parameter check
	GS_ASSERT(name);
	{
		
		SceNetId rid = 0;
		int result = -1;
		#define GET_HOST_BY_NAME_BUFFER_SIZE 1024
		char *buf = gsimalloc(GET_HOST_BY_NAME_BUFFER_SIZE);
		IN_ADDR ip;

		static struct hostent ahostent;
		static char * aliases = NULL;
		static char * ipPtrs[MAX_IPS + 1];
		static unsigned int ips[MAX_IPS];

		SceNetResolverParam params;
		memset(&params, 0x00, sizeof(params));
		params.allocate = GSSceNetResolverFunctionAllocate;
		params.free = GSSceNetResolverFunctionFree;
		params.user = 0;

		ahostent.h_name = "";
		ahostent.h_aliases = &aliases;
		ahostent.h_addrtype = AF_INET;
		ahostent.h_addr_list = ipPtrs;

		gsDnsCache* cache = GGSDnsCache;

		while(cache)
		{
			// traverse the DNS cache to see if we have prefetched this hostname
			if( 0 == strcmp(name, cache->hostname) )
			{
				// found it!
				ip = cache->ip;
				result = 1;
				break;
			}

			cache = cache->next;			
		}

		if(-1 == result)
		{
			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Network, GSIDebugLevel_Notice, "gsSocketGetHostByName: cache miss: %s!\r\n", name);

			rid = sceNetResolverCreate("gsSocketGetHostByName", &params, 0);
	
			// this will block until completed
			result = sceNetResolverStartNtoa
			(
				rid,
				name,
				&ip,
				0, // (0 == default value of 1 second) (should we use GSI_RESOLVER_TIMEOUT instead? - Josh)
				0, // (0 == default value of 5 times) (should we use GSI_RESOLVER_RETRY instead? - Josh)
				0  // (no flags == block until complete)
			);

			sceNetResolverDestroy(rid);
		}

		if(result >= 0)
		{
			ahostent.h_length = sizeof(IN_ADDR);
			memcpy(&ips[0], &ip, sizeof(IN_ADDR));
			ahostent.h_addr_list[0] = (char*)&ips[0];
			ahostent.h_addr_list[1] = NULL;
		}
		

		if(buf)
		{
			const char *out = sceNetInetNtop(SCE_NET_AF_INET, &ip, buf, GET_HOST_BY_NAME_BUFFER_SIZE);
			gsDebugFormat(GSIDebugCat_Common, GSIDebugType_Network, GSIDebugLevel_Notice, "gsSocketGetHostByName: %s = out:%s buf:%s\r\n", name, out?out:"void", buf?buf:"void");
		}
		gsifree(buf);
		return &ahostent;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char* gsSocketInetNtoa(in_addr in)
{
	static char buf[sizeof("XXX.XXX.XXX.XXX")];
	sceNetInetNtop(SCE_NET_AF_INET, &in, buf, sizeof(buf));
	return buf;
}

gsi_u32 inet_addr(const char * name)
{
	int ret;
	gsi_u32 result = 0;
	ret = sceNetInetPton(AF_INET, name, &result);
	if(ret <= 0)
	{
		// 0 == Invalid character string provided.
		// < 0 means error, but we treat that all the same at this layer. (Josh)
		result = INADDR_NONE;
	}
	return result;
}

HOSTENT * getlocalhost(void)
{
	#define MAX_IPS  5
	static HOSTENT localhost;
	static char * aliases = NULL;
	static char * ipPtrs[MAX_IPS + 1];
	static unsigned int ips[MAX_IPS];
	//int result = 0;
	gsi_u32 addr;

	static SceNetCtlInfo info;

	localhost.h_name = "localhost";
	localhost.h_aliases = &aliases;
	localhost.h_addrtype = AF_INET;
	localhost.h_length = 0;
	localhost.h_addr_list = (char **)ipPtrs;
	ipPtrs[0] = (char *)&ips[0];
	ipPtrs[1] = NULL;
	ips[0] = 0;

	int Error = sceNetCtlInetGetInfo(SCE_NET_CTL_INFO_IP_ADDRESS, &info);
	if(Error < 0)
	{
		printf("getlocalhost sceNetCtlInetGetInfo returned %d\r\n", Error);
		return NULL;
	}

	// fill in the hostent structure
	addr = inet_addr(info.ip_address); // NBO
	memcpy(&ips[0], &addr, sizeof(addr)); // still NBO
	localhost.h_length = (gsi_u16)sizeof(addr);

	return &localhost;
}

#define MAX_EVENTS	3

int vitaSocketselect(
	SOCKET theSocket, 
	int* theReadFlag, 
	int* theWriteFlag, 
	int* theExceptFlag)
{
	SceNetId eid;
	SceNetEpollEvent event;
	SceNetEpollEvent events[MAX_EVENTS];
	int nevents;
	int ret = 0;
	int i;

	if( !theReadFlag &&
		!theWriteFlag && 
		!theExceptFlag )
	{
		// there is nothing to test for, just ditch.
		return WSAEINVAL;
	}

	eid = sceNetEpollCreate("select", 0);
	if (eid < 0) 
	{

		if(theExceptFlag)
			*theExceptFlag = 1;
		if(theWriteFlag)
			*theWriteFlag = 0;
		if(theReadFlag)
			*theReadFlag = 0;

		return 1;
	}

	memset(&event, 0, sizeof(event));
	event.events = (theReadFlag != NULL?SCE_NET_EPOLLIN:0) | (theWriteFlag != NULL?SCE_NET_EPOLLOUT:0);
	event.data.ext.id = theSocket;
	ret = sceNetEpollControl(eid, SCE_NET_EPOLL_CTL_ADD, theSocket, &event);

	if( ret < 0 )
	{
		if(theExceptFlag)
			*theExceptFlag = 1;
		if(theWriteFlag)
			*theWriteFlag = 0;
		if(theReadFlag)
			*theReadFlag = 0;
		
		sceNetEpollDestroy(eid);
		return 1;
		
	}

	nevents = sceNetEpollWait(eid, events, MAX_EVENTS, 0);
	if (nevents < 0) 
	{
		if(theExceptFlag)
			*theExceptFlag = 1;
		if(theWriteFlag)
			*theWriteFlag = 0;
		if(theReadFlag)
			*theReadFlag = 0;

		sceNetEpollDestroy(eid);
		return 1;
	}

	// clear the flags before we check
	if(theExceptFlag)
		*theExceptFlag = 0;
	if(theWriteFlag)
		*theWriteFlag = 0;
	if(theReadFlag)
		*theReadFlag = 0;


	ret = 0;
	for (i = 0; i < nevents; i++) 
	{
		if (events[i].events & SCE_NET_EPOLLIN) 
		{
			if( theReadFlag )
			{
				*theReadFlag = 1;
				ret = 1;
			}
		}
		if (events[i].events & SCE_NET_EPOLLOUT) 
		{
			if( theWriteFlag )
			{
				*theWriteFlag = 1;
				ret = 1;
			}
		}
		if (events[i].events & (SCE_NET_EPOLLERR | SCE_NET_EPOLLHUP)) 
		{
			if( theExceptFlag )
			{
				*theExceptFlag = 1;
				ret = 1;
			}
		}
	}

	sceNetEpollDestroy(eid);
	return ret;
}
