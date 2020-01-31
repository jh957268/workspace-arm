//=============================================================================
//
// @file OwMutex.cpp
//
// @brief This file contains the implementation for OwMutex class
//
//=============================================================================

#include <time.h>
#include <errno.h>

#include "OwMutex.h"

//=============================================================================
//
// OwMutex is implemented using pthread provided mutex. "Error Checking"
// mutex type has been chosen to implement the following,
// 1. avoid deadlock. if a thread attempts to lock  a mutex that it already
//    owns, mutex returns EDEADLK.
// 2. Only the thread that owns the Mutex can release the Mutex.
//
// Fast and recursive mutexes perform no checks, thus allowing a locked
// mutex to be unlocked by a thread  other than  its  owner
//
//=============================================================================
//
// Constructor creates mutex of kind PTHREAD_MUTEX_ERRORCHECK_NP

OwMutex::OwMutex
(
)
{
    // Initialize the pthread mutex for error checking kind
	pthread_mutexattr_init( &m_mutexAttr );
    pthread_mutexattr_settype( &m_mutexAttr, PTHREAD_MUTEX_ERRORCHECK_NP );
    pthread_mutex_init( &m_mutex, &m_mutexAttr );
}

//=============================================================================
//
// Destructor destroys the mutex and its attribute

OwMutex::~OwMutex
(
)
{
    pthread_mutex_destroy( &m_mutex );
    pthread_mutexattr_destroy( &m_mutexAttr );
}

//=============================================================================
//
// This function attempts to take the Mutex. If the Mutex
// has been taken by another task, the calling task blocks until either
// the Mutex is released, or the timeout occurs. If the calling
// task already owns the Mutex, the function returns successfully.

PiStatus
OwMutex::take
(
	PiTime  timeout
)
{
    if ( timeout == PI_FOREVER )
    {
        // suspend till the Mutex is obtained
        if ( 0 != pthread_mutex_lock( &m_mutex ) )
            return ERROR;
    }
    else
    {
        struct timespec  tout = { 0, 0 };
        int  retcode          = 0;
        int  secs             = (int)( timeout / 1000 );
        int  msecs            = (int)( timeout % 1000 );

        // set timeout
        tout.tv_sec  = (int)( time( NULL ) + secs );
        tout.tv_nsec = msecs * 1000; // nanoseconds

        // suspend till the Mutex is obtained or timeout expires
        retcode = pthread_mutex_timedlock( &m_mutex, &tout );

        if ( ETIMEDOUT == retcode )
        {
            return TIMEOUT;
        }
        else if ( ( EDEADLK != retcode ) && ( 0 != retcode ) )
        {
            // Deadlock is not return as error
            return ERROR;
        }
    }

    return OK;
}

//=============================================================================
//
// This function attempts to give back a Mutex. A Mutex can be
// released if it is given back exactly the same number of time it is
// taken by the same task. If the Mutex is released successfully,
// it is given to one of the task (with highest priority) that is
// blocked by it.
//
// NOTE: return PiStatus::NOT_READY (if task has taken the mutex more times
// than given it back) not implemented since it is not supported by
// 'Error checking' mutex type

PiStatus
OwMutex::give
(
)
{
    // unlock the mutex
    if ( 0 != pthread_mutex_unlock( &m_mutex ) )
    {
        return  ERROR;
    }

    return  OK;
}

//=============================================================================
