//=============================================================================
//
/// @file  OwSystemTime.cpp
/// @brief  Interface to Linux time management
//
//=============================================================================

#include  <time.h>
#include  "OwSystemTime.h"

//=============================================================================
/// This function sets the current Linux system time.
/// @param timestamp Current time as input
/// @sa Retrieve the time with OwSystemTime::get_ms()

void
OwSystemTime::set( PiTime timestamp_ms )
{
	long long  seconds = timestamp_ms / 1000;
	long long  remaining_milliseconds = timestamp_ms - seconds * 1000;

	struct timespec  ts;
	ts.tv_sec = seconds;
	ts.tv_nsec = remaining_milliseconds * 1000000;

	clock_settime( CLOCK_REALTIME, &ts );
}


//=============================================================================
/// This function sets the current Linux system time.
/// @param timestamp Current time as input
/// @sa Retrieve the time with OwSystemTime::get_seconds()

void
OwSystemTime::set_seconds( PiTime timestamp_sec )
{
	struct timespec  ts;
	ts.tv_sec = timestamp_sec;
	ts.tv_nsec = 0;

	clock_settime( CLOCK_REALTIME, &ts );
}


//=============================================================================
/// This function gets the current Linux system time.
/// @return The current Linux system time in milliseconds

PiTime
OwSystemTime::get_ms()
{
	PiTime  result = 0;
	struct timespec  ts;

	int  res = clock_gettime( CLOCK_REALTIME, &ts );

	if ( 0 == res )
	{
		result = ( PiTime ) ts.tv_sec;
		result *= 1000;
		result += ( PiTime )( ts.tv_nsec / 1000000 );
	}

	return result;
}


//=============================================================================
/// This function gets the current Linux system time.
/// @return  The current Linux system time in seconds

PiTime
OwSystemTime::get_seconds()
{
	PiTime  result = 0;
	struct timespec  ts;

	int  res = clock_gettime( CLOCK_REALTIME, &ts );

	if ( 0 == res )
	{
		result = ( PiTime ) ts.tv_sec;
	}

	return result;
}


//=============================================================================

