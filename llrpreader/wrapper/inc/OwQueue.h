//=============================================================================
/**
 * @file OwQueue.h
 *
 * @brief This file contains the declaration for OwQueue class
 *
 */
//=============================================================================

#ifndef OWQUEUE_H
#define OWQUEUE_H

//=============================================================================

#include <pthread.h>

#include "PiTypes.h"

//=============================================================================
/**
 * @class OwQueue
 *
 * @brief OwQueue is a queue to exchange data among tasks.
 *
 */
class OwQueue
{
    public:

        /**
         * Constructor of the OwQueue object
         *
         * @param size Number of bytes in the queue
         */
        OwQueue(unsigned int size);

        /**
         * destructor destroys al the queue resources
         */
        virtual ~OwQueue();

        /**
         * This function writes the contents of the buffer to the queue.
         * If the empty space in the queue is not enough to hold the data,
         * the calling task blocks. It continues blocking until either the
         * queue has enough space or the function timesout.
         *
         * @param buffer    Buffer containing data
         * @param size      Number of bytes in the data
         * @param timeout   Timeout period in milliseconds
         *
         * @return
         *      PiStatus::OK if the data is written successfully
         *      PiStatus::TIMEOUT if the timeout occurs
         */
        PiStatus write(const char* buffer, unsigned int size, PiTime timeout);

        /**
         * This function reads number of bytes from the queue. If the contents
         * in the queue have fewer bytes than the buffer, the calling task
         * blocks. It continues to block until queue contains enough bytes, or
         * the function timesout.
         *
         * @param buffer    Buffer to hold data
         * @param size      Number of bytes in the buffer
         * @param timeout   Timeout period in milliseconds
         *
         * @return
         *      PiStatus::OK if the data is read successfully
         *      PiStatus::TIMEOUT if timeout occurs
         */
        PiStatus read(char* buffer, unsigned int size, PiTime timeout);

        /**
         * This function empties the contents of the queue, which wakes up
         * tasks that blocks on writing
         */
        void flush();

    protected:

    private:

        /**
         * Not supported constructors and operators. Kept private
         * to avoid default behaviour
         */
        OwQueue(const OwQueue&);
        OwQueue& operator=(const OwQueue&);

        /*
         * Private member function
         */
        unsigned char*  m_queueBuffer;  /**< queue buffer: circular buff */
        unsigned int    m_queueSize;    /**< queue size */
        unsigned int    m_avail;        /**< available data in bytes */
        unsigned int    m_empty;        /**< queue empty size in bytes */
        unsigned int    m_readOffset;   /**< read offset for circular buff */
        unsigned int    m_writeOffset;  /**< write offset for circular buff */
        pthread_mutex_t m_mutex;        /**< mutex for data operation */
        pthread_cond_t  m_cond;         /**< condition variable */

};

//=============================================================================

#endif /* ifndef OWQUEUE_H */

//=============================================================================
