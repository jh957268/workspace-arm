//=============================================================================
/**
 * @file OwQueue.cpp
 *
 * @brief This file contains implementation for the OwQueue class
 *
 */
//=============================================================================

#include <time.h>
#include <errno.h>
#include <string.h>

#include <iostream>
using namespace std;

#include "OwQueue.h"

//=============================================================================
/**
 * OwQueue is implemented using circular buffer along with pthread cond_wait
 * feature.
 * On write operation,
 *      1. Obtain mutex
 *      2. Given data is written to circular buffer on enough space availability
 *         (signal from read thread)
 *      3. Signal threads that are pending for data in the queue (read) using
 *          condition_wait variable
 *      4. Release mutex
 *
 * On read operation,
 *      1. Obtain mutex
 *      2. Read data from circular buffer on enough data availability
 *         (signal from write thread)
 *      3. Signal threads that are pending for data to write onto the queue
 *         (write) using condition_wait variable
 *      4. Release mutex
 *
 * On flush operation,
 *      1. Obtain mutex
 *      2. reset circular buffer resources (offset, avail, empty)
 *      3. Signal threads that are pending for data to write onto the queue
 *         (write) using condition_wait variable
 *      4. Release mutex
 */

/**
 * Constructor
 */
OwQueue::OwQueue(unsigned int size)
    : m_queueBuffer((unsigned char*)0)
    , m_queueSize(size)
    , m_avail(0)
    , m_empty(size)
    , m_readOffset(0)
    , m_writeOffset(0)
{
    // allocate buffer
    m_queueBuffer = new unsigned char[size];
    //if (NULL == m_queueBuffer)
        // TODO: throw exception ?

    // mutex and condition variable for data operations and task pend/resume
    // operations
    pthread_mutex_init(&m_mutex, NULL); // 'fast' mutex
    pthread_cond_init(&m_cond, NULL);
}

//=============================================================================
/**
 * Destructor
 */
OwQueue::~OwQueue()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
    delete [] m_queueBuffer;
}

//=============================================================================
/**
 * This function writes the contents of the buffer to the queue.
 * If the empty space in the queue is not enough to hold the data,
 * the calling task blocks. It continues blocking until either the
 * queue has enough space or the function timesout.
 */
PiStatus
OwQueue::write(const char* buffer, unsigned int size, PiTime timeout)
{
    PiStatus retCode     = OK;
    struct timespec tout = {0, 0};
    int retcode          = 0;

    pthread_mutex_lock(&m_mutex);

    if (m_empty < size)
    {
        // need to wait till circular buffer has enough space to write
        if (timeout == PI_FOREVER)
        {
            while (m_empty < size) {
                retcode = pthread_cond_wait(&m_cond, &m_mutex);
            }
        }
        else
        {
            int secs    = (int)(timeout / 1000);
            int msecs   = (int)(timeout % 1000);

            // set timeout
            tout.tv_sec  = (int)(time(NULL) + secs);
            tout.tv_nsec = msecs * 1000; // nanoseconds

            // need to wait till circular buffer has enough space to write
            // or till timeout expires
            while ((m_empty < size) && (retcode != ETIMEDOUT)) {
                retcode = pthread_cond_timedwait(&m_cond, &m_mutex, &tout);
            }
        }
    }

    if (retcode == 0)
    {
        unsigned int tmpOffset = m_writeOffset + size;

        if (tmpOffset > m_queueSize)
            tmpOffset -= m_queueSize;

        // copy the data into the queue circular buffer
        if ((m_writeOffset + size) < m_queueSize)
            memcpy(m_queueBuffer + m_writeOffset, buffer, size);
        else {
            int copy1 = m_queueSize - m_writeOffset;
            if (copy1)
                memcpy(m_queueBuffer + m_writeOffset, buffer, copy1);
            if (size - copy1)
                memcpy(m_queueBuffer, buffer + copy1, (size - copy1));
        }

        m_writeOffset = tmpOffset;
        m_empty      -= size;
        m_avail      += size;
        pthread_cond_broadcast(&m_cond);    // wakeup pending threads
    }
    else if (retcode == ETIMEDOUT) {
        retCode = TIMEOUT; // timeout occurred
    }
    else
        retCode = ERROR;

    pthread_mutex_unlock(&m_mutex);

    return retCode;
}

//=============================================================================
/**
 * This function reads number of bytes from the queue. If the contents
 * in the queue have fewer bytes than the buffer, the calling task
 * blocks. It continues to block until queue contains enough bytes, or
 * the function timesout.
 */
PiStatus
OwQueue::read(char* buffer, unsigned int size, PiTime timeout)
{
    PiStatus retCode     = OK;
    struct timespec tout = {0, 0};
    int retcode          = 0;

    pthread_mutex_lock(&m_mutex);

    if (m_avail < size)
    {
        // need to wait till circular buffer has requested size of data
        if (timeout == PI_FOREVER)
        {
            while (m_avail < size) {
                retcode = pthread_cond_wait(&m_cond, &m_mutex);
            }
        }
        else
        {
            int secs    = (int)(timeout / 1000);
            int msecs   = (int)(timeout % 1000);

            // set timeout
            tout.tv_sec  = (int)(time(NULL) + secs);
            tout.tv_nsec = msecs * 1000; // nanoseconds

            // need to wait till circular buffer has requested size of data
            // or till timeout expires
            while ((m_avail < size) && (retcode != ETIMEDOUT)) {
                retcode = pthread_cond_timedwait(&m_cond, &m_mutex, &tout);
            }
        }
    }

    if (retcode == 0)
    {
        // always buffer is filled with requested size
        if ((m_readOffset + size) < m_queueSize)
        {
            memcpy(buffer, m_queueBuffer + m_readOffset, size);
        }
        else {
            int copy1 = m_queueSize - m_readOffset;
            if (copy1)
                memcpy(buffer, m_queueBuffer + m_readOffset, copy1);
            if (size - copy1)
                memcpy(buffer + copy1, m_queueBuffer, (size - copy1));
        }

        m_readOffset += size;
        if (m_readOffset > m_queueSize)
            m_readOffset -= m_queueSize;

        m_avail -= size;
        m_empty += size;
        pthread_cond_broadcast(&m_cond);    // wakeup pending threads
    }
    else if (retcode == ETIMEDOUT) {
        retCode = TIMEOUT; // timeout occurred
    }
    else
        retCode = ERROR;

    pthread_mutex_unlock(&m_mutex);

    return retCode;
}

//=============================================================================
/**
 * This function empties the contents of the queue, which wakes up
 * tasks that blocks on writing
 */
void
OwQueue::flush()
{
    pthread_mutex_lock(&m_mutex);

    // empty the queue
    m_avail         = 0;
    m_empty         = m_queueSize;
    m_readOffset    = 0;
    m_writeOffset   = 0;

    pthread_cond_broadcast(&m_cond);    // wakeup pending threads
    pthread_mutex_unlock(&m_mutex);
}

//=============================================================================
