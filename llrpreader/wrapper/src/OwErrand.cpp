//=============================================================================
//
/// @file  OwErrand.cpp
///
/// @brief  This file defines data members and member functions for
/// 	the OwErrand and OwErrandRunner objects.
//
//=============================================================================

#include  <string.h>

#include  "SysTypes.h"
#include  "OwErrand.h"
#include  "OwSystemTime.h"
#include  "SPrintf.h"

//=============================================================================
// Static data members

OwErrandRunner**  OwErrandRunner::instance = 0;

//=============================================================================
// Constructor of OwErrand

OwErrand::OwErrand
(
	unsigned int  interval,
	const char  *pcName,
	unsigned int  runner
) :
	mInterval( interval ),
	elapse( interval ),
	started( false ),
	runner( runner )
{
	OwErrandRunner::getInstance( runner )->registerTask( this );
	strncpy( name, pcName, MAX_NAME_LENGTH - 1 );
}

//=============================================================================
// Destructor of OwErrand

OwErrand::~OwErrand()
{
	OwErrandRunner::getInstance( runner )->unregisterTask( this );
}

//=============================================================================
// Constructor of OwErrandRunner

OwErrandRunner::OwErrandRunner( unsigned int  id ):
	OwTask( OwTask::MEDIUM_HIGH, 4096, "OwErrandRunner" ),
	id( id ),
	mNoSleepCount( 0 ),
	mIterations( 0 )
{
	for ( UWORD  i = 0; i < OwErrand::MAX_RUNNER; i++ )
	{
		mEventCount[ i ] = 0;
		mElapsedTime[ i ] = 0;
	}
#if 0
	char  commandName[ 32 ];

	sprintf( commandName, "show_times_%d", id );

	subscribe(
		commandName,
		&OwErrandRunner::process_show_times,
		"Syntax: show_times_N"NL
		"Display the total elapsed time for each task managed"NL );
#endif
	run();
}

//=============================================================================
// OwErrandRunner::main

void
OwErrandRunner::main
(
	OwTask*
)
{
	set<OwErrand*>::const_iterator  ip;
	PiTime  entryTime_ms;
	PiTime  exitTime_ms;
	PiTime  sleepTime_ms;
	PiTime  startTime_ms;
	PiTime  endTime_ms;
	UWORD  i;

//	char*	pcName;

	while ( true )
	{
		mIterations++;
		entryTime_ms = OwSystemTime::get_ms();  // Get a time stamp before execution

		mutex.take( PI_FOREVER );

		for ( ip = registry.begin(), i = 0; ip != registry.end(); ip++, i++ )
		{
			if ( (*ip)->isStarted() && (*ip)->isTimeout( INTERVAL ) )
			{
				startTime_ms = OwSystemTime::get_ms();	// Get a time stamp before execution
				(*ip)->task();
//				pcName = (*ip)->getName();
				endTime_ms = OwSystemTime::get_ms();		// Get a time stamp after execution

				if ( i < OwErrand::MAX_RUNNER )
				{
					mElapsedTime[ i ] += ( endTime_ms - startTime_ms );
					mEventCount[ i ]++;
				}
			}
		}
		mutex.give();

		exitTime_ms = OwSystemTime::get_ms();	  // Get a time stamp after execution

		sleepTime_ms = INTERVAL - ( exitTime_ms - entryTime_ms );

		if ( sleepTime_ms > 0 )
		{
#if 0
			if ( sleepTime_ms > INTERVAL )
			{
				sleepTime_ms = INTERVAL;
			}
			OwTask::sleep( sleepTime_ms );
#endif
		}
		else
		{
			mNoSleepCount++;
		}

		OwTask::sleep( INTERVAL );
	}

} // OwErrandRunner::main()


//=============================================================================

void
OwErrandRunner::showTimes
(
	void
)
{
#if 0
	char*	pcName;
	set<OwErrand*>::const_iterator  ip;
	UWORD  i;
	PiTime  elapsedTime[ OwErrand::MAX_RUNNER ];
	unsigned int  eventCount[ OwErrand::MAX_RUNNER ];

	for ( UWORD  i = 0; i< OwErrand::MAX_RUNNER; i++ )
	{
		elapsedTime[ i ] = mElapsedTime[ i ];
		eventCount[ i ] = mEventCount[ i ];
	}

	for ( ip = registry.begin(), i = 0; ip != registry.end(); ip++, i++ )
	{
		pcName = (*ip)->getName();
		unsigned int  interval = (*ip)->getInterval();

		clipPrintf( "%s\t"  "Int %d\t"  "Cnt %u\t"  "Time %lld"NL,
				pcName, interval, eventCount[ i ], elapsedTime[ i ] );
	}

	clipPrintf( "Iter %llu, No sleep  %llu"NL, mIterations, mNoSleepCount );
#endif

} // OwErrandRunner::showTimes()


//=============================================================================

#if 0
int
OwErrandRunner::process_show_times
(
	ArgvType  &argv
)
{
	showTimes();

	return ( OK );

} // OwErrandRunner::process_show_times()
#endif

//=============================================================================
// OwErrandRunner::getInstance

OwErrandRunner*
OwErrandRunner::getInstance
(
	unsigned int  index
)
{
	// If index is out of range, default to zero
	if ( index >= OwErrand::MAX_RUNNER )
	{
		index = 0;
	}

	// If deposit is not initialized, initialize it.
	if ( 0 == instance )
	{
		instance = new OwErrandRunner*[ OwErrand::MAX_RUNNER ];

		for ( unsigned int  i = 0; i < OwErrand::MAX_RUNNER; i++ )
		{
			instance[ i ] = 0;
		}
	}

	// If the instance doesn't exist, instantiate it
	if ( 0 == instance[ index ] )
	{
		instance[ index ] = new OwErrandRunner( index );
	}

	return instance[ index ];

} // OwErrandRunner::getInstance()


//=============================================================================
