//=============================================================================
//
/// @file  OwTask.cpp
///
/// @brief  This file contains the implementation for OwTask class
//
//=============================================================================

#include  <stdio.h>
#include  <signal.h>
#include  <string.h>
#include  <errno.h>
#include  <unistd.h>

#include  "OwTask.h"
#include  "SPrintf.h"

//=============================================================================

char*  newCharArray( const char  *pcName, size_t  size );

using namespace  std;

//=============================================================================
//
/// OwTask is an abstract class implemented using POSIX thread. The derived
/// task class need to implement task entry function main(), which is pure
/// virtual in OwTask.
///
/// 1. Schedule policy: Default SCHED_OTHER
/// 2. Thread priority: is applicable only if schedpolicy either SCHED_RR or
///    SCHED_FIFO. For SCHED_OTHER, priority is ignored.
///    sched-policy need to be changed (SCHED_RR/SCHED_FIFO) for using thread
/// priority.
/// 3. Stacksize: In pthread, the stacksize attribute shall define the minimum
///    stack size (in bytes) allocated for the created threads stack.
///    The value of stacksize shoud be greater than PTHREAD_STACK_MIN (16K)
///    Maximum allowed stack size for pthread can not be controlled.
///
/// NOTE:
/// 1. On destroying OwTask, it does not stop thread execution
//

//=============================================================================
/// Constructor initializes pthread attributes and other resources

OwTask::OwTask
(
	OwTask::Priority  priority,
	unsigned int  stackSize,
	const char*  name,
	int on_exit

)
{
	const char  *pcName = "OwTask::OwTask";
	// Copy the name of the thread
	if ( name != 0 )
	{
		strcpy( this->name, name );
	}
	destroy_on_exit = on_exit;
	// initialize the pthread attribute
	pthread_attr_init( &m_pthreadAttr );

	// If priority is LOW, don't change scheduling class. Default
	// is SCHED_OTHER, which seems to be lower than any SCHED_RR priority.
	if ( LOW != priority )
	{
		// Set minimum stack size
		pthread_attr_setstacksize( &m_pthreadAttr, 32768 );

		// Set schedule policy
		// if SCHED_RR is not supported, default SCHED_OTHER will be used
		pthread_attr_setschedpolicy( &m_pthreadAttr, SCHED_RR );

		// Set inherit property as explicit initialization from schedparam and
		// schedpolicy
		pthread_attr_setinheritsched( &m_pthreadAttr, PTHREAD_EXPLICIT_SCHED );

		// Set detach state. the thread resources are immediately freed when it
		// terminates
		//pthread_attr_setdetachstate( &m_pthreadAttr, PTHREAD_CREATE_DETACHED );
		pthread_attr_setdetachstate( &m_pthreadAttr, PTHREAD_CREATE_DETACHED );

		// Set priority
		// priority does matter only when the thread schedule policy is
		// either SCHED_RR or SCHED_FIFO
		pthread_attr_getschedparam( &m_pthreadAttr, &m_schedParam );
		m_schedParam.sched_priority = 50 + priority;
		pthread_attr_setschedparam( &m_pthreadAttr, &m_schedParam );
		pthread_attr_setscope( &m_pthreadAttr, PTHREAD_SCOPE_SYSTEM );
	}
}


//=============================================================================
/// Destructor destroys all pthread resources
/// Note: Destruction of OwTask does not stop spawned child thread.

#if 0
OwTask::~OwTask()
{
	/*
	 * Destroy all the pthread resources
	 */
	pthread_attr_destroy( &m_pthreadAttr );
}
#endif

//=============================================================================
/// Thread entry function. Invokes thread specific main() implementation
///
/// @param arg pointer to OwTask block

void*
OwTask::threadStart
(
	void*  arg
)
{
	OwTask*  taskPtr = (OwTask*) arg;

    printf( "%s started, PID=%d, TID=%d"NL,
    		(taskPtr->name? taskPtr->name : "Anonymous task "),
    		getpid(), taskPtr->getId() );

	taskPtr->main( taskPtr );

	if (taskPtr->destroy_on_exit == true)
	{
		printf("Delete allocated task object" NL);   // for testing only if the task object need to be deleted
		delete taskPtr;
	}

	pthread_exit( NULL );

	return  0; // To suppress compiler warning
}


//=============================================================================
/// This function is called by parent task to run OwTask::main in a
/// child task

void OwTask::run
(
)
{
	// create the pthread
	int  ret = pthread_create( &m_pthreadID, &m_pthreadAttr, threadStart,
			(void*) this );

	if ( ret != 0 )
	{
		// cerr << "pthread creation failed. ret code : " << ret << endl;
		// cout << strerror(errno) << endl;
		printf( "pthread creation failed in function OwTask::run."NL );
		printf( "Error code from pthread_create %d."NL, ret);
	}
}


//=============================================================================
/// This function causes the calling thread to suspend for specified
/// milliseconds

void
OwTask::sleep
(
	PiTime  milliseconds
)
{
	usleep( MILLISECS_TO_MICROSECS( milliseconds ) ); // suspends thread
}


//=============================================================================
/// Get process/thread ID

unsigned int
OwTask::getId
(
)
{
	return pthread_self();
}


//=============================================================================
/// Get scheduled priority

unsigned int
OwTask::getPriority
(
)
{
	int  my_schedpolicy;
	sched_param  my_schedparam;
	pthread_getschedparam( this->getId(), &my_schedpolicy, &my_schedparam );

	return  my_schedparam.sched_priority;
}


//=============================================================================
