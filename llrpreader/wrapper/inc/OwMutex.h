//=============================================================================
/**
 * @file OwMutex.h
 *
 * @brief This file contains the declaration for OwMutex class.
 *
 */
//=============================================================================

#ifndef OWMUTEX_H
#define OWMUTEX_H

//=============================================================================

#include <pthread.h>

#include "PiTypes.h"

//=============================================================================
/**
 * @class OwMutex
 *
 * @brief OwMutex represents a mutual exclusive semphore.
 * For simplicity, OwMutex, once constructed, shall not be destructed.
 * This prevents the destructor from having to release all the pending tasks.
 *
 */
class OwMutex
{
    public:
        /**
         * constructor & destructor
         */
        OwMutex();
        ~OwMutex();

        /**
         * This function attempts to take the Mutex. If the Mutex
         * has been taken by another task, the calling task blocks until either
         * the Mutex is released, or the timeout occurs. If the calling
         * task already owns the Mutex, the function returns successfully.
         *
         * @param timeout Tmieout period in milliseconds
         *
         * @return
         *      PiStatus::OK if the Mutex is obtained successfully.
         *      PiStatus::TIMEOUT if timeout occurs
         */
        PiStatus take(PiTime timeout);

        /**
         * This function attempts to give back a Mutex. A Mutex can be
         * released if it is given back exactly the same number of time it is
         * taken by the same task. If the Mutex is released successfully,
         * it is given to one of the task (with highest priority) that is
         * blocked by it.
         *
         * @return
         *      PiStatus::OK if the Mutex is released successfully.
         *      PiStatus::NOT_READY if task has taken the Mutex more
         *      times than given it back.
         */
        PiStatus give();

    protected:

    private:

        /**
         * Not supported constructors and operators. Kept private
         * to avoid default behaviour
         */
        OwMutex(const OwMutex&);
        OwMutex& operator=(const OwMutex&);

        /**
         * Private member variables
         */
        pthread_mutexattr_t m_mutexAttr;    /**< mutex attribute */
        pthread_mutex_t     m_mutex;        /**< pthread mutex */
};

//=============================================================================

#endif /* ifndef OWMUTEX_H */

//=============================================================================
