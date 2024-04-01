///////////////////////////////////////////////////////////////////////////////
// File:	gsUtilPs2.c
// SDK:		GameSpy Common EE code
//
// Copyright (c) IGN Entertainment, Inc.  All rights reserved.  
// This software is made available only pursuant to certain license terms offered
// by IGN or its subsidiary GameSpy Industries, Inc.  Unlicensed use or use in a 
// manner not expressly authorized by IGN or GameSpy is prohibited.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "../gsCommon.h"

gsi_i64 gsiStringToInt64(const char *theNumberStr)
{
	return atol(theNumberStr);
}

void gsiInt64ToString(char theNumberStr[33], gsi_i64 theNumber)
{
	// you want to fit the number! 
	// give me a valid string!
	GS_ASSERT(theNumberStr != NULL);

	sprintf(theNumberStr, "%ld", theNumber);
}

void gsiDebugPrint(const char* format, va_list params) // unics
{
	vprintf(format, params);
}

void* _gsi_memalign(size_t boundary, size_t size)
{
	return memalign(boundary, size);
}

extern int sceCdReadClock();

#if !defined(__MWERKS__) && !defined(_PS2)
typedef unsigned char u_char;
#endif

typedef struct {
        u_char stat;            /* status */
        u_char second;          /* second */
        u_char minute;          /* minute */
        u_char hour;            /* hour   */

        u_char pad;             /* pad    */
        u_char day;             /* day    */
        u_char month;           /* month  */
        u_char year;            /* year   */
} sceCdCLOCK;

static unsigned long GetTicks()
{
	unsigned long ticks;
	asm volatile (" mfc0 %0, $9 " : "=r" (ticks));
    return ticks;
}

#define DEC(x) (10*(x/16)+(x%16))
#define _BASE_YEAR 			 70L
#define _MAX_YEAR 			138L
#define _LEAP_YEAR_ADJUST 	 17L
int _days[] = {-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};

static time_t _gmtotime_t (
        int yr,     /* 0 based */
        int mo,     /* 1 based */
        int dy,     /* 1 based */
        int hr,
        int mn,
        int sc
        )
{
        int tmpdays;
        long tmptim;
        struct tm tb;

        if ( ((long)(yr -= 1900) < _BASE_YEAR) || ((long)yr > _MAX_YEAR) )
                return (time_t)(-1);

        tmpdays = dy + _days[mo - 1];
        if ( !(yr & 3) && (mo > 2) )
                tmpdays++;

        tmptim = (long)yr - _BASE_YEAR;

        tmptim = ( ( ( ( tmptim ) * 365L
                 + ((long)(yr - 1) >> 2) - (long)_LEAP_YEAR_ADJUST
                 + (long)tmpdays )
                 * 24L + (long)hr )
                 * 60L + (long)mn )
                 * 60L + (long)sc;

        tb.tm_yday = tmpdays;
        tb.tm_year = yr;
        tb.tm_mon = mo - 1;
        tb.tm_hour = hr;
        
        return (tmptim >= 0) ? (time_t)tmptim : (time_t)(-1);
}

time_t time(time_t *timer)
{
	time_t tim;
	sceCdCLOCK clocktime; /* defined in libcdvd.h */

	sceCdReadClock(&clocktime); /* libcdvd.a */

	tim =   _gmtotime_t ( DEC(clocktime.year)+2000,
							DEC(clocktime.month),
							DEC(clocktime.day),
							DEC(clocktime.hour),
							DEC(clocktime.minute),
							DEC(clocktime.second));

	if(timer)
		*timer = tim;
		
	return tim;
}

gsi_time current_time_hires()  // returns current time in microseconds
{
	unsigned int ticks;
	static unsigned int msec = 0;
	static unsigned int lastticks = 0;
	sceCdCLOCK lasttimecalled; /* defined in libcdvd.h */

	if(!msec)
	{
		sceCdReadClock(&lasttimecalled); /* libcdvd.a */
		msec =  (unsigned int)(DEC(lasttimecalled.day) * 86400000) +
				(unsigned int)(DEC(lasttimecalled.hour) * 3600000) +
				(unsigned int)(DEC(lasttimecalled.minute) * 60000) +
				(unsigned int)(DEC(lasttimecalled.second) * 1000);
		msec *= 1000;
	}

	ticks = (unsigned int)GetTicks();
	if(lastticks > ticks)
		msec += ((sizeof(unsigned int) - lastticks) + ticks) / 300;
	else
		msec += (unsigned int)(ticks-lastticks) / 300;
	lastticks = ticks;

	return msec;
}

void msleep(gsi_time msec)
{
	#ifdef SN_SYSTEMS
		sn_delay((int)msec);
	#endif
	#ifdef EENET
		if(msec >= 1000)
		{
			sleep(msec / 1000);
			msec -= (msec / 1000);
		}
		if(msec)
			usleep(msec * 1000);
	#endif
	#ifdef INSOCK
		DelayThread(msec * 1000);
	#endif
}
