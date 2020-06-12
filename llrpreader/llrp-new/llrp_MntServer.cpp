//=============================================================================
///
/// @file AkMntServer.cpp
///
/// Copyright (c) 2009 MTI Laboratory Inc.
///
/// @brief < This file contains member function for AkMntServer class
//=============================================================================

#include  "llrp_MntServer.h"
#include  "llrp_MsgProcessor.h"
#include  "llrp_ROSpecExecutor.h"

#include  "iReaderapi.h"
#include  "debug_print.h"
#include  "muxserial.h"

#include <iostream>

using namespace std;

#define _time64		time

//=============================================================================

LLRP_MntServer* LLRP_MntServer::spInstance = 0;

const CTypeRegistry* LLRP_MntServer::llrp_pTypeRegistry = 0;

//=============================================================================
// Constructor

LLRP_MntServer::LLRP_MntServer(void * handle):
    OwTask( MEDIUM, 2048, "LLRPMntServer", true ),
    socketState( SOCKET_IDLE )
{
    iReaderHandle = handle;

	m_Recv.bFrameValid = false;
	m_Recv.nBuffer = 0;
	m_pRcvMessage = 0;
	m_biReaderIsFailed = FALSE;
	mpMutex = new OwMutex();
}

#if 0  // testing calling the contructor
LLRP_MntServer::LLRP_MntServer():
OwTask( MEDIUM, 2048, "LLRPMntServer" )
{
}
#endif

INT32
LLRP_MntServer::initialize(void)
{
	INT32 status = IReaderApiGetAntList(iReaderHandle, &m_antCount, m_antList);

	if (IREADER_SUCCESS != status)
	{
		return status;
	}

	for (int i = 0; i <= MAX_NUM_ANTENNAS; i++)
	{
		setAntPower(i, ANT_DEFAULT_POWER);
		setAntState(i, ANT_UNPLUGED);
	}

	for (int i = 0; i < m_antCount; i++)
	{
		setAntState(m_antList[i],  ANT_PLUGED);
	}
	IReaderApiTagSearchTimeout(iReaderHandle, 100);
	return IREADER_SUCCESS;
}


//=============================================================================
// main

void
LLRP_MntServer::main( OwTask * )
{
//	if (IREADER_SUCCESS != initialize())
//	{
//		printf("Fail to initialize the iReader. Thread bails out..."NL);
//		return;
//	}
	bill_out = false;
    while ( true )
    {
    	if (true == bill_out)
    	{
    		return;
    	}
        switch ( socketState )
        {
        	case SOCKET_IDLE:
            socketInitialize();
            break;

        	case SOCKET_INITIAL:
            socketCreate();
            break;

        	case SOCKET_CREATE:
            socketBind();
            break;

        	case SOCKET_BIND:
            socketListen();
            break;

        	case SOCKET_LISTEN:
            socketAccept();
            break;

        	case SOCKET_ACCEPT:
        	if (-1 != clientSocket)
        	{
        		socketDeliver<LLRP_MntAgent,vector<LLRP_MntAgent*>>(agentPool,clientSocket);
        	}
        	if (-1 != iclientSocket)
        	{
           		socketDeliver<IAgent,vector<IAgent*>>(iagentPool,iclientSocket);
        		//isocketDeliver();
        	}
        	if (-1 != sclientSocket)
        	{
          		socketDeliver<SAgent,vector<SAgent*>>(sagentPool,sclientSocket);
        		//ssocketDeliver();
        	}

			socketState = SOCKET_LISTEN;			
										//return;	kill the thread
            break;
        }
    }	// while( true )

} // AkMntServer::main()

LLRP_MntServer
*LLRP_MntServer::getInstance
(
	UINT32 handle
)
{
	if ( 0 == spInstance )
	{
		spInstance = new LLRP_MntServer( IReader::getInstance());
	}

	return( spInstance );

} // LLRP_MntServer:getInstance()

int
LLRP_MntServer::initRegistry
(
	void
)
{
	CTypeRegistry *   pTypeRegistry;

	pTypeRegistry = getTheTypeRegistry();
    if(NULL == pTypeRegistry)
    {
        DBG_PRINT(DEBUG_CRITICAL, "ERROR: getTheTypeRegistry failed\n");
        return -1;
    }
	llrp_pTypeRegistry = pTypeRegistry;
	return 0;
}

//=============================================================================
// socketInitialize

void
LLRP_MntServer::socketInitialize()
{
    //----------------------------------------
    // Initialize Linux server address
    //
    memset( &saServer, 0, sizeof(saServer) );
    saServer.sin_family = AF_INET;
    saServer.sin_addr.s_addr = INADDR_ANY;
    saServer.sin_port = htons( SERVER_PORT );
    socketState = SOCKET_INITIAL;

} // LLRP_MntServer:socketInitialize()

//=============================================================================
// socketCreate

void
LLRP_MntServer::socketCreate()
{
    //------------------------------------------
    // Create a TCP socket in Linux
    listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);

    if ( -1 == listenSocket )
    {
        DBG_PRINT(DEBUG_CRITICAL, "AkMntServer:: Socket creation failed"NL );
        socketState = SOCKET_INITIAL;

        return;
    }
    int enable = 1;
    if (::setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    if (::setsockopt(listenSocket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    ilistenSocket = ::socket(AF_INET, SOCK_STREAM, 0);

    if ( -1 == ilistenSocket )
    {
        DBG_PRINT(DEBUG_CRITICAL, "AkMntServer:: Socket creation failed"NL );
        socketState = SOCKET_INITIAL;

        return;
    }

    if (::setsockopt(ilistenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    if (::setsockopt(ilistenSocket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
	
    slistenSocket = ::socket(AF_INET, SOCK_STREAM, 0);

    if ( -1 == slistenSocket )
    {
        DBG_PRINT(DEBUG_CRITICAL, "AkMntServer:: Socket creation failed"NL );
        socketState = SOCKET_INITIAL;

        return;
    }

    if (::setsockopt(slistenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    if (::setsockopt(slistenSocket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
	

    socketState = SOCKET_CREATE;
}

//=============================================================================
// socketBind

void
LLRP_MntServer::socketBind()
{
    // Bind socket to server address
    //
    int  iResult = ::bind( listenSocket, (SOCKADDR*) &saServer,
    		sizeof(saServer) );

    if ( SOCKET_ERROR == iResult )
    {
    	perror("bind");
        socketState = SOCKET_CREATE;
        bill_out = true;
        return;
    }

    saServer.sin_port = htons( ISERVER_PORT );
    iResult = ::bind( ilistenSocket, (SOCKADDR*) &saServer,
    		sizeof(saServer) );

    if ( SOCKET_ERROR == iResult )
    {
        socketState = SOCKET_CREATE;
        bill_out = true;
        return;
    }
	
    saServer.sin_port = htons( SSERVER_PORT );
    iResult = ::bind( slistenSocket, (SOCKADDR*) &saServer,
    		sizeof(saServer) );

    if ( SOCKET_ERROR == iResult )
    {
        socketState = SOCKET_CREATE;
        bill_out = true;
        return;
    }	

    socketState = SOCKET_BIND;
}

//=============================================================================
// socketListen

void
LLRP_MntServer::socketListen()
{
    DBG_PRINT(DEBUG_INFO, "LLRP_MntServer: Listening to port %d"NL, SERVER_PORT);
    int  iResult = ::listen( listenSocket, SOMAXCONN );

    if ( SOCKET_ERROR == iResult )
    {
        DBG_PRINT(DEBUG_CRITICAL, "AkMntServer:: Listen failed: %d"NL, iResult);

        socketState = SOCKET_BIND;
        bill_out = true;
        return;
    }

    DBG_PRINT(DEBUG_INFO, "LLRP_MntServer: Listening to port %d"NL, ISERVER_PORT);
    iResult = ::listen( ilistenSocket, SOMAXCONN );

    if ( SOCKET_ERROR == iResult )
    {
        DBG_PRINT(DEBUG_CRITICAL, "AkMntServer:: iListen failed: %d"NL, iResult);

        socketState = SOCKET_BIND;
        bill_out = true;
        return;
    }
	
    DBG_PRINT(DEBUG_INFO, "LLRP_MntServer: Listening to port %d"NL, SSERVER_PORT);
    iResult = ::listen( slistenSocket, SOMAXCONN );

    if ( SOCKET_ERROR == iResult )
    {
        DBG_PRINT(DEBUG_CRITICAL, "AkMntServer:: iListen failed: %d"NL, iResult);

        socketState = SOCKET_BIND;
        bill_out = true;
        return;
    }	

    socketState = SOCKET_LISTEN;
}

//=============================================================================
// socketAccept

void
LLRP_MntServer::socketAccept()
{
    //------------------------------
    // Accepting a client connection in Linux
    //
    int addrLen = sizeof( pin );


    fd_set readfds;
    SOCKET maxfd, fd;
    unsigned int i;
    int status;

    FD_ZERO(&readfds);
    maxfd = -1;
    FD_SET(listenSocket, &readfds);
    if (listenSocket > maxfd)
    {
    	maxfd = listenSocket;
    }
    FD_SET(ilistenSocket, &readfds);
    if (ilistenSocket > maxfd)
    {
    	maxfd = ilistenSocket;
    }
    FD_SET(slistenSocket, &readfds);
    if (slistenSocket > maxfd)
    {
    	maxfd = slistenSocket;
    }	

    do {
		status = ::select(maxfd + 1, &readfds, NULL, NULL, NULL);			
    } while (status < 0 && errno == EINTR);

    if (status < 0)
    {
        bill_out = true;
        return;
    }
    fd = INVALID_SOCKET;

#if 0	
    if (FD_ISSET(listenSocket, &readfds))
    {
    	fd = listenSocket;
    }
    else if (FD_ISSET(ilistenSocket, &readfds))
    {
    	fd = ilistenSocket;
    }
    else if (FD_ISSET(slistenSocket, &readfds))
    {
    	fd = slistenSocket;
    }	
	
    if (fd == INVALID_SOCKET)
    {
        bill_out = true;
        return;
    }
#endif

    clientSocket = -1;
    iclientSocket = -1;
	sclientSocket = -1;


    //if (fd == listenSocket)
	if (FD_ISSET(listenSocket, &readfds))	
    {
    	clientSocket = ::accept( listenSocket, (SOCKADDR *) &pin,(socklen_t *) &addrLen );

    	if ( -1 == clientSocket )
    	{
    		// Failure
    		DBG_PRINT(DEBUG_CRITICAL, "LLRP_MntServer:: accept failed"NL );
    		bill_out = true;
    		socketState = SOCKET_LISTEN;

    		return;
    	}
		DBG_PRINT(DEBUG_INFO, "LLRP_MntServer:: Accepting client from %s:%d "
            "at socket %d"NL,
            inet_ntoa(pin.sin_addr), ntohs(pin.sin_port), clientSocket);		
    }
    // else if (fd == ilistenSocket)
	if (FD_ISSET(ilistenSocket, &readfds))		
    {
    	iclientSocket = ::accept( ilistenSocket, (SOCKADDR *) &pin,(socklen_t *) &addrLen );

    	if ( -1 == iclientSocket )
    	{
    		// Failure
    		DBG_PRINT(DEBUG_CRITICAL, "LLRP_MntServer:: iaccept failed"NL );
    		bill_out = true;
    		socketState = SOCKET_LISTEN;

    		return;
    	}
		DBG_PRINT(DEBUG_INFO, "LLRP_MntServer:: Accepting iClient from %s:%d "
            "at socket %d"NL,
            inet_ntoa(pin.sin_addr), ntohs(pin.sin_port), iclientSocket);		
    }
    // else if (fd == slistenSocket)
	if (FD_ISSET(slistenSocket, &readfds))	
    {
    	sclientSocket = ::accept( slistenSocket, (SOCKADDR *) &pin,(socklen_t *) &addrLen );

    	if ( -1 == sclientSocket )
    	{
    		// Failure
    		DBG_PRINT(DEBUG_CRITICAL, "LLRP_MntServer:: iaccept failed"NL );
    		bill_out = true;
    		socketState = SOCKET_LISTEN;

    		return;
    	}
		DBG_PRINT(DEBUG_INFO, "LLRP_MntServer:: Accepting sClient from %s:%d "
            "at socket %d"NL,
            inet_ntoa(pin.sin_addr), ntohs(pin.sin_port), sclientSocket);		
    }	

    //DBG_PRINT(DEBUG_INFO, "LLRP_MntServer:: Accepting client from %s:%d "
    //        "at socket %d"NL,
    //        inet_ntoa(pin.sin_addr), ntohs(pin.sin_port), fd);

    socketState = SOCKET_ACCEPT;

} // LLRP_MntServer:socketAccept()


//=============================================================================
// socketDeliver

char tagInfo[2048];

unsigned char keepAlive[64] = { 0x04, 0x3E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00 };

unsigned char notifyy[256] = {0x04, 0x3F, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF6, 0x00, 0x16, 0x00, 0x80, 
					 0x00, 0x0C, 0x00, 0x04, 0xC6, 0x00, 0x5C, 0xE3, 0xA8, 0xB0, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00};


unsigned char response[128] = {0x04, 0x0D, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1F, 0x00, 0x0E, 0x00, 0x00,
					  0x00, 0x06, 0x53, 0x75, 0x63, 0x65, 0x73, 0x73};

//char keepAlive[64] = { 0x04, 0x3E, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00 };

template <class T, class P>
void
LLRP_MntServer::socketDeliver(P &aPool, int fd)
{
    unsigned int ii;
    T* ap;

	mpMutex->take(0);
	
    for ( ii = 0; ii < aPool.size(); ii++ )
    {
        if ( aPool[ ii ]->isIdle() )
        {
            // We found an idle agent
            ap = aPool[ ii ];
			DBG_PRINT(DEBUG_INFO, "Found an idle agent ID[%d]"NL, ap->getObjectID());
            break;
        }
    }

    if ( ii == aPool.size() )
    {
        // No idle agent found
        ap = new T;
        aPool.push_back( ap );
    }
	//mpMutex->give();
    // Now ap is a usable agent
    ap->setConnection( fd );
	ap->setRunning();
	mpMutex->give();
	ap->run();			// start the thread

    //socketState = SOCKET_LISTEN;
}


void
LLRP_MntServer::isocketDeliver()
{
    unsigned int ii;
    IAgent* ap;

	mpMutex->take(0);
    for ( ii = 0; ii < iagentPool.size(); ii++ )
    {
        if ( iagentPool[ ii ]->isIdle() )
        {
            // We found an idle agent
            ap = iagentPool[ ii ];
			DBG_PRINT(DEBUG_INFO, "Found an idle agent ID[%d]"NL, ap->getObjectID());
            break;
        }
    }

    if ( ii == iagentPool.size() )
    {
        // No idle agent found
        ap = new IAgent;
        iagentPool.push_back( ap );
    }
	//mpMutex->give();
    // Now ap is a usable agent
    ap->setConnection( iclientSocket );
	ap->setRunning();
	mpMutex->give();
	ap->run();			// start the thread

    //socketState = SOCKET_LISTEN;
}

#if 0
void
LLRP_MntServer::ssocketDeliver()
{
    unsigned int ii;
    SAgent* ap;

	mpMutex->take(0);
    for ( ii = 0; ii < sagentPool.size(); ii++ )
    {
        if ( sagentPool[ ii ]->isIdle() )
        {
            // We found an idle agent
            ap = sagentPool[ ii ];
			DBG_PRINT(DEBUG_INFO, "Found an idle agent ID[%d]"NL, ap->getObjectID());
            break;
        }
    }

    if ( ii == sagentPool.size() )
    {
        // No idle agent found
        ap = new IAgent;
        sagentPool.push_back( ap );
    }
	//mpMutex->give();
    // Now ap is a usable agent
    ap->setConnection( iclientSocket );
	ap->setRunning();
	mpMutex->give();
	ap->run();			// start the thread

    //socketState = SOCKET_LISTEN;
}
#endif

//=============================================================================
// complete
//
// Called by an AkMntAgent to report this disconnection to a
// maintenance terminal.
// The AkMntAgent object passes itself in the parameter.
// The AkMntAgent is put to a recycle pool for later usage.


//=============================================================================
// clearConnections
//
// The function calls "AkMntAgent::disconnect()"
// for all active instances of AkMntAgent.

void
LLRP_MntServer::clearConnections(void)
{

     socketState = SOCKET_IDLE;
 }

void
LLRP_MntServer::sendMessage(const char*  pcMessage)
{

	int  iResult = ::send( clientSocket, pcMessage, strlen(pcMessage), 0 );

	if ( iResult == SOCKET_ERROR )
	{
		DBG_PRINT(DEBUG_CRITICAL, "LLRP_MntServer:: sendMessage failed with error: %d"NL, iResult );

		closesocket(clientSocket);
	}
}

void
LLRP_MntServer::disconnectSocket(void)
{
	closesocket(clientSocket);
    socketState = SOCKET_IDLE;
}
//=============================================================================

EResultCode
LLRP_MntServer::recvAdvance (
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
#if 1 // linux and WIN32 use same select
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
            pDecoder = new CFrameDecoder(llrp_pTypeRegistry,
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
LLRP_MntServer::sendMessage (
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
LLRP_MntServer::sendReaderEventNotification(void)
{
	DBG_PRINT(DEBUG_INFO,"Notify"NL);

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
LLRP_MntServer::send_iReader_Failed_Notification(void)
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

EResultCode	
LLRP_MntServer::addROSpecToList( CMessage *  pMessage )
{
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
LLRP_MntServer::enableROSpec (llrp_u32_t ROSpecID )
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
LLRP_MntServer::startROSpec (llrp_u32_t ROSpecID )
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
			LLRP_ROSpecExecutor *pLLRP_ROSpecExecutor = new LLRP_ROSpecExecutor(this, *msg);
			pLLRP_ROSpecExecutor->run();
			return RC_OK;
		}
	}
	return RC_MiscError;
}

EResultCode
LLRP_MntServer::stopROSpec (llrp_u32_t ROSpecID )
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
LLRP_MntServer::stopAllROSpec (void )
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
LLRP_MntServer::deleteROSpec (llrp_u32_t ROSpecID )
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

// This function will be accessed by main cli thread to display the agent info
void 
LLRP_MntServer::showAgentStatus (void )
{
    unsigned int ii;
    LLRP_MntAgent* ap;

	// Need semaphore protection to access the agent pool

	// Semaphore protect to avoid the Server thread to change the Pool size
	DBG_PRINT(DEBUG_INFO, "Current Connected Agent Status:"NL);
	mpMutex->take(0);
    for ( ii = 0; ii < agentPool.size(); ii++ )
    {
        if ( false == (agentPool[ ii ]->isIdle()) )
        {
            // We found a non-idle agent
			char ipaddr[32];

            ap = agentPool[ ii ];
			ap->get_iReader_ipAddress((char *)ipaddr);
			DBG_PRINT(DEBUG_INFO, "LLRPMntAgent ID %d -> iReader IP %s"NL, ap->getObjectID(), ipaddr);
			ap->printAntennaList();
        }
    }
	mpMutex->give();
	// Release semaphore
}
