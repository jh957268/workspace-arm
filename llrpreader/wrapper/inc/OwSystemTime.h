//=============================================================================
//
/// @file  OwSystemTime.h
/// @brief  This file contains implementation for OwSystemTime.
//
//=============================================================================

#ifndef OWSYSTEMTIME_H
#define OWSYSTEMTIME_H

//=============================================================================

#include "PiTypes.h"

//=============================================================================

#define SECS_TO_MILLISECS(a) (a * 1000) /**< converts secs to millisecs */

//=============================================================================
//
/// @class  OwSystemTime
/// @brief  OwSystemTime keeps system timestamp.

class OwSystemTime
{
	public:

		/**
		 * This function sets the system time.
		 *
		 * @param  timestamp_ms
		 */
		static void		set( PiTime  timestamp_ms );

		/**
		 * This function sets the system time.
		 *
		 * @param  timestamp_sec
		 */
		static void		set_seconds( PiTime  timestamp_sec );

		/**
		 * This function gets the system time
		 *
		 * @return  The current timestamp in milliseconds
		 */
		static PiTime	get_ms();

		/**
		 * This function gets the system time
		 *
		 * @return  The current timestamp in seconds
		 */
		static PiTime	get_seconds();

	protected:

	private:

		/**
		 * Not supported constructors and operators. Kept private
		 * to avoid default behaviour
		 */
		OwSystemTime(const OwSystemTime&);
		OwSystemTime& operator=(const OwSystemTime&);

};

//=============================================================================

#endif /* ifndef OWSYSTEMTIME_H */

//=============================================================================
