/*
 * CFile1.c
 *
 * Created: 12/24/2013 9:16:01 PM
 *  Author: jhong
 */ 
#include "iAgent.h"

#include  "iReaderapi.h"
#include  "debug_print.h"
#include  "iAgent_executor.h"

#include <iostream>

using namespace std;

unsigned int IAgent::nextId = 0;

#define _time64		time

//=============================================================================
// Constructor

IAgent::IAgent():
	OwTask( MEDIUM, 2048, "IAgent" ),
	isSocketClosed ( false ),
	id( ++nextId )
{
	DBG_PRINT( DEBUG_INFO, "IAgent[%d]:: Created"NL, id );
	idle = false;					// Assume it will be run right away to avoid race condition 
									// that the MntServer will use it again if another connection coming in
} // IAgent::IAgent()

extern uint8_t m_antpower[];

void
IAgent::main
(
	OwTask *
)
{
	time_t now;
	//char data[128];

	while (true)
	{
#if 0
		OwTask::sleep(1000);
        now = time(NULL);
        sprintf(data, "the server time is %ld\r\n\r\n",now);
        if (sendMessage(data) <= 0)
        {
        	break;
        }
#endif
#if 1
		if (iAgent_receiveMsgObj(&t_rxMsg) <= 0)
		{
			break;
		}
		// Now process the receive message
		iAgent_ProcessMsgObj(&t_rxMsg);
#endif
		
	}
	::close(clientSocket);
	idle = true;  // Once this is set to true, other connection (from LLRP_Mntserver) may use this object before the
	              // phtread exit if preemption "can" happen
}

int IAgent::iAgent_receiveMsgObj(iMsgObj *hMsg)
{
	uint8_t  soh;
	uint8_t  datalen[2];
	uint16_t xdatalen;
	int rcv_len, retval = 0;

	#define	GETCHAR(head) do {	 \
		retval = get_bytes(head, 1);		\
	} while (0)

	while(1)
	{
		GETCHAR(&soh);
		if (retval <= 0)
		{
			return (retval);
		}
		if(soh != HDR1)
		{
			continue;
		}
		GETCHAR(&soh);
		if (retval <= 0)
		{
			return (retval);
		}
		if (HDR2 != soh)
			continue;
			
		retval = get_bytes(datalen, 2);
		if (retval <= 0)
		{
			return (retval);
		}
		hMsg->dataLen = ((datalen[0] << 8) | datalen[1]);
		get_bytes(datalen, 2);
		if (retval <= 0)
		{
			return (retval);
		}
		xdatalen = ((datalen[0] << 8) | datalen[1]);
		xdatalen = ~xdatalen;
		if (hMsg->dataLen != xdatalen)
			continue;			
		break;
	}

	if ( MSG_MAX_DATA_LENGTH < hMsg->dataLen)
	{
		return (-1);
	}

	GETCHAR(&hMsg->opCode);
	if (retval <= 0)
	{
		return (retval);
	}
	if (0 == (hMsg->dataLen - 1))
	{
		// No need to read more
		return (1);
	}
	retval = get_bytes(&hMsg->data[0], hMsg->dataLen - 1);		// -1 because opcode is read above
	if (retval <= 0)
	{
		return (retval);
	}
	return (1);
}

int IAgent::iAgent_ProcessMsgObj(iMsgObj *hMsg)
{
	switch (hMsg->opCode)
	{
		case 0x81:
			iAgent_SetAntPort(hMsg);
			break;
		case 0x8b:
			iAgent_GetAntBitMap(hMsg);
			break;
		case 0x82:
			iAgent_ReadTags(hMsg);
			break;
		case 0x83:
			iAgent_ReadTagsRSSI(hMsg);
			break;			
		case 0x88:
			iAgent_SetTxPower(hMsg);
			break;
		case 0x87:
			iAgent_StartExecutor(hMsg);
			break;
		case 0x91:
			iAgent_GetRegion(hMsg);
			break;
		case 0x92:
			iAgent_GetSearchTimeout(hMsg);
			break;
		default:
			break;
	}

	return 0;
}

void IAgent::iAgent_SetAntPort(iMsgObj *hMsg)
{
	uint8_t buff [] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};

#if 0
	//Int32 ret = SetAntPort(hMsg->data[0]);
	Int32 ret = IReaderApiSelectAnt(hMsg->data[0] + 1);  // The DLL in host size has minus 1, add it back
	buff[6] = hMsg->opCode;
	buff[7] = (uint8_t)ret;
	cc3000_send_packet(buff, 8);	
#endif
}

void IAgent::iAgent_GetAntBitMap(iMsgObj *hMsg)
{
	uint8_t buff [264] = {HDR1, HDR2, 0x01, 1, 0xfe, 0xfd, 0x00, 0x00};
	int antCount, antList[64];
	int status = IREADER_SUCCESS;

	buff[5] = ~buff[3];	
	buff[4] = ~buff[2];
	buff[6] = hMsg->opCode;
	memset(&buff[7], 0x0, 256);

	IReader *handle = IReader::getInstance();

	status = IReaderApiGetAntList(handle, &antCount, antList);
	//antCount = 2;      for testing
	//antList[0] = 1;
	//antList[1] = 9;

	if (IREADER_SUCCESS == status)
	{
		for (int i = 0; i < antCount; i++)
		{
			// ant no start from 1
			buff[6 + antList[i]] = 1;
		}

	}
	sendMessage(buff, 263 );

}

void IAgent::iAgent_GetRegion(iMsgObj *hMsg)
{
	uint8_t buff [12] = {HDR1, HDR2, 0x00, 3, 0xff, 0xfd, 0x00, 0x00};

	int region = IReader::getInstance()->IReaderGetRegion();
	buff[5] = ~buff[3];
	buff[6] = hMsg->opCode;
	buff[7] = (region >> 8) & 0xff;   // upper byte first
	buff[8] = (region) & 0xff;
	sendMessage(buff, 9);
}

void IAgent::iAgent_GetSearchTimeout(iMsgObj *hMsg)
{
	uint8_t buff [12] = {HDR1, HDR2, 0x00, 3, 0xff, 0xfd, 0x00, 0x00};

	int timeout = IReader::getInstance()->IReaderGetSearchTimeout();
	buff[5] = ~buff[3];
	buff[6] = hMsg->opCode;
	buff[7] = (timeout >> 8) & 0xff;   // upper byte first
	buff[8] = (timeout) & 0xff;
	sendMessage(buff, 9);
}

void IAgent::iAgent_ReadTags(iMsgObj *hMsg)
{
	uint8_t buff [256] = {HDR1, HDR2, 0x00, 33, 0xff, 0xfd, 0x00, 0x00};

#if 0
	int ttagCount = 0;
	int antID = hMsg->data[0];
	
	IReaderApiReadTagsMetaDataRSSI(antID, DEFAULT_TX_POWER, &ttagCount, (struct taginfo_rssi *)&buff[9]);
	buff[6] = hMsg->opCode;	
	buff[7] = antID;
	buff[8] = ttagCount;
	buff[3] = 3 + (ttagCount * TAGLEN_RSSI);
	buff[5] = ~buff[3];
	cc3000_send_packet(buff, buff[3] + 6 );
#endif
}

void IAgent::iAgent_ReadTagsRSSI(iMsgObj *hMsg)
{
	uint8_t buff [256] = {HDR1, HDR2, 0x00, 33, 0xff, 0xfd, 0x00, 0x00};

	IReader *handle = IReader::getInstance();

	int ttagCount = 0;
	int antID = hMsg->data[0];
	int pwr = hMsg->data[1];
	
	IReaderApiReadTagsMetaDataRSSI((void *)handle, antID, pwr, &ttagCount, (struct taginfo_rssi *)&buff[9]);
	buff[6] = hMsg->opCode;	
	buff[7] = antID;
	buff[8] = ttagCount;
	buff[3] = 3 + (ttagCount * TAGLEN_RSSI);
	buff[5] = ~buff[3];
	//c3000_send_packet(buff, buff[3] + 6 );

}

void IAgent::iAgent_SetTxPower(iMsgObj *hMsg)
{
	uint8_t buff [] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};
#if 0
	int antid = hMsg->data[0];
	int pwr = hMsg->data[1];

	Console_Printf("ID = %d, pwr = %d\n", antid, pwr);	
	buff[6] = hMsg->opCode;
	m_antpower[antid -1] = pwr;
	cc3000_send_packet(buff, buff[3] + 6 );
#endif
}

void IAgent::iAgent_StartExecutor(iMsgObj *hMsg)
{
	uint8_t buff [] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};

	int flag = hMsg->data[0];
	
	if (flag != 0)
	{
		IAgent_Executor::getInstance()->start_executor(clientSocket);
	}
	else
	{
		IAgent_Executor::getInstance()->stop_executor(clientSocket);
	}
	
	buff[6] = hMsg->opCode;

	sendMessage(buff, 8);     // should be 7

}

void IAgent::iAgent_CallBack(uint8_t *tReadBuf, int itReadCnt, int antID)
{
#if 0
	tReadBuf[6] = 0x88;
	tReadBuf[7] = antID;
	tReadBuf[8] = itReadCnt;
	tReadBuf[3] = 3 + (itReadCnt * TAGLEN_RSSI);
	tReadBuf[5] = ~tReadBuf[3];
	tReadBuf[0] = HDR1;
	tReadBuf[1] = HDR2;
	tReadBuf[2] = 0x00;
	tReadBuf[4] = 0xff;
	//if (cc3000_send_packet(tReadBuf, tReadBuf[3] + 6 ) == 0)
	//	return;
	//OSSleep(100);
    cc3000_send_packet(tReadBuf, tReadBuf[3] + 6 );
#endif
}

int
IAgent::sendMessage
(
	const char*  message
)
{
	int  iResult = ::send( clientSocket, message, strlen(message), 0 );

	if ( iResult == SOCKET_ERROR )
	{
		DBG_PRINT(DEBUG_INFO, "IAgent[%d]:: sendMessage failed with error: %d"NL, id, iResult );

		disconnect();
	}
	return (iResult);
//	SPrintf( "AkMntAgent[%d]:: Msg sent"NL, id );

} // IAgent::sendMessage()

int
IAgent::sendMessage
(
	uint8_t*  buff,
	int 	  len
)
{
	int  iResult = ::send( clientSocket, buff, len, 0 );

	if ( iResult == SOCKET_ERROR )
	{
		DBG_PRINT(DEBUG_INFO, "IAgent[%d]:: sendMessage failed with error: %d"NL, id, iResult );

		return (SOCKET_ERROR);
	}
	return (iResult);
//	SPrintf( "AkMntAgent[%d]:: Msg sent"NL, id );

} // IAgent::sendMessage()

void
IAgent::disconnect
(
)
{
	// Close TCP socket
	// Linux environment

	isSocketClosed = true;
	::close(clientSocket);
	DBG_PRINT(DEBUG_INFO, "IAgent[%d]:: close socket"NL, id );

	// Tell the controller not to send more indication to here anymore
	//AkMsgProcessor::getInstance()->unsetController( this );

	// Tell AkRec that there is one less controller now
	//AkRec::getInstance()->decControllerCount();

	// inform the AkMntServer that it is going idle
	//AkMntServer::getInstance()->complete( this );

} // IAgent::disconnect()

//=============================================================================
// setConnection
// This function is call by "AkMntServer" to pass down the file
// descriptor of a newly established TCP socket. It also gives a
// semaphore to release its pending thread.

void
IAgent::setConnection
(
	SOCKET  connectClient
)
{
	DBG_PRINT(DEBUG_INFO, "IAgent[%d]:: Socket %d assigned"NL, id, connectClient );

	clientSocket = connectClient;
	// sem.give();				 // Release the thread

} // IAgent::setConnection()

int IAgent::get_bytes(uint8_t *buff, int req_len)
{
	int result;
	int remain = req_len;

	if (remain == 0)
	{
		return 1;   // XXX, should not call this if req_len is 0, now this is workaround
	}
	while (remain)
	{
		result = ::recv(clientSocket, (char *)&buff[req_len - remain], remain, 0);

		// printf("recv result = %d\n", result);
		// since it is blocking socket, it returns 0 if remote close the socket, send FIN, or -1 if error
		if (result <= 0)
		{
			return result;
		}
		if (result == remain)
		{
			break;
		}
		remain -= result;
	}
	return (req_len);
}
