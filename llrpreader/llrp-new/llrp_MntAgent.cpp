#include  "llrp_MntAgent.h"
#include  "llrp_MntServer.h"
#include  "llrp_MsgProcessor.h"
#include  "llrp_ROSpecExecutor.h"

#include  "iReaderapi.h"
#include  "debug_print.h"

#include <iostream>

using namespace std;

unsigned int LLRP_MntAgent::nextId = 0;

#define _time64		time

//=============================================================================
// Constructor

LLRP_MntAgent::LLRP_MntAgent():
	OwTask( MEDIUM, 2048, "llrpMntAgent" ),
	isSocketClosed ( false ),
	id( ++nextId )
{
	DBG_PRINT( DEBUG_INFO, "LLRPMntAgent[%d]:: Created"NL, id );
	idle = false;					// Assume it will be run right away to avoid race condition 
									// that the MntServer will use it again if another connection coming in
	m_Recv.bFrameValid = false;
	m_Recv.nBuffer = 0;
	m_pRcvMessage = 0;
	m_biReaderIsFailed = FALSE;
	mpLLRP_ROSpecExecutor = 0;

} // AkMntAgent::AkMntAgent()

INT32
LLRP_MntAgent::initialize(void)
{
	if (0 != iReaderHandle)
		return IREADER_SUCCESS;

	DBG_PRINT(DEBUG_INFO,"LLRP_MntAgent (%d) initialize iReader %s, please wait..."NL, id, m_iReader_ip);
	iReaderHandle = IReaderApiInit((char *)m_iReader_ip, REGION_USA);

	if ( 0 == iReaderHandle)
	{
		DBG_PRINT(DEBUG_INFO,"LLRP_MntAgent (%d) initialize iReader Fails!!"NL, id);
		return (-1);
	}

	DBG_PRINT(DEBUG_INFO,"LLRP_MntAgent (%d) initialize iReader Success!!"NL, id);
	DBG_PRINT(DEBUG_INFO,"LLRP_MntAgent (%d) retrieves connected antennas list..Please wait.."NL, id);
	// Now rescan all the RF Port to get the antenna list
	INT32 status;

#if 0 // Done in IReaderApiInit to avoid doing a again in case the LLRPagent connect to the same reader
	for (int i = 1; i <= 8; i++)
	{
		status = IReaderApiSyncChannel(iReaderHandle, i);
		if (IREADER_SUCCESS != status)
		{
			DBG_PRINT(DEBUG_INFO,"Rescan iReader Channel fails, close iReader Handler."NL);
			IReaderApiClose(iReaderHandle);
			return (status);
		}
	}
#endif
	status = IReaderApiGetAntList(iReaderHandle, &m_antCount, m_antList);

	if (IREADER_SUCCESS != status)
	{
		IReaderApiClose(iReaderHandle);
		return status;
	}

   	printAntennaList();

	for (int i = 0; i <= MAX_NUM_ANTENNAS; i++)
	{
		setAntPower(i, ANT_DEFAULT_POWER);
		setAntState(i, ANT_UNPLUGED);
	}

	for (int i = 0; i < m_antCount; i++)
	{
		setAntState(m_antList[i],  ANT_PLUGED);
	}
	m_enableRSSI = 0;
	m_enablePCBits = 0;
	m_enableCRCBits = 0;
	m_eROReportTrigger = ROReportTriggerType_None;
	m_report_N = 0;
	IReaderApiTagSearchTimeout(iReaderHandle, DEFAULT_TAG_SEARCH_TIME);
	return IREADER_SUCCESS;
}

void
LLRP_MntAgent::printAntennaList(void)
{
	DBG_PRINT(DEBUG_INFO,"Connected antennas List:"NL);
	DBG_PRINT(DEBUG_INFO,"{ ");
	for (int i = 0; i < m_antCount; i++)
	{
		DBG_Printf("%d ", m_antList[i]);
	}
	DBG_Printf(" }"NL);
}

//=============================================================================
// main
// Receive the C&M from Ethernet at this routine

void
LLRP_MntAgent::main
(
	OwTask *
)
{
	DBG_PRINT(DEBUG_INFO, "LLRP_MntAgent[%d]:: Running"NL, id );

	socketDeliver();
					
	DBG_PRINT(DEBUG_INFO, "LLRP_MntAgent[%d]:: Ending"NL, id );
	idle = true;  // Once this is set to true, other connection (from LLRP_Mntserver) may use this object before the
	              // phtread exit if preemption "can" happen

} // AkMntAgent::main()


//=============================================================================
// setConnection
// This function is call by "AkMntServer" to pass down the file
// descriptor of a newly established TCP socket. It also gives a
// semaphore to release its pending thread.

void
LLRP_MntAgent::setConnection
(
	SOCKET  connectClient
)
{
	DBG_PRINT(DEBUG_INFO, "LLRP_MntAgent[%d]:: Socket %d assigned"NL, id, connectClient );

	clientSocket = connectClient;
	// sem.give();				 // Release the thread

} // LLRP_MntAgent::setConnection()


//=============================================================================
// send

void
LLRP_MntAgent::sendMessage
(
	const char*  message
)
{
	int  iResult = ::send( clientSocket, message, strlen(message), 0 );

	if ( iResult == SOCKET_ERROR )
	{
		DBG_PRINT(DEBUG_INFO, "LLRP_MntAgent[%d]:: sendMessage failed with error: %d"NL, id, iResult );

		disconnect();
	}
//	SPrintf( "AkMntAgent[%d]:: Msg sent"NL, id );

} // AkMntAgent::sendMessage()


//=============================================================================
// socketDeliver

void
LLRP_MntAgent::socketDeliver
(
)
{
	isSocketClosed = false;

	sendReaderEventNotification();

	while ( true )
	{
		EResultCode eRC = recvAdvance(1000ul, 0);

		// Process this first
		if (m_biReaderConnectFailed == TRUE)
		{
			send_iReader_Failed_Notification();
			DBG_PRINT(DEBUG_CRITICAL,"iReader failed to connect!!!"NL);
			IReaderApiClose(iReaderHandle);
			iReaderHandle = 0;
			closesocket(clientSocket);
			return;
		}
		// receive messages until disconnected

		if ( RC_RecvTimeout == eRC )
		{
			if (m_biReaderIsFailed == TRUE)
			{
				send_iReader_Failed_Notification();
				// close the iReader and release the iReaderHandle */
 				// One this iReader is closed, the other client or agent associated to this iReader will 
				// detect this iReader has failed, will also flow to this point and will also try to close the 
				// iReader. Since this iReader is alreday closed, the close will not has effect, and is OK.
				// JOO 11-29-2012

				DBG_PRINT(DEBUG_CRITICAL,"iReader has failed!!!"NL);
				IReaderApiClose(iReaderHandle);
				clearList_ADD_ROSPEC();
				iReaderHandle = 0;
				m_biReaderIsFailed = FALSE;
				// do not close the client, this client may want to talk to another iReader, JOO 11-29-012
				// closesocket(clientSocket);
				// return;
				continue;
			}

			if (iKeepAliveSendCount > 2)
			{
				DBG_PRINT(DEBUG_WARNING,"Client does not response keep alive message"NL);
				stopAllROSpec();
				// Better code (will change) should Check the idle flag instead of sleep
				// JOO 11-29-2012
				OwTask::sleep(500);		// to make sure executor has exit before clearling and deleting all the ROSPEC, it is good to use a flag to synchoronize
				clearList_ADD_ROSPEC();
				// Do not close the iReader, other agents or client may still talk to the iReader
				// (Multiple agent or client (logical readers) can connect to the same iReader)
				// JOO, 11-29,2012
				// IReaderApiClose(iReaderHandle);
				iReaderHandle = 0;
				closesocket(clientSocket);
				return;
			}

			CKEEPALIVE keepAlive;
			keepAlive.setMessageID(16);

			sendMessage(&keepAlive);
			iKeepAliveSendCount++;
			// DBG_PRINT(DEBUG_INFO,"Keep Alive"NL);

			// delete pcKeepAlive;
			// ::send( clientSocket, (const char *)keepAlive, 10, 0 );

			continue;
		}
		else if ( RC_OK == eRC )
		{
			if ( 0 != m_pRcvMessage )
			{
				
				LLRP_MsgProcessor::getInstance()->processMessage(this, m_pRcvMessage);

				// delete m_pRcvMessage; processMessage will delete m_pRcvMessage
				m_pRcvMessage = 0;
			}
		}
		else
		{
			/* the socket is closed or error */
			DBG_PRINT(DEBUG_INFO,"Client socket is closed or Error"NL);
			break;
		}

#if 0
		FD_ZERO(&readset);
		FD_SET(clientSocket, &readset);
		int  iResult = select(clientSocket + 1, &readset, NULL, NULL, &tv);

		if ( iResult == 0 )
		{
			// Timeout, send keep alive

			DBG_PRINT(DEBUG_INFO,"Keep Alive"NL);
			::send( clientSocket, (const char *)keepAlive, 10, 0 );

			continue;
		}

		iResult = ::recv( clientSocket, m_rcvbuff, sizeof(m_rcvbuff), 0 );
		// Received successfully
		m_rcvbuff[iResult]=0;	 // Make sure it ends with a null
		DBG_PRINT(DEBUG_INFO, "LLRP_MntAgent[%d]:: received %d bytes"NL, getId(), iResult );

		if (m_rcvbuff[0] == 0x04 && m_rcvbuff[1] == 0x03)
		{
			DBG_PRINT(DEBUG_INFO,"Response"NL);
			::send( clientSocket, (const char *)response, 24, 0 );
			
		}
#endif

#if 0
		if ( !isSocketClosed )
		{
		   AkMsgProcessor::getInstance()->process( this, recvbuf, iResult );
		}
		else
		{
			// out of synch
			DBG_Printf ("AkMntAgent: Synch to close socket.\n");
			close (connectSocket);
			break;
		}
#endif
	}
	stopAllROSpec();

	// Better code (will change) should Check the idle flag instead of sleep
	// JOO 11-29-2012
	OwTask::sleep(1000);		// to make sure executor has exit before clearling and deleting all the ROSPEC, it is good to use a flag to synchoronize
	clearList_ADD_ROSPEC();
	// Do not close the iReader, other agents or client may still talk to the iReader
	// (Multiple agent or client (logical readers) can connect to the same iReader)
	// JOO, 11-29,2012

	// IReaderApiClose(iReaderHandle);
	iReaderHandle = 0;
	closesocket(clientSocket);
} // LLRP_MntAgent::socketDeliver()


//=============================================================================
// disconnect
// This function is call by "AkMntServer" to disconnect the TCP socket.

void
LLRP_MntAgent::disconnectSocket
(
)
{
	// Close TCP socket
	// Linux environment

	isSocketClosed = true;
	closesocket(clientSocket);
	DBG_PRINT(DEBUG_INFO, "LLRP_MntAgent[%d]:: close socket"NL, id );


	// Tell the controller not to send more indication to here anymore
	//AkMsgProcessor::getInstance()->unsetController( this );

	// Tell AkRec that there is one less controller now
	// AkRec::getInstance()->decControllerCount();

	// inform the AkMntServer that it is going idle
	// AkMntServer::getInstance()->complete( this );

} // AkMntAgent::disconnectSocket()

//=============================================================================
// disconnect
// This function is call by "AkMntServer" to disconnect the TCP socket.

void
LLRP_MntAgent::disconnect
(
)
{
	// Close TCP socket
	// Linux environment

	isSocketClosed = true;
	closesocket(clientSocket);
	DBG_PRINT(DEBUG_INFO, "LLRP_MntAgent[%d]:: close socket"NL, id );

	// Tell the controller not to send more indication to here anymore
	//AkMsgProcessor::getInstance()->unsetController( this );

	// Tell AkRec that there is one less controller now
	//AkRec::getInstance()->decControllerCount();

	// inform the AkMntServer that it is going idle
	//AkMntServer::getInstance()->complete( this );

} // AkMntAgent::disconnect()


EResultCode
LLRP_MntAgent::recvAdvance (
  int		nMaxMS,
  time_t	timeLimit)
{
    CErrorDetails *pError = &m_Recv.ErrorDetails;

    /*
     * Clear the error details in the receiver state.
     */
    pError->clear();

    /*
     * Loop until victory or some sort of exception happens
     */
    for(;;)
    {
        int                     rc;

        /*
         * Note that the frame is in progress.
         * Existing buffer content, if any, is deemed
         * invalid or incomplete.
         */
        m_Recv.bFrameValid = FALSE;

        /*
         * Check to see if we have a frame in the buffer.
         * If not, how many more bytes do we need?
         *
         * LLRP_FrameExtract() status
         *
         * FRAME_ERROR          Impossible situation, like message
         *                      length too small or the like.
         *                      Recovery in this situation is
         *                      unlikely and probably the app
         *                      should drop the connection.
         *
         * FRAME_READY          Frame is complete. Details are
         *                      available for pre-decode decisions.
         *
         * FRAME_NEED_MORE      Need more input bytes to finish the frame.
         *                      The m_nBytesNeeded field is how many more.
         */
        m_Recv.FrameExtract = CFrameExtract(m_Recv.pBuffer, m_Recv.nBuffer);

        /*
         * Framing error?
         */
        if(CFrameExtract::FRAME_ERROR == m_Recv.FrameExtract.m_eStatus)
        {
            pError->resultCodeAndWhatStr(RC_RecvFramingError,
                    "framing error in message stream");
            break;
        }

        /*
         * Need more bytes? extractRc>0 means we do and extractRc is the
         * number of bytes immediately required.
         */
        if(CFrameExtract::NEED_MORE == m_Recv.FrameExtract.m_eStatus)
        {
            unsigned int        nRead = m_Recv.FrameExtract.m_nBytesNeeded;
            unsigned char *     pBufPos = &m_Recv.pBuffer[m_Recv.nBuffer];

            /*
             * Before we do anything that might block,
             * check to see if the time limit is exceeded.
             */
            if(0 != timeLimit)
            {
                if(time(NULL) > timeLimit)
                {
                    /* Timeout */
                    pError->resultCodeAndWhatStr(RC_RecvTimeout,
                            "timeout");
                    break;
                }
            }

            /*
             * If this is not a block indefinitely request use poll()
             * to see if there is data in time.
             */
            if(nMaxMS >= 0)
            {
#ifdef linux_use_poll
                struct pollfd   pfd;

                pfd.fd = m_pPlatformSocket->m_sock;
                pfd.events = POLLIN;
                pfd.revents = 0;

                rc = poll(&pfd, 1, nMaxMS);
#endif /* linux */
#if 1  //  WIN32 and linux use same socket call
                fd_set          readfds;
                struct timeval  timeout;

                timeout.tv_sec = nMaxMS / 1000u;
                timeout.tv_usec = (nMaxMS % 1000u) * 1000u;

                FD_ZERO(&readfds);
                FD_SET(clientSocket, &readfds);
                rc = select(-1, &readfds, NULL, NULL, &timeout);

#endif /* WIN32 */
                if(0 > rc)
                {
                    /* Error */
                    pError->resultCodeAndWhatStr(RC_RecvIOError,
                            "poll failed");
                    break;
                }
                if(0 == rc)
                {
                    /* Timeout */
                    pError->resultCodeAndWhatStr(RC_RecvTimeout,
                            "timeout");
                    break;
                }
            }

            /*
             * Read (recv) some number of bytes from the socket.
             */
            rc = recv(clientSocket, (char*)pBufPos, nRead, 0);
            if(0 > rc)
            {
                /*
                 * Error. Note this could be EWOULDBLOCK if the
                 * file descriptor is using non-blocking I/O.
                 * So we return the error but do not tear-up
                 * the receiver state.
                 */
                pError->resultCodeAndWhatStr(RC_RecvIOError,
                        "recv IO error");
                break;
            }

            if(0 == rc)
            {
                /* EOF */
                pError->resultCodeAndWhatStr(RC_RecvEOF,
                        "recv end-of-file");
                break;
            }

            /*
             * When we get here, rc>0 meaning some bytes were read.
             * Update the number of bytes present.
             * Then loop to the top and retry the FrameExtract().
             */
            m_Recv.nBuffer += rc;

            continue;
        }

        /*
         * Is the frame ready?
         * If a valid frame is present, decode and enqueue it.
         */
        if(CFrameExtract::READY == m_Recv.FrameExtract.m_eStatus)
        {
            /*
             * Frame appears complete. Time to try to decode it.
             */
            CFrameDecoder *     pDecoder;
            CMessage *          pMessage;

            /*
             * Construct a new frame decoder. It needs the registry
             * to facilitate decoding.
             */
            pDecoder = new CFrameDecoder(LLRP_MntServer::llrp_pTypeRegistry,
                    m_Recv.pBuffer, m_Recv.nBuffer);

            /*
             * Make sure we really got one. If not, weird problem.
             */
            if(pDecoder == NULL)
            {
                /* All we can do is discard the frame. */
                m_Recv.nBuffer = 0;
                m_Recv.bFrameValid = FALSE;
                pError->resultCodeAndWhatStr(RC_MiscError,
                        "decoder constructor failed");
                break;
            }

            /*
             * Now ask the nice, brand new decoder to decode the frame.
             * It returns NULL for some kind of error.
             */
            pMessage = pDecoder->decodeMessage();

            /*
             * Always capture the error details even when it works.
             * Whatever happened, we are done with the decoder.
             */
            m_Recv.ErrorDetails = pDecoder->m_ErrorDetails;

            /*
             * Bye bye and thank you li'l decoder.
             */
            delete pDecoder;

            /*
             * If NULL there was an error. Clean up the
             * receive state. Return the error.
             */
            if(NULL == pMessage)
            {
                /*
                 * Make sure the return is not RC_OK
                 */
                if(RC_OK == pError->m_eResultCode)
                {
                    pError->resultCodeAndWhatStr(RC_MiscError,
                            "NULL message but no error");
                }

                /*
                 * All we can do is discard the frame.
                 */
                m_Recv.nBuffer = 0;
                m_Recv.bFrameValid = FALSE;

                break;
            }

            /*
             * Yay! It worked. Enqueue the message.
             */
            // XXX m_listInputQueue.push_back(pMessage); JOO
			m_pRcvMessage = pMessage;

            /*
             * Note that the frame is valid. Consult
             * Recv.FrameExtract.m_MessageLength.
             * Clear the buffer count to be ready for next time.
             */
            m_Recv.bFrameValid = TRUE;
            m_Recv.nBuffer = 0;

            break;
        }

        /*
         * If we get here there was an FrameExtract status
         * we didn't expect.
         */

        /*NOTREACHED*/
        // assert(0);
    }

    return pError->m_eResultCode;
}

EResultCode
LLRP_MntAgent::sendMessage (
  CMessage *                    pMessage)
{
    CErrorDetails *             pError = &m_Send.ErrorDetails;
    CFrameEncoder *             pEncoder;

    /*
     * Clear the error details in the send state.
     */
    pError->clear();

    /*
     * Make sure the socket is open.
     */
    if(NULL == clientSocket)
    {
        pError->resultCodeAndWhatStr(RC_MiscError, "not connected");
        return pError->m_eResultCode;
    }

    /*
     * Construct a frame encoder. It needs to know the buffer
     * base and maximum size.
     */
    pEncoder = new CFrameEncoder(m_Send.pBuffer, SEND_BUFFER_SIZE /*m_nBufferSize*/);

    /*
     * Check that the encoder actually got created.
     */
    if(NULL == pEncoder)
    {
        pError->resultCodeAndWhatStr(RC_MiscError,
                "encoder constructor failed");
        return pError->m_eResultCode;
    }

    /*
     * Encode the message. Return value is ignored.
     * We check the encoder's ErrorDetails for results.
     */
    pEncoder->encodeElement(pMessage);

    /*
     * Regardless of what happened capture the error details
     * and the number of bytes placed in the buffer.
     */
    m_Send.ErrorDetails = pEncoder->m_ErrorDetails;
    m_Send.nBuffer = pEncoder->getLength();

    /*
     * Bye bye li'l encoder.
     */
    delete pEncoder;

    /*
     * If the encoding appears complete write the frame
     * to the connection. NB: this is not ready for
     * non-blocking I/O (EWOULDBLOCK).
     */
    if(RC_OK == pError->m_eResultCode)
    {
        int             rc;

        rc = send(clientSocket, (char*)m_Send.pBuffer,
            m_Send.nBuffer, 0);
        if(rc != (int)m_Send.nBuffer)
        {
            /* Yikes! */
            pError->resultCodeAndWhatStr(RC_SendIOError, "send IO error");
        }
    }

    /*
     * Done.
     */
    return pError->m_eResultCode;
}

void
LLRP_MntAgent::sendReaderEventNotification(void)
{
	DBG_PRINT(DEBUG_INFO,"LLRP Connection Notify"NL);

	CREADER_EVENT_NOTIFICATION * pcEventNotify = new CREADER_EVENT_NOTIFICATION;
	CReaderEventNotificationData *pcNotifyData = new CReaderEventNotificationData;

	CConnectionAttemptEvent *pcAttemptEvent = new CConnectionAttemptEvent;

	EConnectionAttemptStatusType eStatuse = ConnectionAttemptStatusType_Success;

	CUTCTimestamp *pcTimestamp = new CUTCTimestamp;
	__time64_t time64us = _time64(0) * (__time64_t)1e6;
	pcTimestamp->setMicroseconds(time64us);
	
	pcAttemptEvent->setStatus(eStatuse);

	pcNotifyData->setTimestamp(pcTimestamp);
	pcNotifyData->setConnectionAttemptEvent(pcAttemptEvent);
	pcEventNotify->setReaderEventNotificationData(pcNotifyData);

	sendMessage(pcEventNotify);

	delete pcEventNotify;
}

void
LLRP_MntAgent::send_iReader_Failed_Notification(void)
{
	CREADER_EVENT_NOTIFICATION *pEvtNotification = new CREADER_EVENT_NOTIFICATION;
	CReaderEventNotificationData * pEvtNotificationData = new CReaderEventNotificationData;

	CReaderExceptionEvent *pReaderExceptionEvent = new CReaderExceptionEvent;

	llrp_utf8v_t exception_Message(32);

	strcpy((char *)exception_Message.m_pValue, "iReader Has Failed");
	exception_Message.m_nValue = strlen((const char *)exception_Message.m_pValue);
	pReaderExceptionEvent->setMessage(exception_Message);

	CROSpecID *pROSpecID = new CROSpecID;
	pROSpecID->setROSpecID(m_uiFailedROSpecID);
	pReaderExceptionEvent->setROSpecID(pROSpecID);
	pEvtNotificationData->setReaderExceptionEvent(pReaderExceptionEvent);

	CUTCTimestamp *pcTimestamp = new CUTCTimestamp;
	__time64_t time64us = _time64(0) * (__time64_t)1e6;
	pcTimestamp->setMicroseconds(time64us);

	pEvtNotificationData->setTimestamp(pcTimestamp);

	pEvtNotification->setReaderEventNotificationData(pEvtNotificationData);
	sendMessage(pEvtNotification);

	delete pEvtNotification;
}

#define HAVE_IREADER	1
EResultCode	
LLRP_MntAgent::addROSpecToList( CMessage *  pMessage )
{
#if HAVE_IREADER
	if ( 0  == iReaderHandle )
	{
		return RC_MiscError;
	}
#endif
	if(NULL != pMessage)
	{
		CADD_ROSPEC * pAddROSpec = (CADD_ROSPEC *)pMessage;
		CROSpec *pROSpec = pAddROSpec->getROSpec();
		pROSpec->setCurrentState(ROSpecState_Disabled);
		m_list_ADD_ROSPEC.push_back(pMessage);
	}
	return RC_OK;
}

EResultCode 
LLRP_MntAgent::enableROSpec (llrp_u32_t ROSpecID )
{

	for (
		std::list<CMessage *>::iterator msg = m_list_ADD_ROSPEC.begin();
		msg != m_list_ADD_ROSPEC.end();
		msg++)
	{
		CADD_ROSPEC * pAddROSpec = (CADD_ROSPEC *)*msg;
		CROSpec *pROSpec = pAddROSpec->getROSpec();
		if (pROSpec->getROSpecID() == ROSpecID)
		{
			if (pROSpec->getCurrentState() != ROSpecState_Disabled)
			{
				return RC_MiscError;
			}

			pROSpec->setCurrentState(ROSpecState_Inactive);
			return RC_OK;
		}
	}
	return RC_MiscError;
}

EResultCode
LLRP_MntAgent::startROSpec (llrp_u32_t ROSpecID )
{
	for (
		std::list<CMessage *>::iterator msg = m_list_ADD_ROSPEC.begin();
		msg != m_list_ADD_ROSPEC.end();
		msg++)
	{

		CADD_ROSPEC * pAddROSpec = (CADD_ROSPEC *)*msg;
		CROSpec *pROSpec = pAddROSpec->getROSpec();
		if (pROSpec->getROSpecID() == ROSpecID)
		{
			if (ROSpecState_Inactive != pROSpec->getCurrentState() )
			{
				return RC_MiscError;
			}
			pROSpec->setCurrentState(ROSpecState_Active);
			if ( 0 == mpLLRP_ROSpecExecutor)
			{
				mpLLRP_ROSpecExecutor = new LLRP_ROSpecExecutor(this, *msg);
			}
			else
			{
				mpLLRP_ROSpecExecutor->setController(this);
				mpLLRP_ROSpecExecutor->setROSpec(*msg);
			}
			mpLLRP_ROSpecExecutor->setRunning();
			mpLLRP_ROSpecExecutor->run();
			return RC_OK;
		}
	}
	return RC_MiscError;
}

EResultCode
LLRP_MntAgent::stopROSpec (llrp_u32_t ROSpecID )
{

	for (
		std::list<CMessage *>::iterator msg = m_list_ADD_ROSPEC.begin();
		msg != m_list_ADD_ROSPEC.end();
		msg++)
	{

		CADD_ROSPEC * pAddROSpec = (CADD_ROSPEC *)*msg;
		CROSpec *pROSpec = pAddROSpec->getROSpec();
		if (pROSpec->getROSpecID() == ROSpecID)
		{
			if (ROSpecState_Active != pROSpec->getCurrentState() )
			{
				return RC_MiscError;
			}
			pROSpec->setCurrentState(ROSpecState_Inactive);
			return RC_OK;
		}
	}
	return RC_MiscError;
}

void
LLRP_MntAgent::stopAllROSpec (void )
{

	for (
		std::list<CMessage *>::iterator msg = m_list_ADD_ROSPEC.begin();
		msg != m_list_ADD_ROSPEC.end();
		msg++)
	{

		CADD_ROSPEC * pAddROSpec = (CADD_ROSPEC *)*msg;
		CROSpec *pROSpec = pAddROSpec->getROSpec();

		pROSpec->setCurrentState(ROSpecState_Inactive);
	}
	return;
}

EResultCode
LLRP_MntAgent::deleteROSpec (llrp_u32_t ROSpecID )
{
	for (
		std::list<CMessage *>::iterator msg = m_list_ADD_ROSPEC.begin();
		msg != m_list_ADD_ROSPEC.end();
		msg++)
	{

		CADD_ROSPEC * pAddROSpec = (CADD_ROSPEC *)*msg;
		CROSpec *pROSpec = pAddROSpec->getROSpec();
		if (pROSpec->getROSpecID() == ROSpecID)
		{
			if (ROSpecState_Active == pROSpec->getCurrentState() )
			{
				return RC_MiscError;
			}
			m_list_ADD_ROSPEC.remove(*msg);
			delete (*msg);
			return RC_OK;
		}
	}
	return RC_MiscError;
}

void
LLRP_MntAgent::send_GET_ROSPEC_RESPONSE(const CMessage *pMsg)
{
	CGET_ROSPECS_RESPONSE * pResp = new CGET_ROSPECS_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);

	strcpy((char *)ErrorDesc.m_pValue, "Success"); 
	ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);

	pLLRPstatus->setStatusCode(StatusCode_M_Success);

	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list addSubParameterToAllList
#if 1
	for (
		std::list<CMessage *>::iterator msg = m_list_ADD_ROSPEC.begin();
		msg != m_list_ADD_ROSPEC.end();
		msg++)
	{

		CADD_ROSPEC * pAddROSpec = (CADD_ROSPEC *)*msg;
		CROSpec *pROSpec = pAddROSpec->getROSpec();

		if (pROSpec != 0)
		{
			CROSpec *pRspROSpec = new CROSpec;
			rospecCopy(pROSpec, pRspROSpec);

			pResp->addROSpec(pRspROSpec);

		}
	}
#endif
	pResp->setMessageID(pMsg->getMessageID());
	DBG_PRINT(DEBUG_INFO,"Sending  CGET_ROSPRC_RESPONSE Response"NL);
	sendMessage(pResp);

	delete pResp;
}

void
LLRP_MntAgent::rospecCopy(CROSpec * src, CROSpec * dst)
{
	CROBoundarySpec *pROBoundarySpec;
	CROSpecStopTrigger * pROSpecStopTrigger;
	CROSpecStartTrigger * pROSpecStartTrigger;

	pROBoundarySpec = src->getROBoundarySpec();

	pROSpecStopTrigger = pROBoundarySpec->getROSpecStopTrigger();
	pROSpecStartTrigger = pROBoundarySpec->getROSpecStartTrigger();

	CROBoundarySpec *pDstROBoundarySpec = new CROBoundarySpec;
	CROSpecStopTrigger * pDstROSpecStopTrigger = new CROSpecStopTrigger;
	CROSpecStartTrigger * pDstROSpecStartTrigger = new CROSpecStartTrigger;

	pDstROSpecStopTrigger->setROSpecStopTriggerType( pROSpecStopTrigger->getROSpecStopTriggerType());
	pDstROSpecStopTrigger->setDurationTriggerValue( pROSpecStopTrigger->getDurationTriggerValue());
	pDstROSpecStartTrigger->setROSpecStartTriggerType(pROSpecStartTrigger->getROSpecStartTriggerType());

	CPeriodicTriggerValue *pPeriodicTriggerValue = pROSpecStartTrigger->getPeriodicTriggerValue();

	if ( NULL != pPeriodicTriggerValue)
	{
		CPeriodicTriggerValue *pDstPeriodicTriggerValue = new CPeriodicTriggerValue;
		pDstPeriodicTriggerValue->setOffset( pPeriodicTriggerValue->getOffset());
		pDstPeriodicTriggerValue->setPeriod( pPeriodicTriggerValue->getPeriod());
		pDstROSpecStartTrigger->setPeriodicTriggerValue(pDstPeriodicTriggerValue);

	}

	pDstROBoundarySpec->setROSpecStartTrigger(pDstROSpecStartTrigger);
	pDstROBoundarySpec->setROSpecStopTrigger(pDstROSpecStopTrigger);
	dst->setROBoundarySpec(pDstROBoundarySpec);

	do
    {
		CAISpec *  pAISpec, * pDstAISpec;
		CParameter * pParameter;

		for (
			std::list<CParameter *>::iterator param = src->beginSpecParameter();
			param != src->endSpecParameter();
			param++)
		{
			const CTypeDescriptor *     pType;

			pParameter = *param;

			pType = pParameter->m_pType;

  			if(&CAISpec::s_typeDescriptor != pType)
				continue;

			pAISpec = ( CAISpec *)pParameter;
			pDstAISpec = new CAISpec;
			CAISpecStopTrigger *pAISpecStopTrigger = pAISpec->getAISpecStopTrigger();
			CAISpecStopTrigger *pAIDstSpecStopTrigger = new CAISpecStopTrigger;

			pAIDstSpecStopTrigger->setAISpecStopTriggerType(pAISpecStopTrigger->getAISpecStopTriggerType());
			pAIDstSpecStopTrigger->setDurationTrigger(pAISpecStopTrigger->getDurationTrigger());

			CTagObservationTrigger *pTagObservationTrigger = pAISpecStopTrigger->getTagObservationTrigger();

			if (NULL != pTagObservationTrigger)
			{
				CTagObservationTrigger *pDstTagObservationTrigger = new CTagObservationTrigger;

#if 1
				pDstTagObservationTrigger->setTriggerType(pTagObservationTrigger->getTriggerType());
				pDstTagObservationTrigger->setNumberOfTags(pTagObservationTrigger->getNumberOfTags());
				pDstTagObservationTrigger->setNumberOfAttempts(pTagObservationTrigger->getNumberOfAttempts());
				pDstTagObservationTrigger->setT(pTagObservationTrigger->getT());
				pDstTagObservationTrigger->setTimeout(pTagObservationTrigger->getTimeout());
#endif
				// *pDstTagObservationTrigger = *pTagObservationTrigger;
				pAIDstSpecStopTrigger->setTagObservationTrigger(pDstTagObservationTrigger);
			}

			pDstAISpec->setAISpecStopTrigger(pAIDstSpecStopTrigger);
			llrp_u16v_t antennaIDs = pAISpec->getAntennaIDs();
			pDstAISpec->setAntennaIDs(antennaIDs);

			for (
				std::list<CInventoryParameterSpec *>::iterator elm = pAISpec->beginInventoryParameterSpec();
				elm != pAISpec->endInventoryParameterSpec();
				elm++ )
			{
				//elm = pAISpec->beginInventoryParameterSpec();
				// pInventoryParamSpec = *(pAISpec->beginInventoryParameterSpec());
				CInventoryParameterSpec *pInventoryParamSpec = *elm;
				CInventoryParameterSpec *pDstInventoryParamSpec = new CInventoryParameterSpec;

				llrp_u16_t inventoryParmSpecID = pInventoryParamSpec->getInventoryParameterSpecID();
				pDstInventoryParamSpec->setInventoryParameterSpecID(inventoryParmSpecID);

				EAirProtocols inventoryProtocolID = pInventoryParamSpec->getProtocolID();
				pDstInventoryParamSpec->setProtocolID(inventoryProtocolID);
			
				for (
					std::list<CAntennaConfiguration *>::iterator antCfgBeginElm = pInventoryParamSpec->beginAntennaConfiguration();
					antCfgBeginElm != pInventoryParamSpec->endAntennaConfiguration();
					antCfgBeginElm++ )
				{
					// Not doing anything for now
					CAntennaConfiguration *pAntennaConfiguration = *antCfgBeginElm;
					CAntennaConfiguration *pDstAntennaConfiguration = new CAntennaConfiguration;

					llrp_u16_t antID = pAntennaConfiguration->getAntennaID();
					pDstAntennaConfiguration->setAntennaID(antID);

					CRFTransmitter * pRFTransmitter = pAntennaConfiguration->getRFTransmitter();
					if ( 0 != pRFTransmitter )
					{
						llrp_u16_t txPower = pRFTransmitter->getTransmitPower();

						CRFTransmitter * pDstRFTransmitter = new CRFTransmitter;
						pDstRFTransmitter->setTransmitPower(txPower);

						pDstAntennaConfiguration->setRFTransmitter(pDstRFTransmitter);

					}
					pDstInventoryParamSpec->addAntennaConfiguration(pDstAntennaConfiguration);

				}
				pDstAISpec->addInventoryParameterSpec(pDstInventoryParamSpec);
			}
			dst->addSpecParameter(pDstAISpec);
		}

	} while (0);

	// process ROReportSpec
	CROReportSpec *pROReportSpec = src->getROReportSpec();

	if (NULL != pROReportSpec)
	{
		CROReportSpec *pDstROReportSpec = new CROReportSpec;

		//*pDstROReportSpec = *pROReportSpec;

		pDstROReportSpec->setROReportTrigger(pROReportSpec->getROReportTrigger());
		pDstROReportSpec->setN(pROReportSpec->getN());

		CTagReportContentSelector *pTagReportContentSelector = pROReportSpec->getTagReportContentSelector();

		if (NULL != pTagReportContentSelector)
		{
			CTagReportContentSelector *pDstTagReportContentSelector = new CTagReportContentSelector;
			//*pDstTagReportContentSelector = *pTagReportContentSelector;

			pDstTagReportContentSelector->setEnableAccessSpecID(pTagReportContentSelector->getEnableAccessSpecID());
			pDstTagReportContentSelector->setEnableAntennaID(pTagReportContentSelector->getEnableAntennaID());
			pDstTagReportContentSelector->setEnableAccessSpecID (pTagReportContentSelector->getEnableAccessSpecID());
			pDstTagReportContentSelector->setEnableChannelIndex (pTagReportContentSelector->getEnableChannelIndex());
			pDstTagReportContentSelector->setEnableFirstSeenTimestamp(pTagReportContentSelector->getEnableFirstSeenTimestamp());

			pDstTagReportContentSelector->setEnableInventoryParameterSpecID(pTagReportContentSelector->getEnableInventoryParameterSpecID());
			pDstTagReportContentSelector->setEnableLastSeenTimestamp(pTagReportContentSelector->getEnableLastSeenTimestamp());
			pDstTagReportContentSelector->setEnablePeakRSSI(pTagReportContentSelector->getEnablePeakRSSI());

			pDstTagReportContentSelector->setEnableROSpecID(pTagReportContentSelector->getEnableROSpecID());
			pDstTagReportContentSelector->setEnableSpecIndex(pTagReportContentSelector->getEnableSpecIndex());
			pDstTagReportContentSelector->setEnableTagSeenCount(pTagReportContentSelector->getEnableTagSeenCount());

			// process the list of Memory Selector
			for (
				std::list<CParameter *>::iterator param = pTagReportContentSelector->beginAirProtocolEPCMemorySelector();
				param != pTagReportContentSelector->endAirProtocolEPCMemorySelector();
				param++)
			{
				const CTypeDescriptor *     pType;

				CParameter *pParameter = *param;

				pType = pParameter->m_pType;

  				if(&CC1G2EPCMemorySelector::s_typeDescriptor != pType)
					continue;
				CC1G2EPCMemorySelector *pC1G2EPCMemorySelector = (CC1G2EPCMemorySelector *)pParameter;
				CC1G2EPCMemorySelector *pDstC1G2EPCMemorySelector = new CC1G2EPCMemorySelector;

				//*pDstC1G2EPCMemorySelector = *pC1G2EPCMemorySelector;
				pDstC1G2EPCMemorySelector->setEnableCRC(pC1G2EPCMemorySelector->getEnableCRC());
				pDstC1G2EPCMemorySelector->setEnablePCBits(pC1G2EPCMemorySelector->getEnablePCBits());
				pDstTagReportContentSelector->addAirProtocolEPCMemorySelector(pDstC1G2EPCMemorySelector);
			}


			pDstROReportSpec->setTagReportContentSelector(pDstTagReportContentSelector);
		}

		dst->setROReportSpec(pDstROReportSpec);
	}

	dst->setROSpecID(src->getROSpecID());
}

void 
LLRP_MntAgent::sendRO_Access_Report_OnDemand(void)
{
	if ( 0 == mpLLRP_ROSpecExecutor )
		return;
	if ( false == mpLLRP_ROSpecExecutor->isIdle())
		return;
	mpLLRP_ROSpecExecutor->sendRO_Access_Report();
}

//=============================================================================
