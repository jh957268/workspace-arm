//=============================================================================
//
/// @file OwErrand.h
///
/// @brief This file defines classes OwErrand and OwErrandRunner.
/// 	Together, these class allows user to define tasks to be executed
///		periodically.
//
//=============================================================================

#ifndef OW_ERRAND_H
#define OW_ERRAND_H

//=============================================================================

#include  <set>
#include  "OwTask.h"
#include  "OwMutex.h"

using namespace std;

//=============================================================================

#define  MAX_NAME_LENGTH	( 32 )

//=============================================================================
/// OwErrand is the base class of the tasks that executes periodically.
/// It is an abstract class.  Objects derived from this class must define
/// public member function task(), in which the periodic job is defined.

class OwErrand
{
 public:
	/// Maximum number of OwErrandRunner
	static const unsigned int  MAX_RUNNER = 20;

	/// Pure virtual function for the derived class to define their
	/// own periodical task
	virtual void task()=0;

	/// Get the interval between two invocation of task()
	unsigned int	getInterval()
	{
		return  mInterval;
	}

	/// Set the interval between two invocation of task()
	void	setInterval(unsigned int interval)
	{
		mInterval = interval;
	}

	/// Starts periodical call to task()
	virtual void	start()
	{
		started = true;
	}

	/// Stops periodical call to task()
	virtual void	stop()
	{
		started = false;
	}

	/// Check if periodical call is started
	virtual bool	isStarted()
	{
		return  started;
	}

	char*	getName( void )
	{
		return  name;
	};

	//-------------------------------------------------------------------------
 protected:
	/// Constructor
	/// @param[in] interval The time interval between two executions
	/// @param[in] runner Specify which OwErrandRunner to run the
	/// task.  Default to zero if the parameter is not given or is out
	/// of range.
	OwErrand(
		unsigned int  interval,
		const char  *pcName,
		unsigned int  runner = 0 );

	/// Destructor
	~OwErrand();

 private:
	friend class  OwErrandRunner;

	/// This function can only be called by OwErrandRunner.
	/// @return True if the time interval between two executions is up.
	/// @param[in] time The elapse time
	bool	isTimeout( unsigned int  time );

	unsigned int	mInterval;	///< Time interval between two executions
			int		elapse;		///< Records time left before timeout
			bool	started;	///< Periodicall calls has started
	unsigned int	runner;		///< Which runner runs this
			char	name[ MAX_NAME_LENGTH ];
};

//=============================================================================
/// OwErrandRunner provides a registry for the OwErrand and
/// a thread that runs the periodical tasks.

class OwErrandRunner :
	public OwTask
{
 public:
	/// Get the specified instance of the object.
	/// @param[in] index Specify which instance of the object
	static OwErrandRunner*	getInstance( unsigned int  index );

	/// For OwErrand object to register itself
	void	registerTask( OwErrand* );

	/// For OwErrand object to unregister itself
	void	unregisterTask( OwErrand* );

	void	showTimes( void );

 private:
	/// Private constructor
	/// @param[i] id To identify the instance
			OwErrandRunner( unsigned int  id );

	// int		process_show_times( ArgvType  &argv );

	/// Main program of OwTask subclasses.
	void main(OwTask*);

	static const unsigned int	INTERVAL = 100;	///< Run every 100ms
	set<OwErrand*> registry;					///< To register OwErrand
	static OwErrandRunner**		instance;		///< Array holding instances
					OwMutex		mutex;			///< Protect critical region
			unsigned int		id;				///< Used to ID the instance
			unsigned long long	mNoSleepCount;
			unsigned long long	mIterations;

						PiTime	mElapsedTime[ OwErrand::MAX_RUNNER ];
				unsigned int	mEventCount[ OwErrand::MAX_RUNNER ];
};

//=============================================================================
// Inline functions

//------------------------------------------------------------
// OwErrand::isTimeout
//
inline bool
OwErrand::isTimeout(unsigned int time)
{
	if((elapse-=time)<=0)
	{
		elapse=mInterval;
		return true;
	}
	return false;
}

//------------------------------------------------------------
// OwErrandRunner::registerTask

inline void
OwErrandRunner::registerTask(OwErrand* r)
{
	mutex.take(PI_FOREVER);
	registry.insert(r);
	mutex.give();
}

//------------------------------------------------------------
// OwErrandRunner::unregisterTask

inline void
OwErrandRunner::unregisterTask(OwErrand* r)
{
	mutex.take(PI_FOREVER);
	registry.erase(r);
	mutex.give();
}


//=============================================================================

#endif

//=============================================================================
