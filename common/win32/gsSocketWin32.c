// UnispySDK 24-02-2024
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_POLLING
struct GSIPoll
{
	WSAPOLLFD polls[FD_SETSIZE];
};

POLL GSISocketPollNew(void)
{
	int i;
	struct GSIPoll* p = (struct GSIPoll*)malloc(sizeof(struct GSIPoll));
	if (!p)
		return NULL;

	for (i = 0; i < FD_SETSIZE; i++)
	{
		p->polls[i].fd = INVALID_SOCKET;
		p->polls[i].revents = 0;
		p->polls[i].events = 0;
	}
	return p;
}

void GSISocketPollFree(POLL thePoll)
{
	(void)thePoll;
}

int GSISocketPollAdd(POLL thePoll, SOCKET theSocket, int flags)
{
	int i;
	struct GSIPoll* p = (struct GSIPoll*)thePoll;

	if (!p)
		return GS_POLLERR_INVALID;

	for (i = 0; i < FD_SETSIZE; i++)
	{
		if (theSocket == p->polls[i].fd)
			break;
	}

	if (i >= FD_SETSIZE)
		return GS_POLLERR_MAX_FD;

	p->polls[i].fd = theSocket;
	p->polls[i].events = 0;
	p->polls[i].revents = 0;
	if (flags & GS_POLLFLAG_READ)
		p->polls[i].events |= POLLRDBAND | POLLRDNORM;
	if (flags & GS_POLLFLAG_WRITE)
		p->polls[i].revents |= POLLWRNORM;

	return GS_POLLERR_NONE;
}

int GSISocketPollRemove(POLL thePoll, SOCKET theSocket)
{
	int i;
	struct GSIPoll* p = (struct GSIPoll*)thePoll;

	if (!p)
		return GS_POLLERR_INVALID;

	for (i = 0; i < FD_SETSIZE; i++)
	{
		if (p->polls[i].fd == theSocket)
		{
			p->polls[i].fd = INVALID_SOCKET;
			p->polls[i].revents = 0;
			p->polls[i].events = 0;
			return GS_POLLERR_NONE;
		}
	}

	return GS_POLLERR_INVALID;
}

int GSISocketPoll(POLL thePoll, POLL_EVTS* theEvents)
{
	int r;
	int i, q;
	struct GSIPoll* p = (struct GSIPoll*)thePoll;

	if (!p || !theEvents)
		return GS_POLLERR_INVALID;

	r = WSAPoll((LPWSAPOLLFD)thePoll, FD_SETSIZE, 0);

	if (r == 0)
		return GS_POLLERR_WAIT;

	if (r < 0)
		return GS_POLLERR_INVALID;

	for (i = 0; i < FD_SETSIZE; i++)
	{
		int rv = p->polls[i].revents;

		if (p->polls[i].fd == INVALID_SOCKET || rv == 0 || rv & POLLNVAL)
			continue;

		(*theEvents)[q].flags = 0;
		(*theEvents)[q].fd = p->polls[i].fd;

		if (rv & POLLIN)
			(*theEvents)[q].flags |= GS_POLLFLAG_READ;
		if (rv & POLLOUT)
			(*theEvents)[q].flags |= GS_POLLFLAG_WRITE;
		if (rv & POLLERR || rv & POLLHUP)
			(*theEvents)[q].flags |= GS_POLLFLAG_ERROR;

		q++;
	}

	return q;
}

int GSISocketPollCheck(POLL_EVTS* theEvents, SOCKET theSocket, int socketNum, int* theReadFlag, int* theWriteFlag, int* theExceptFlag)
{
	if (!theEvents)
		return GS_POLLERR_INVALID;

	for (int i = 0; i < socketNum; i++)
	{
		if ((*theEvents)[i].fd != theSocket)
			continue;

		if (theReadFlag)
			*theReadFlag = (*theEvents)[i].flags & GS_POLLFLAG_READ;

		if (theWriteFlag)
			*theWriteFlag = (*theEvents)[i].flags & GS_POLLFLAG_WRITE;

		if (theExceptFlag)
			*theExceptFlag = (*theEvents)[i].flags & GS_POLLFLAG_ERROR;

		return GS_POLLERR_NONE;
	}

	return GS_POLLERR_INVALID;
}
#endif
