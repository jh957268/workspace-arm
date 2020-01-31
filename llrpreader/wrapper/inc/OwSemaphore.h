//=============================================================================
//
// @file OwSemaphore.h
//
// @brief This file contains declaration for OwSemaphore class.
//
//
//=============================================================================

#ifndef OWSEMAPHORE_H
#define OWSEMAPHORE_H

//=============================================================================
#include  <pthread.h>
#include  <semaphore.h>

#include  "PiTypes.h"

//=============================================================================
//
// @class OwSemaphore
//
// @brief OwSemaphore represents a counting semaphore.
// For simplicity, OwMutex, once constructed, shall not be destructed.
// This prevents the destructor from having to release all the pending tasks.

class OwSemaphore
{
	public:
		//-------------------------------------------------
		// Constructor of the OwSemaphore object
		// @param init initial value of the counter

		OwSemaphore( unsigned int  init );

		//-------------------------------------------------
		// Destructor

		~OwSemaphore();

		//-------------------------------------------------
		//
		// This function attempts to decrement one from the counting semaphore.
		// if the counter is greater than zero, it is decremented successfully
		// and the function returns. If the counter is zero, the calling task
		// blocks until either the count is greater than zero, or the timeout
		// occurs
		//
		// @param timeout Timeout period in milliseconds
		//
		// @return
		//	  PiStatus::OK if the semaphore is decremented successfully
		//	  PiStatus::TIMEOUT if the timeout occurs.

			PiStatus	take( PiTime  timeout );

		//-------------------------------------------------
		//
		// This function increments the counting semaphore by one. If a task
		// is waiting for the semaphore, the task is resumed and semaphore
		// is decremented.
		//
		// @return PiStatus::OK

			PiStatus	give();

		//-------------------------------------------------
		//
		// This function returns the current value of the counter
		//
		// @return value of the counter

		unsigned int	getCount() const;

	protected:

	private:
		// Not supported constructors and operators. Kept private
		// to avoid default behaviour

		OwSemaphore(const OwSemaphore&);
		OwSemaphore& operator=(const OwSemaphore&);

		// Private member variables

		sem_t	m_semaphore;	/**< semaphore variables */

};

//=============================================================================

#endif

//=============================================================================
