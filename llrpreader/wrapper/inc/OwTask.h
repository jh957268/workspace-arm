//=============================================================================
/**
 * @file OwTask.h
 *
 * @brief This file contains the declaration for the OwTask class.
 *
 */
//=============================================================================

#ifndef OWTASK_H
#define OWTASK_H

//=============================================================================

#include <pthread.h>

#include "PiTypes.h"
#include "SysTypes.h"

//=============================================================================

#define MILLISECS_TO_MICROSECS(val) (val * 1000) /**< converts millisecsonds
                                                   to microseconds */

//=============================================================================
/**
 * @class OwTask
 *
 * @brief OwTask is an abstract class. Active classes shall derive from
 * osTask and define the pure virtual function main. When function main
 * returns, the task terminates.
 * To maintain simplicity, instances of OwTask shall not be destructed
 * (to prevent the destruction of data members that are still referenced
 * by codes running under a different thread). Destruction of an OwTask
 * object does not stop the spawned task from executing.
 *
 */
#define MAX_THREAD_NAME_LEN		20
class OwTask
{
    public:

        /**
         * An enum priority represents the task priority.
         */
		enum Priority
        {
            LOW         = 1,
            MEDIUM_LOW  = 2,
            MEDIUM      = 3,
            MEDIUM_HIGH = 4,
            HIGH        = 5
        };

        /**
         * Creates an OwTask object
         *
         * @param priority Priority of the task
         * @param stackSize Number of bytes reserved for the stack
         * @param optional name of the task
         *
         * @return N/A
         */
        OwTask(OwTask::Priority priority, unsigned int stackSize,
        		const char* name=0, int on_exit=false);

        /**
         * Destructor
         * Destruction of OwTask does not stop spawned child thread.
         */
        virtual ~OwTask() {pthread_attr_destroy( &m_pthreadAttr );} // body moved here or the linker will have
        															// undefined reference to typeinfo error message

        /**
         * This is a pure virtual function that shall be refined in the
         * derived class of OwTask. The derived class shall define the
         * "main program" for the task here. When OwTask::run is called,
         * this function starts in a child task.
         *
         * @param task Pointer to the OwTask object whose member function
         * run is called

         * @return none
         */
        virtual void main(OwTask* task)=0;

        /**
         * This function is called by parent task to run OwTask::main in a
         * child task
         */
        void run();

        /**
         * These functions return the process/thread ID and actual priority given
         * by the underlying OS, respectively.  They are for debugging purpose.
         */
        unsigned int getId();
        unsigned int getPriority();
        void 		 taskYield(){pthread_yield();}

        /**
         * This function causes the calling thread to suspend for specified
         * milliseconds
         *
         * @param milliseconds sleep period in milliseconds
         *
         * @return none
         */
        static void sleep(PiTime milliseconds);

    protected:

    private:

        /**
         * Not supported constructors and operators. Kept private
         * to avoid default behaviour
         */
        OwTask(const OwTask&);
        OwTask& operator=(const OwTask&);

        /**
         * private member functions
         */
        static void* threadStart(void* arg);

        /**
         * Private member variables
         */
        pthread_attr_t      m_pthreadAttr;  /**< pthread attributes */
        pthread_t           m_pthreadID;    /**< pthread ID */
        struct sched_param  m_schedParam;
        char			    name[MAX_THREAD_NAME_LEN];		/**< name of the task */
        int 				destroy_on_exit;
};

//=============================================================================

#endif /* ifndef OWTASK_H */

//=============================================================================
