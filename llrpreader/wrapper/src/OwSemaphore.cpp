//=============================================================================
//
/// @file OwSemaphore.cpp
///
/// @brief This file contains the implementation for OwSemaphore class
//
//=============================================================================

#include  <time.h>
#include  <errno.h>
#include  <stdio.h>

#include  "OwSemaphore.h"
#include  "SPrintf.h"

//=============================================================================

#define NOT_SHARED 0 // Does not support of sharing semaphores among processes

//=============================================================================
/// OwSempahore is implemented using pthread provided semaphore. This semaphore
/// is local to the process. It can be accessed by threads created by the parent
/// process. LinuxThreads currently does not support  process-shared  semaphores

// Constructor initializes semaphore variable

OwSemaphore::OwSemaphore(unsigned int value)
{
    // initialize the semaphore
    if ( 0 != sem_init( &m_semaphore, NOT_SHARED, value ) )
    {
    	// Shouldn't be here
        printf( "Error: %s: Failed to initialize semaphore"NL, "OwSemaphore::OwSemaphore" );
    }
}


//=============================================================================
/// Destructor destroys the semaphore variable

OwSemaphore::~OwSemaphore()
{
    sem_destroy( &m_semaphore );
}


//=============================================================================
/// This function attempts to decrement one from the counting semaphore.
/// if the counter is greater than zero, it is decremented successfully
/// and the function returns. If the counter is zero, the calling task
/// blocks until either the count is greater than zero, or the timeout
/// occurs

PiStatus
OwSemaphore::take
(
	PiTime  timeout
)
{
    if ( PI_FOREVER == timeout )
    {
        // Wait till the semaphore is obtained
        sem_wait( &m_semaphore );
    }
    else
    {
        struct timespec tout = { 0, 0 };
        int secs             = (int)( timeout / 1000 );
        int msecs            = (int)( timeout % 1000 );
        int retcode;

        tout.tv_sec  = (int)( secs + time( NULL ) );
        tout.tv_nsec = msecs * 1000;

        // Wait till the semaphore is obtained or timeout expires
        retcode = sem_timedwait( &m_semaphore, &tout );

        if ( ( 0 != retcode ) && ( ETIMEDOUT == errno ) )
        	return TIMEOUT;
        else if ( 0 != retcode )
        	return ERROR;
    }

    return OK;
}


//=============================================================================
/// This function increments the counting semaphore by one. If a task
/// is waiting for the semaphore, the task is resumed and semaphore
/// is decremented.

PiStatus
OwSemaphore::give()
{
    if ( 0 != sem_post( &m_semaphore ) )
        return ERROR;

    return OK;
}


//=============================================================================
/// This function returns the current value of the counter

unsigned int
OwSemaphore::getCount() const
{
    int val = 0;

    // sem_getvalue always returns zero. no need of error checking
    sem_getvalue( (sem_t*)&m_semaphore, &val );

    return val;
}


//=============================================================================


