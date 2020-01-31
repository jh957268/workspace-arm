//=============================================================================
//
/// @file  AkMntAgent.h
///
/// Copyright (c) 2009 MTI Laboratory, Inc. USA
///
/// @brief  This file contains the declarations for AkMntAgent class
//
//=============================================================================

#ifndef LLRPMNTAGENT_H
#define LLRPMNTAGENT_H

//============================================================
// for Linux BSD Socket library
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//============================================================

#include "OwTask.h"
#include "llrp_ROSpecExecutor.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ltkcpp.h"

using namespace std;
using namespace LLRP;

#define IP_ADDR_STR_SIZE	32
#define ZERO_IP_ADDRESS		"0.0.0.0"
#define ZERO_IP_ADDRESS_LEN	sizeof(ZERO_IP_ADDRESS)

//=============================================================================
/// AkMntAgent handles the communication with a maintenance terminal.
/// It receives a connected socket from the AkMngServer and loops to
/// receive incoming messages until the socket is broken or closed.

class LLRP_MntAgent :
    public OwTask,
    public LLRP_Controller
{
public:
    /// Constructor
    ///
    		LLRP_MntAgent();

    /// Main program of the agent for a maintenance terminal.
    /// Inherited from OwTask
    ///
    void	main(OwTask*);

    /// Receive the socket so we can communicate to the client
    ///
    /// @param[in] sock The file descriptor of the socket
    void	setConnection( SOCKET  sock );

	void	set_iReader_ipAddress(char *ip_address, int len)
	{
		memmove((void *)m_iReader_ip, (const void *)ip_address, len);
		m_iReader_ip[len] = 0;
	}

	void	get_iReader_ipAddress(char *ip_address)
	{
		strncpy((char *)ip_address, (const char *)m_iReader_ip, IP_ADDR_STR_SIZE);
	}

	void	setRunning() 
	{
		m_Recv.bFrameValid = false;
		m_Recv.nBuffer = 0;
		m_pRcvMessage = 0;
		m_biReaderIsFailed = FALSE;
		m_biReaderConnectFailed = FALSE;
		iReaderHandle = 0;
		m_antCount = 0;
		iReaderHandle = 0;
		iKeepAliveSendCount = 0;
		set_iReader_ipAddress(ZERO_IP_ADDRESS, ZERO_IP_ADDRESS_LEN);
		idle = false;
	}


    /// Disconnect from the client
    ///
    void	disconnectSocket();

    /// Disconnect from the client
    ///
    void	disconnect();

    /// Returns true if this AkMntAgent is idle
    ///
    bool	isIdle() { return idle; }

	void	printAntennaList(void);

    /// Send a message to the client.  Inherited from AkController.
    ///
    /// @param[in] message Null terminated string of an outgoing message
    void	sendMessage( const char*  pcMessage );
	EResultCode	sendMessage( CMessage *  pMessage );

	EResultCode	addROSpecToList( CMessage *  pMessage );
	EResultCode enableROSpec (llrp_u32_t ROSpecID );
	EResultCode startROSpec (llrp_u32_t ROSpecID );
	EResultCode stopROSpec (llrp_u32_t ROSpecID );
	EResultCode deleteROSpec (llrp_u32_t ROSpecID );

	void send_GET_ROSPEC_RESPONSE(const CMessage *pMsg);
	void sendRO_Access_Report_OnDemand(void);

	void		decKeepAliveSendCount(void)
	{
		iKeepAliveSendCount--;
	}

	void		iReaderHasFailed (llrp_u32_t failedID)
	{
		m_uiFailedROSpecID = failedID;
		m_biReaderIsFailed = TRUE;
	}

	void		iReaderCannotConnect (void)
	{
		m_biReaderConnectFailed = TRUE;
	}

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

	void rospecCopy(CROSpec *src, CROSpec *dst);

	unsigned int getObjectID(void )
	{
		return id;
	}
	
private:
    /// Loop to receiver the next message unil the socket is colsed
    ///
    void	socketDeliver();

    SOCKET			clientSocket;	///< Socket of TCP connection to the client
    volatile bool	isSocketClosed;
    		bool	idle;			///< True if this agent is idle
    unsigned int	id;				///< ID of the agent
static unsigned int  nextId;		///< Next agent ID available

	UINT8	m_iReader_ip[IP_ADDR_STR_SIZE];	// iReader IP address

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

	CConnection::RecvState	m_Recv;

#define SEND_BUFFER_SIZE 8192
	CConnection::SendState  m_Send;

	CMessage	*m_pRcvMessage;
	bool		m_biReaderIsFailed;
	bool		m_biReaderConnectFailed;
	llrp_u32_t	m_uiFailedROSpecID;
	INT32		iKeepAliveSendCount;
	LLRP_ROSpecExecutor *mpLLRP_ROSpecExecutor;

	std::list<CMessage *>       m_list_ADD_ROSPEC;
};
//=============================================================================

#endif

//=============================================================================
