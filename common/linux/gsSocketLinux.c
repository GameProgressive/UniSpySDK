// UnispySDK 24-02-2024
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsPlatformUtil.h"
#include "../gsPlatformSocket.h"
#include "../gsAssert.h"
#include "../gsDebug.h"

#include <sys/epoll.h>


int GSISocketPollAdd(SOCKET epollFd, SOCKET theSocket, int flags)
{

}

int GSISocketPollRemove(SOCKET epollFd, SOCKET theSocket);
int GSISocketPoll(SOCKET epollFd, POLL_FD* pollFd);
int GSISocketPollCheck(POLL_FD* poll, SOCKET* theSocket, int socketNum, int* theReadFlag, int* theWriteFlag, int* theExceptFlag);


int GSISocketSelect(SOCKET epollFd, SOCKET theSocket, int* theReadFlag, int* theWriteFlag, int* theExceptFlag)
{
    struct epoll_event event;
    struct epoll_event events[MAX_EPOLL_EVENTS];
    int ret;
    int i;

    event.data.fd = theSocket;
    event.events = 0;

    if (theReadFlag != NULL)
        event.events |= EPOLLIN;
    if (theWriteFlag != NULL)
        event.events |= EPOLLOUT;
    if (theExceptFlag != NULL)
        event.events |= EPOLLERR;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, theSocket, &event) != 0)
        return -1;

    ret = epoll_wait(epollFd, &evts, MAX_EPOLL_EVENTS, 0);

	if(gsiSocketIsError(ret))
		return -1;

    if (i = 0; i < ret; i++)
    {
        if (events[i].data.fd == theSocket)
        {
            if ((events[i].events & EPOLLIN) && theReadFlag != NULL)
                *theReadFlag = 1;
            if ((events[i].events & EPOLLOUT) && theWriteFlag != NULL)
                *theWriteFlag = 1;
            if ((events[i].events & EPOLLERR) && theExceptFlag != NULL)
                *theExceptFlag = 1;
        }
    }

    epoll_ctl(epollFd, EPOLL_CTL_DEL, theSocket, &event); // remove the socket from the queue

    if (ret == 0)
        return 0;

    return 1;
}
