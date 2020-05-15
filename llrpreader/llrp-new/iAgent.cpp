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
#include "CSqlite.h"

#include <iostream>

using namespace std;

unsigned int IAgent::nextId = 0;

#define _time64		time

extern CSqlite *Sqlite_db;

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
	DBG_PRINT( DEBUG_INFO, "IAgent[%d]:: Closed"NL, id );
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
		case 0x93:
			iAgent_Sqlite_Select(hMsg);
			break;
		case 0x94:
			iAgent_Sqlite_Insert(hMsg);
			break;		
		default:
			break;
	}

	return 0;
}

int IAgent::iAgent_sendMsgObj(iMsgObj *hMsg)
{
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

#define NL "\n"
void IAgent::iAgent_ReadTagsRSSI(iMsgObj *hMsg)
{
	uint8_t buff [256] = {HDR1, HDR2, 0x00, 33, 0xff, 0xfd, 0x00, 0x00};
	int len;

	IReader *handle = IReader::getInstance();

	int ttagCount = 0;
	int antID = (hMsg->data[0] << 8) | hMsg->data[1] ;
	int pwr = (hMsg->data[2] << 8) | hMsg->data[3];    // e.g 2500

	printf("antid = %d, power = %d\n", antID, pwr);
	
	IReaderApiReadTagsMetaDataRSSI((void *)handle, antID, pwr, &ttagCount, (struct taginfo_rssi *)&buff[9]);

	buff[0] = HDR1;
	buff[1] = HDR2;
	buff[6] = hMsg->opCode;	
	buff[7] = antID;
	buff[8] = ttagCount;
	buff[3] = 3 + (ttagCount * TAGLEN_RSSI);
	buff[5] = ~buff[3];
	
	len = buff[3] + 6;

	::send( clientSocket, buff, len, 0 );

}

void IAgent::iAgent_SetTxPower(iMsgObj *hMsg)
{
	uint8_t buff [] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};

	int antid = hMsg->data[0];
	int pwr = (hMsg->data[1] << 8) | hMsg->data[2];    // e.g 2500

	// Console_Printf("ID = %d, pwr = %d\n", antid, pwr);	
	buff[6] = hMsg->opCode;
	//m_antpower[antid -1] = pwr;
	sendMessage(buff, 8);
}

void IAgent::iAgent_StartExecutor(iMsgObj *hMsg)
{
	uint8_t buff [] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};

	int flag = hMsg->data[0];
	int fd = (hMsg->data[1] << 8) | (hMsg->data[2]);
 	
	if (flag != 0)
	{
		IAgent_Executor::getInstance()->start_executor(((fd == DATABASE_MAGIC) || (fd == DATABASE_USER_MAGIC)) ? fd : clientSocket);
	}
	else
	{
		IAgent_Executor::getInstance()->stop_executor(((fd == DATABASE_MAGIC) || (fd == DATABASE_USER_MAGIC))? fd : clientSocket);
	}
	
	buff[6] = hMsg->opCode;

	sendMessage(buff, 8);     // should be 7

}

int IAgent::iAgent_CallBack(int fd, uint8_t *tReadBuf, int itReadCnt, int antID)
{
	int len;
	
	tReadBuf[6] = 0xAB;				// Aync Tag data indicator
	tReadBuf[7] = antID;
	tReadBuf[8] = itReadCnt;
	tReadBuf[3] = 3 + (itReadCnt * TAGLEN_RSSI);   // 3 is for data indicator, antID, itReadCnt
	tReadBuf[5] = ~tReadBuf[3];
	tReadBuf[0] = HDR1;
	tReadBuf[1] = HDR2;
	tReadBuf[2] = 0x00;								// This assumes the packet side is less then 256 bytes, i.e, < 10 tags
	tReadBuf[4] = 0xff;
	//if (cc3000_send_packet(tReadBuf, tReadBuf[3] + 6 ) == 0)
	//	return;
	//OSSleep(100);
	
	if ((DATABASE_MAGIC == fd) || (DATABASE_USER_MAGIC == fd))
	{
		// write data to Database
		Sqlite_db->begin_transaction();
		struct taginfo_rssi *pTaginfo_rssi;
		pTaginfo_rssi = (struct taginfo_rssi *)&tReadBuf[9];   
		for (int i = 0; i < itReadCnt; i++)
		{
			char pcbits[8], epcdata[32];
			short rssi;
			float frssi;

			sprintf(pcbits,"%02x %02x", u8(pTaginfo_rssi->tagid[0]), u8(pTaginfo_rssi->tagid[1]));

			sprintf(epcdata, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
									u8(pTaginfo_rssi->tagid[2]),
									u8(pTaginfo_rssi->tagid[3]),
									u8(pTaginfo_rssi->tagid[4]),
									u8(pTaginfo_rssi->tagid[5]),
									u8(pTaginfo_rssi->tagid[6]),
									u8(pTaginfo_rssi->tagid[7]),
									u8(pTaginfo_rssi->tagid[8]),
									u8(pTaginfo_rssi->tagid[9]),
									u8(pTaginfo_rssi->tagid[10]),
									u8(pTaginfo_rssi->tagid[11]),
									u8(pTaginfo_rssi->tagid[12]),
									u8(pTaginfo_rssi->tagid[13])
									);

			if (DATABASE_USER_MAGIC == fd)
			{
				// look up database and perform the action field
				int action = 0;   // no action
				Sqlite_db->user_tag_action(epcdata, antID, &action);
				
				// Now process the action according
			
			}
			else
			{
				rssi = pTaginfo_rssi->tagid[14];
				rssi = (rssi << 8) | ((pTaginfo_rssi->tagid[15]) & 0xff);
				frssi = (float)(rssi/10.0);
				Sqlite_db->insert_tag(epcdata, antID, frssi);
			}
		}
		Sqlite_db->commit();
		return itReadCnt;
	}

	len = tReadBuf[3] + 6;
	int iResult = ::send( fd, tReadBuf, len, 0 );
	
	if (iResult == len)
	{
		return itReadCnt;
	}
	DBG_PRINT(DEBUG_INFO, "IAgent Callback send fails with error: %d, fd = %d" NL, iResult, fd );
	return -1;
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

int 
IAgent::sendResponse(uint8_t resp_code, int msg_len, uint8_t *msg)
{
	uint8_t buff [256] = {HDR1, HDR2};
	int len = msg_len + 1;             // 1 is the response code
	
	buff[2] = (len >> 8) & 0xff;
	buff[3] = (len & 0xff);
	buff[4] = ~buff[2];
	buff[5] = ~buff[3];
	buff[6] = resp_code;
	memcpy(&buff[7], msg, msg_len);
	return (sendMessage(buff, len + 6));
}

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

void IAgent::iAgent_Sqlite_Select(iMsgObj *hMsg)
{
	int fd = clientSocket;
	int offset, limit, table;
	char sel_cl[24];

	uint8_t buff [32] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};
	
	limit = (hMsg->data[0] << 8) | hMsg->data[1];
	offset = (hMsg->data[2] << 8) | hMsg->data[3];
	table = hMsg->data[4];
	if (offset == 0 && limit == 0)
	{
		sprintf(sel_cl, "all");
	}
	else
	{
		sprintf(sel_cl, "limit %d offset %d", limit, offset );
	}

	buff[6] = hMsg->opCode;

	sendMessage(buff, 8);     // should be 7

	parm[0] = fd;
	parm[1] = table;
	Sqlite_db->select_tag(sel_cl, Sqlite_callback, (void *)parm, table);
	buff[3] = 5;
	buff[5] = 0xfa;
	sprintf((char *)&buff[7], "done");
	sendMessage(buff, 11);     // should be 11
}

void IAgent::iAgent_Sqlite_Insert(iMsgObj *hMsg)
{	
	int antid, action;
	char *insert_str;
	char *tmp1, *tmp2;
	char epc[32];
#define ERR_CODE	1	
	int error_code = ERR_CODE;
	

	uint8_t buff [32] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};

	insert_str = (char *)hMsg->data;
	
	do
	{
		tmp1 = strchr(insert_str, '=');
		if (tmp1 == 0)
		{
			// send error
			error_code = ERR_CODE;
			break;;
		}
		tmp1++;
		tmp2 = strchr(tmp1, '&');
		if (tmp2 == 0)
		{
			// send error
			break;;
		}
		*tmp2 = 0;
		sprintf(epc, "%s", tmp1);
	
		tmp2++;
		tmp1 = strchr(tmp2, '=');
		if (tmp1 == 0)
		{
			// send error
			break;
		}
		tmp1++;	
		tmp2 = strchr(tmp1, '&');
		if (tmp2 == 0)
		{
			// send error
			break;
		}
		*tmp2 = 0;
		antid = atoi(tmp1);
		tmp2++;

		tmp1 = strchr(tmp2, '=');
		if (tmp1 == 0)
		{
			// send error
			break;
		}
		tmp1++;	
		tmp2 = strchr(tmp1, '&');
		if (tmp2 == 0)
		{
			// send error
			break;
		}
		*tmp2 = 0;	
		action = atoi(tmp1);
		error_code = 0;
	} while (0);
	
	buff[6] = hMsg->opCode;
	
	if ( 0 == error_code)
	{
		error_code = Sqlite_db->insert_user_tag(epc, antid, action);
	}
	buff[7] = (uint8_t)error_code;
	sendMessage(buff, 8);     // should be 8
}

int IAgent::Sqlite_callback(void *param, int argc, char **argv, char **azColName)
{

	char buff[128] = {HDR1, HDR2};;
	// tag_id tag_val antid  rssi first_seen last_seen seen_cnt
	int table = ((int *)param)[1];
	if (table == 0)
	{
		sprintf(&buff[7], "%s~%s~%s~%s~%s~%s~%s", argv[0], argv[1], argv[3], argv[4], argv[5], argv[6], argv[7]);
	}
	else
	{
		char *action;
		if (*argv[4] == '0')
		{
			action = "No Action";
		}
		else if (*argv[4] == '1')
		{
			action = "Open Gate";
		}
		else
		{
			action = "Beep";
		}
		sprintf(&buff[7], "%s~%s~%s~%s~%s~%s~%s", argv[0], argv[1], argv[3], action, argv[5], argv[6], argv[7]);		
	}
	int len = strlen(&buff[7]);
	len = len + 1;
	buff[2] = (len >> 8) & 0xff;
	buff[3] = (len & 0xff);
	buff[4] = ~buff[2];
	buff[5] = ~buff[3];
	buff[6] = 0x93;

	//sprintf(&buff[7], "%s~%s~%s~%s~%s~%s~%s", argv[0], argv[1], argv[3], argv[4], argv[5], argv[6], argv[7]);

	//printf("%s\n", &buff[7]);

	int fd = ((int *)param)[0];

	::send( fd, buff, len + 6, 0 );

	return 0;
}
