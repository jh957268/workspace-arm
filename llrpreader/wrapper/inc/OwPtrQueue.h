//=============================================================================
/**
 * @file OwPtrQueue.h
 *
 * @brief This file contains class declaration and implementation for OwPtrQueue
 *
 */
//=============================================================================

#ifndef OWPTRQUEUE_H
#define OWPTRQUEUE_H

//=============================================================================

#include "OwQueue.h"

//=============================================================================

#define TO_PTR_QUEUE(a) ((OwPtrQueue*)a)

//=============================================================================
/**
 * @class OwPtrQueue
 *
 * @brief This class provides wrapper for sending pointer across
 * OwQueue
 *
 */
class OwPtrQueue : private OwQueue
{
    public:
        /**
         * @brief constructor
         *
         * @param sz size of the queue in bytes
         */
        OwPtrQueue(unsigned int sz)
            : OwQueue(sz)
            {
            }

        /**
         * @brief destructor for OwPtrQueue
         */
        ~OwPtrQueue() { }

        /**
         * @brief Write pointer address into the queue
         *
         * @param ptr       pointer to be sent
         * @param timeout   Timeout period in milliseconds
         *
         * @return
         *      PiStatus::OK if the data is written successfully
         *      PiStatus::TIMEOUT if the timeout occurs
         */
        inline PiStatus send(void* ptr, PiTime timeout);

        /**
         * @brief Read pointer address from the queue
         *
         * @param ptr       pointer read from the queue
         * @param timeout   Timeout period in milliseconds
         *
         * @return
         *      PiStatus::OK if the data is read successfully
         *      PiStatus::TIMEOUT if timeout occurs
         */
        inline PiStatus receive(void** ptr, PiTime timeout);

        OwQueue::flush; // base class function to external user

    protected:

    private:

        /**
         * Not supported constructors and operators. Kept private
         * to avoid default behaviour
         */
        OwPtrQueue(const OwPtrQueue&);
        OwPtrQueue& operator=(const OwPtrQueue&);

};

///////////////////////////////////////////////////////////////////////////
// inline functions
///////////////////////////////////////////////////////////////////////////

/**
 * @brief Write pointer address into the queue
 */
inline PiStatus
OwPtrQueue::send(void* ptr, PiTime timeout)
{
    unsigned int tmp = (unsigned int)ptr;
    return write((const char*)&tmp , sizeof(ptr), timeout);
}

/**
 * @brief Read pointer address from the queue
 */
inline PiStatus
OwPtrQueue::receive(void** ptr, PiTime timeout)
{
    PiStatus retVal;
    unsigned int tmp = 0;

    retVal = read((char*)&tmp, sizeof(*ptr), timeout);
    *ptr = (void*)tmp;

    return retVal;
}

//=============================================================================

#endif /* ifndef OWPTRQUEUE_H */

//=============================================================================
