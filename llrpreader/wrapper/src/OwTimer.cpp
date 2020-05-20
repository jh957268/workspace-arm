//=============================================================================
//
/// @file  OwTimer.cpp
/// @brief  This file contains the implementations for OwTimer class.
//
//=============================================================================

#include  "OwTimer.h"
#include  <time.h>
#include  <errno.h>
#include  <stdio.h>
#include  <iostream>
#include  <string.h>
#include  "OwSystemTime.h"
#include  <unistd.h>
#include  "SPrintf.h"

//=============================================================================

using namespace std;

#define NOT_SHARED 0 // Does not support of sharing semaphores among processes

//=============================================================================
//
/// OwTimer is implemented using pthread and pthread's timedcond_wait feature.
/// On timer startup, a thread will be spawned which will wait for timeout
/// or cancel signal from the parent thread.
/// On timeout, the child thread would execute timeout callback function.
//
//=============================================================================
/// Constructor initializes necessary thread and condwait resources

OwTimer::OwTimer
(
	OwTimer::Handler*  handler,
	const char*  pcName
)
	: mpHandler( handler )
	, mbIsStarted( false )
	, mTimeout_ms( 0 )
	, mbThreadIsCreated( false )
{
	printf( "Timer %s created"NL, pcName );
	strncpy( mcTimerName, pcName, 32 );
	// initialize the pthread attribute
	pthread_attr_init( &mpThreadAttr );

	// pthread_attr_setstacksize( &mpThreadAttr, 32768 );

	// Set schedule policy
	// if SCHED_RR is not supported, default SCHED_OTHER will be used
	// pthread_attr_setschedpolicy( &mpThreadAttr, SCHED_RR );

	pthread_attr_setinheritsched( &mpThreadAttr, PTHREAD_EXPLICIT_SCHED );

#if 0
	// Set detach state. the thread resources are immediately freed when it
	// terminates
	//pthread_attr_setdetachstate( &m_pthreadAttr, PTHREAD_CREATE_DETACHED );
	pthread_attr_setdetachstate( &mpThreadAttr, PTHREAD_CREATE_DETACHED );

	// Set priority
	// priority does matter only when the thread schedule policy is
	// either SCHED_RR or SCHED_FIFO
	pthread_attr_getschedparam( &mpThreadAttr, &m_schedParam );
	m_schedParam.sched_priority = 50 + 3;
	pthread_attr_setschedparam( &mpThreadAttr, &m_schedParam );
	pthread_attr_setscope( &mpThreadAttr, PTHREAD_SCOPE_SYSTEM );
#endif

	// Mutex used for condition wait
	pthread_mutex_init( &mMutex, NULL ); // 'fast' mutex
	pthread_cond_init( &mTimerCond, NULL );

	// initialize the semaphore
	sem_init( &mSemaphore, NOT_SHARED, 0 );

} // OwTimer::OwTimer()


//=============================================================================
/// Destructor

OwTimer::~OwTimer()
{
	// signal the timer thread
	if ( mbThreadIsCreated )
	{
		// cancel if the timer is running
		cancel();

		// stop the timer thread
		mbThreadIsCreated = false;
		sem_post(&mSemaphore);

		// wait the thread to exit
		pthread_join( mpThreadID, NULL );
	}

	/*
	 * Destroy all the pthread resources
	 */
	pthread_attr_destroy( &mpThreadAttr );
	pthread_cond_destroy( &mTimerCond );
	pthread_mutex_destroy( &mMutex );
	sem_destroy( &mSemaphore );

} // OwTimer::~OwTimer()


//=============================================================================
/// Thread entry function. Invokes thread specific main() implementation
/// @param  void*  arg - pointer to OwTimer block

void*
OwTimer::threadStart
(
	void*  arg
)
{
	OwTimer*  timerPtr = (OwTimer*)arg;
	PiTime  currTime_ms;
	struct timespec  tout = { 0, 0 };
	int  retcode = 0;
	bool  shutdown = false;

	struct sched_param SchedParam;

	printf( "Timer started, PID = %d, TID = %lu" NL, getpid(), pthread_self() );

	// printf("Timer started---\n");

	SchedParam.sched_priority = sched_get_priority_max(SCHED_RR);
	// SchedParam.sched_priority = 0;
	// printf("After\n");

	if ( sched_setscheduler( getpid(), SCHED_RR, &SchedParam) == -1 )
	{
		printf( "OwTimer::threadStart set priority failed" NL );
	}
	//else
	//{
	//	printf( "OwTimer::threadStart set priority successfully" NL );
	//}

	// printf("After---\n");


	while ( ! shutdown )
	{
		// Wait for the start signal from OwTimer object
		sem_wait( &(timerPtr->mSemaphore) );
		retcode = 0;

		// on OwTimer object destruction, exit the thread
		if ( (timerPtr->mbThreadIsCreated) && (timerPtr->mbIsStarted == true) )
		{
			pthread_mutex_lock( &(timerPtr->mMutex) );
			currTime_ms = OwSystemTime::get_ms();
			currTime_ms += timerPtr->mTimeout_ms;
			int secs	= (int)(currTime_ms / 1000);
			int msecs   = (int)(currTime_ms % 1000);

			// set timeout
			tout.tv_sec  = secs;
			tout.tv_nsec = msecs * 1000000; // nanoseconds

			time_t T= time(NULL);
			struct  tm tm = *localtime(&T);
			printf("%02d:%02d:%02d Timer Wait timeout!\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
			// need to wait till timeout expires or cancel from parent thread
			retcode = pthread_cond_timedwait( &(timerPtr->mTimerCond),
					 &(timerPtr->mMutex), &tout );

			if ( retcode == ETIMEDOUT )
			{
				timerPtr->mbIsStarted = false;
				timerPtr->running = true;
				
				// invoke the timer callback function
				T= time(NULL);
				tm = *localtime(&T);
				printf("%02d:%02d:%02d Timer expired, call handler!\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
				timerPtr->mpHandler->handleTimeout( timerPtr );
			}
			else
			{
				//printf("Timer Canceled!\n");
				T= time(NULL);
				tm = *localtime(&T);
				printf("%02d:%02d:%02d Timer canceled!\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
			}

			pthread_mutex_unlock( &(timerPtr->mMutex) );
		}
		else
		{
			shutdown = false;
		}
	}

	cout << "OwTimer thread shutdown" << endl;

	pthread_exit( 0 );

} // OwTimer::threadStart()


//=============================================================================
/// This function starts a timer. An OwTimer can not be started twice

PiStatus
OwTimer::start
(
	PiTime  timeout_ms
)
{
	PiStatus  retCode = NOT_READY;

	// create the thread only once
	if ( ! mbThreadIsCreated )
	{
		// create the pthread
		int  ret = pthread_create( &mpThreadID, &mpThreadAttr, &threadStart,
				(void*)this );

		if ( 0 == ret )
		{
			mbThreadIsCreated = true;
		}
		else
		{
			cerr << "pthread creation failed. ret code : " << ret << endl;
		}
	}

	if ( mbThreadIsCreated && ! mbIsStarted )
	{
		time_t T= time(NULL);
		struct  tm tm = *localtime(&T);
		mbIsStarted = true;
		suspended = false;
		running = false;
		printf( "%02d:%02d:%02d Timer %s started"NL, tm.tm_hour, tm.tm_min, tm.tm_sec, mcTimerName );
		mTimeout_ms = timeout_ms;

		// signal the thread
		sem_post( &mSemaphore );
		retCode = OK;
	}

	return  retCode;

} // OwTimer::start()


//=============================================================================
/// This function cancels a timer

void
OwTimer::cancel()
{
	// If the timer is running, signal to cancel the timer
	if ( mbIsStarted )
	{
		time_t T= time(NULL);
		struct  tm tm = *localtime(&T);
		
		printf( "%02d:%02d:%02d Timer %s canceled"NL, tm.tm_hour, tm.tm_min, tm.tm_sec, mcTimerName );
		pthread_mutex_lock( &mMutex );
		mbIsStarted = false;
		pthread_cond_signal( &mTimerCond );
		//printf( "Timer %s did canceled"NL, mcTimerName );
		pthread_mutex_unlock( &mMutex );
	}

} // OwTimer::cancel()


//=============================================================================

