#define PINGER_UDP_PING_SIZE  64
#include "../pinger.h"

#ifdef _WIN32
#include <process.h>
#include <conio.h>
#define s_addr S_un.S_addr
#else
#include <arpa/inet.h>
#define getch getchar
#define Sleep sleep
#define _getpid getpid
#endif

void pinged(unsigned int IP,
			unsigned short port,
			int ping,
			const char * data,
			int len,
			void * param)
{
	IN_ADDR addr;
	addr.s_addr = IP;

	printf("PINGED!\n"
	       "%s:%d %dms", inet_ntoa(addr), ntohs(port), ping);

	if(data)
		printf(" (%s)\n", data);
	else
		printf("\n");
}

void reply(unsigned int IP,
		   unsigned short port,
		   int ping,
		   const char * data,
		   int len,
		   void * param)
{
	IN_ADDR addr;
    addr.s_addr = IP;
	printf("REPLY!\n"
	       "%s:%d %dms", inet_ntoa(addr), ntohs(port), ping);

	if(data)
		printf(" (%s)\n", data);
	else
		printf("\n");
}

void setData(unsigned int IP, unsigned short port, char * data, int len, void * param)
{
	char buffer[32];
	sprintf(buffer, "my pid is %d", _getpid());
	GS_ASSERT((strlen(buffer) + 1) <= (unsigned int)len);
	strcpy(data, buffer);
}

int CheckKeyHit(void)
{
	// Check for a key press.
	/////////////////////////
	if(_kbhit())
	{
		// What char was pressed?
		/////////////////////////
		return getch();
	}

	return -1;
}

int main(int argc, char * argv[])
{
	int c;
	u_short localPort = htons(2222);
	u_short remotePort = htons(2222);
	unsigned int remoteIP;
	char * remoteAddress;

	GS_ASSERT(argc == 2);
	remoteAddress = argv[1];

	if(remoteAddress != NULL)
	{
		// Check for "a.b.c.d".
		///////////////////////
		remoteIP = inet_addr(remoteAddress);
		if(remoteIP == INADDR_NONE)
		{
			HOSTENT * hostent;

			// Check for "machine.host.domain".
			///////////////////////////////////
			hostent = gethostbyname(remoteAddress);
			if(hostent == NULL)
			{
				GS_FAIL();
				return 0;
			}

			// Grab the IP.
			///////////////
			GS_ASSERT(remoteIP != 0);
			remoteIP = *(unsigned int *)hostent->h_addr_list[0];
		}
	}
	else
	{
		// Any IP.
		//////////
		remoteIP = INADDR_ANY;
	}

	pingerInit(NULL, localPort, pinged, NULL, setData, NULL);

	do
	{
		c = CheckKeyHit();
		if(c != -1)
		{
			if(c == 'p')
			{
				pingerPing(remoteIP, remotePort, reply, NULL, PINGERFalse, 1000);
			}
			else if(c == 'q')
			{
				break;
			}
		}

		pingerThink();

		Sleep(1);
	}
	while(c != 'q');

	pingerShutdown();

	return 0;
}

