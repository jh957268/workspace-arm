//=============================================================================
//
/// @file  CWatchDogFeeder.h
/// @brief  Declarations for Watch Dog Feeder class
//
//=============================================================================

#ifndef CWATCHDOGFEEDER_H_
#define CWATCHDOGFEEDER_H_

//=============================================================================

#include  "OwTask.h"
//#include  "Command.h"
//#include  "PiTypes.h"

//=============================================================================

class CWatchDogFeeder :
	public OwTask
{
	public:
				CWatchDogFeeder();
	virtual		~CWatchDogFeeder();

	//-----------------------------------------------------
	private:
		int		mWatchDogFD;
		bool	mbDebug;
		int		interval;
		//PiTime	mFeedTime;

	void	main( OwTask *task );
};

//=============================================================================

#endif

//=============================================================================
