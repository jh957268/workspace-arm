//=============================================================================
//
/// @file  OwTimer.h
/// @brief  This file contains the implementations for OwTimer class.
//
//=============================================================================

#ifndef OWTIMER_H
#define OWTIMER_H

#include  <pthread.h>
#include  <semaphore.h>

#include  "PiTypes.h"

//=============================================================================
//
// @class OwTimer
//
// @brief OwTimer represents a timer, which calls a handler function when
// expires
//

class OwTimer
{
	public:

		/**
		 * @brief This is the base class of timer handler. Objects that handles
		 * timer shall derive from OwTimer::Handler and define handleTimeout
		 * function. When timer times out, handleTimeout is called.
		 *
		 * @param timer	 The OwTimer object that causes timeout
		 */
		struct Handler
		{
			virtual ~Handler(){}
			virtual void handleTimeout( OwTimer*  timer ) = 0;
		};

		/**
		 * @brief constructs an OwTimer object, which calls member function
		 * handleTimeout of the handler object when timer expires.
		 *
		 * @param handler   The handler object
		 */
		OwTimer( Handler*  handler,
					const char*  name );

		/**
		 * @brief destructor
		 */
		~OwTimer();

		/**
		 * @brief This function starts a timer. An OwTimer can not be started
		 * twice
		 *
		 * @param timeout   Timeout period in milliseconds
		 *
		 * @return
		 *	  PiStatus::OK if timer started correctly
		 *	  PiStatus::NOT_READY if the timer has been started and not yet
		 *	  expired
		 */
		PiStatus start( PiTime  timeout );

		/**
		 * @brief This function cancels a timer
		 */
		void cancel();

		/**
		 * @brief This function returns true if the timer is still running.
		 *
		 * @return
		 *	  TRUE  if the timer is running
		 *	  FALSE if the timer is not running
		 */
		bool hasStarted() { return mbIsStarted; }

	protected:

	private:

		/**
		 * Not supported constructors and operators. Kept private
		 * to avoid default behaviour
		 */
		OwTimer( const OwTimer& );
		OwTimer& operator=( const OwTimer& );

		/**
		 * Private member functions
		 */

		/*
		 * @brief Thread entry function
		 */
		static void* threadStart( void*  arg );

		// Private member variables

		Handler*		mpHandler;		/**< handler object */
		pthread_attr_t	mpThreadAttr;  /**< pthread attributes */
		pthread_t		mpThreadID;	/**< pthread ID */
		pthread_mutex_t	mMutex;		/**< mutex for condition wait */
		pthread_cond_t	mTimerCond;	/**< condition variable to implement
											 suspend and resume operation */
				bool	mbIsStarted;		/**< task suspend status */
				PiTime	mTimeout_ms;		/**< timeout period in millisecs */
				sem_t	mSemaphore;		/**< semaphore to start thread */
				bool	mbThreadIsCreated;	/**< command to exit thread
											 0 = NOT RUNNING/STOP
											 1 = RUNNING */

				char	mcTimerName[ 32 ];

};

//=============================================================================

#endif /* ifndef OWTIMER_H */

//=============================================================================
