//=============================================================================
///
/// @file  AkMntServer.h
///
/// Copyright (c) 2009 MTI Laboratory, Inc. USA
///
/// @brief  This file contains the declarations of AkMntServer object.
///
//=============================================================================

#ifndef	LLRPMNTSERVER_H
#define	LLRPMNTSERVER_H

//============================================================
// for Linux BSD Socket library
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//============================================================

#include <vector>
#include "OwTask.h"
#include "OwMutex.h"
#include "llrp_Controller.h"
#include "llrp_MntAgent.h"
#include "iAgent.h"
#include "sAgent.h"

#include "ltkcpp.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;
using namespace LLRP;

typedef sockaddr SOCKADDR;

///
/// This object establishes a TCP server for maintenance terminals.
/// It listen to the TCP port for connection from the client.  Once
/// connected, it hand over the connection to AkMntAgent, which
/// continues message exchanges with the client.
///
class LLRP_MntServer:
    public OwTask,
	public LLRP_Controller
{
public:
    /// Represents the state of socket connection
    ///
    enum SocketState
    {
        SOCKET_IDLE,
        SOCKET_INITIAL,
        SOCKET_CREATE,
        SOCKET_BIND,
        SOCKET_LISTEN,
        SOCKET_ACCEPT
    };

    /// The main program for the TCP server.  Inherited from OwTask.
    ///
    void	main(OwTask*);

    /// Get the instance of the object
    ///
    static LLRP_MntServer*	getInstance(UINT32 handle);
	static int				initRegistry(void);
  
    /// Requests all AkMntAgent to disconnect with maintenance
    /// terminals
    void	clearConnections(void);
	void	sendMessage( const char*  pcMessage);
	EResultCode	sendMessage( CMessage *  pMessage );

	EResultCode	addROSpecToList( CMessage *  pMessage );
	EResultCode enableROSpec (llrp_u32_t ROSpecID );
	EResultCode startROSpec (llrp_u32_t ROSpecID );
	EResultCode stopROSpec (llrp_u32_t ROSpecID );
	EResultCode deleteROSpec (llrp_u32_t ROSpecID );
	void		showAgentStatus(void);

	void		iReaderHasFailed (llrp_u32_t failedID)
	{
		m_uiFailedROSpecID = failedID;
		m_biReaderIsFailed = TRUE;
	}

	void	disconnectSocket(void);

	inline void
    setAntState (int antID, enum EANTSTATE eState)
	{
		m_antState[antID] = eState;
	}
	
	inline void
    setAntPower (int antID, UINT16 power)
	{
		m_antPower[antID] = power;
	}
	static const CTypeRegistry *  llrp_pTypeRegistry;

	// LLRP_MntServer();   testing calling the contructor, public member, used for  LLRP_MntServer abc object instantiation
	// LLRP_MntServer(UINT32 iReaderHandle);  testing calling contructor

private:
    /// Server port for maintenance terminal
    static const int SERVER_PORT = 5084;
    static const int ISERVER_PORT = 6084;
	static const int SSERVER_PORT = 9998;

    /// Private constructor
    ///
    //LLRP_MntServer();
	LLRP_MntServer(void * iReaderHandle);

    static LLRP_MntServer* spInstance; ///< Points to the instance

    /// Soecket operations
    void	socketInitialize();
    void	socketCreate();
    void	socketBind();
    void	socketListen();
    void	socketAccept();

	template <class T, class P>
    void	socketDeliver(P &pool, int fd);

    void	isocketDeliver();

	INT32	initialize(void);

	EResultCode recvAdvance(int nMaxMS,  time_t timeLimit);
	void		sendReaderEventNotification(void);
	void		send_iReader_Failed_Notification(void);
	void		stopAllROSpec(void);

	void		clearList_ADD_ROSPEC(void)
	{
		std::list<CMessage *>::iterator fmsg, lmsg;

		fmsg =  m_list_ADD_ROSPEC.begin();
		lmsg =  m_list_ADD_ROSPEC.end();
		CMessage * pMsg;

		while ((fmsg =  m_list_ADD_ROSPEC.begin()) != lmsg)
		{
			pMsg = *fmsg;
			m_list_ADD_ROSPEC.remove(pMsg);
			delete pMsg;
		}
#if 0
		for ( ; fmsg != lmsg ; fmsg++)
		{
			pMsg = *fmsg;
//			m_list_ADD_ROSPEC.remove(pMsg);
			delete pMsg;

		}

		if (fmsg != lmsg)
		{
			pMsg = *fmsg;
			m_list_ADD_ROSPEC.remove(pMsg);
			delete pMsg;
		}
		else
			return;

		fmsg =  m_list_ADD_ROSPEC.begin();
		lmsg =  m_list_ADD_ROSPEC.end();

		if (fmsg != lmsg)
		{
			pMsg = *fmsg;
			delete pMsg;
		}
		else
			return;
#endif

#if 0
		for (
			std::list<CMessage *>::iterator msg = m_list_ADD_ROSPEC.begin();
			msg != m_list_ADD_ROSPEC.end();
			msg++)
		{
			//delete *msg;
			m_list_ADD_ROSPEC.remove(*msg);
			// delete *msg;
		}
#endif
	}

    /// Socket Definition
    SocketState	socketState;
    	SOCKET	listenSocket;
    	SOCKET  ilistenSocket;
		SOCKET  slistenSocket;
    	SOCKET	clientSocket;
    	SOCKET	iclientSocket;
    	SOCKET	sclientSocket;

    struct sockaddr_in	saServer;
    struct sockaddr_in  pin;
    
    std::vector<LLRP_MntAgent*>	agentPool;
    std::vector<IAgent*>		iagentPool;
	std::vector<SAgent*>		sagentPool;

	// static const CTypeRegistry *  llrp_pTypeRegistry;

	char	m_rcvbuff[1024];

	int 	bill_out;

	CConnection::RecvState	m_Recv;

#define SEND_BUFFER_SIZE 8192
	CConnection::SendState  m_Send;

	CMessage	*m_pRcvMessage;
	bool		m_biReaderIsFailed;
	llrp_u32_t	m_uiFailedROSpecID;
	OwMutex		*mpMutex;

	std::list<CMessage *>       m_list_ADD_ROSPEC;

 };

//=============================================================================

#endif

//=============================================================================

