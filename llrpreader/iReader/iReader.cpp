#include "iReader.h"
#include "iReadererror.h"
#include "muxserial.h"

IReader* IReader::spInstance = 0;



#define	IF_ERROR_RETURN(errcode) do {		\
	if ((errcode) != CRC_OK)				\
	{										\
		if ((errcode) == 0)					\
	   		return(IREADER_TIMEOUT_1); 		\
		else if ((errcode) == SOCKET_ERROR)	\
	   		return(IREADER_SOCKET_ERROR); 	\
		else								\
			return ((errcode));				\
	}										\
  } while (0)

#if 0
IReader::IReader(char *server_ip)
{
    memset((void *)m_antlist, 0x00, sizeof(m_antlist));
}
#endif

IReader::~IReader()
{
}

IReader
*IReader::getInstance
(
	void
)
{
	if ( 0 == spInstance )
	{
		spInstance = new IReader("0.0.0.0");
	}

	return( spInstance );

} // LLRP_MntServer:getInstance()

int  IReader::IReaderInit(char *ipaddr, int region)
{
    int i, error;
    UInt32 initVal;
#define INIT_MAGIC      0xAB    

	for(i = 0; i < MAXANT; i++)
		m_antpower[i] = 2500;

    return (IREADER_SUCCESS);
}

int  IReader::IReaderConnect(void)
{
    int ret;
    
    ret = Connect();
    return (ret);
}

int  IReader::IReaderDisconnect(void)
{
    Disconnect();
    return (IREADER_SUCCESS);
}

int  IReader::IReaderSetRegion(int region)
{
    int error;
    
    m_region = region;
    error = setregion(region);
    IF_ERROR_RETURN(error);
    return (IREADER_SUCCESS);
}

int  IReader::IReaderSetPowerLevel(int antid, int pwr, int doset)
{
    Int32 error;

    if (doset)
    {
	    //error = setpower(m_antpower[antid]);
	    error = setpower(pwr);
		IF_ERROR_RETURN(error);
        return (IREADER_SUCCESS);
    }
    m_antpower[antid] = pwr;
    return (IREADER_SUCCESS);
}

int  IReader::IReaderSetWritePowerLevel(int pwr)
{
    Int32 error;

    error = setwritepower(pwr);
    IF_ERROR_RETURN(error);
    return (IREADER_SUCCESS);
}

int  IReader::IReaderTagSearchTimeout(int timeout)
{
    m_tagserachtimeout = timeout;
    return (IREADER_SUCCESS);
}

int  IReader::IReaderGetPowerLevel(int antid, int *pwr)
{
    *pwr = m_antpower[antid];
    return (IREADER_SUCCESS);
}

int  IReader::IReaderReadTags(int *tagcount, struct taginfo *tagrbuf)
{
	unsigned char  buf[] = {0xff, 0x4, 0x22, 0, 0, 0x0, 0x50, 0, 0, 0, 0, 0, 0};
	u16   i,j, tags = 0;
	int blk;
    Int32 error;
    // int retval;
    // static int antseted = 0;

    buf[5] = m_tagserachtimeout/256;
    buf[6] = m_tagserachtimeout%256;
    *tagcount = 0;
#if 0
    if (antseted == 0)
    {
	SelectModule(MUX_MODULE);

	if ((error = SetAntPort(antid)) != 0)
	{
	    SelectModule(READER_MODULE);
		return error;
	}
	SelectModule(READER_MODULE);
    // antseted = 1;
    }
#endif
	// IF_ERROR_RETURN(setpower(m_antpower[antid]));
	// error = setpower(m_antpower[antid]);
	// IF_ERROR_RETURN(error);
    error = clrtagbuf();
	IF_ERROR_RETURN(error);
#if 0
	if(((retval = sendmsg(buf)) == 0))
		return (IREADER_TIMEOUT1);
    else if (retval == SOCKET_ERROR)
		return (IREADER_SOCKET_ERROR);
    IF_ERROR_RETURN(sendmsg(buf));
	if(((retval = sendmsg(buf)) == 0) || (retval == SOCKET_ERROR))
		return (retval);
#endif  
	error = sendmsg(buf);
	IF_ERROR_RETURN(error);
	if (m_rxMsg.dataLen == 0)
		return IREADER_SUCCESS;

	tags = m_rxMsg.data[0];

	buf[1] = 2;
	buf[2] = 0x29;
	buf[3] = 0;

	blk = tags/13;
	if(blk*13 < tags)
		blk++;

	for (i = 0; i < blk; i++)
	{
		if((i+1) < blk)
			buf[4] = 13;
		else
			buf[4] = tags - i*13;
#if 0
	    if(((retval = sendmsg(buf)) == 0) || (retval == SOCKET_ERROR))
		    return (retval);
#endif
        error = sendmsg(buf);
		IF_ERROR_RETURN(error);
		if (m_rxMsg.dataLen)
		{
			for(j = 0; j < (m_rxMsg.dataLen)/18; j++)
			{
				memcpy(tagrbuf[j+i*13].tagid,&m_rxMsg.data[4+j*18], TAGLEN);
				if((j+i*13) >= TAGBUFLEN)
				{	
					error = clrtagbuf();
					IF_ERROR_RETURN(error);
					*tagcount = TAGBUFLEN; 
					return (IREADER_SUCCESS);
				}
			}
		}	
	}
	// IF_ERROR_RETURN((clrtagbuf()));
	*tagcount = tags; 
	return (IREADER_SUCCESS);
}

int  IReader::IReaderReadTagsMetaDataRSSI(int *tagcount, struct taginfo_rssi *tagrbuf)
{
	unsigned char  buf[] = {HDR1, HDR2, 0x00, 0x0A, 0x82, 0x00, 0x00, 0x00, 0x0D, 0x0A};
	u16   j; 
    int totaltags, tags, tcount;
    char *ptagdata;
    int taglen;
    
    struct taginfo_rssi *pTaginfo;
	
    Int32 error;

    buf[2] = 0x00;
    buf[3] = 0x0A;
    buf[4] = 0x90;			// Inventory by timing

    buf[5] = m_tagserachtimeout/256;
    buf[6] = m_tagserachtimeout%256;

    *tagcount = 0;
    // error = clrtagbuf();
	// IF_ERROR_RETURN(error);

	error = sendmsg(buf);
	IF_ERROR_RETURN(error);

	totaltags = m_rxMsg.data[3];
    if (totaltags > MAX_TAGS_PER_ANTENNA)
        return (IREADER_INVALID);

	if ((m_rxMsg.dataLen) == 0 || (totaltags == 0))
		return IREADER_SUCCESS;

    tags = 0;

	buf[3] = 0x08;
	buf[4] = 0x92;      // Get tags info from buffer
    buf[5] = 0x0;       // Preset CRC to 0
	buf[6] = 0x0D;
	buf[7] = 0x0A;

	error = sendmsg(buf);
	IF_ERROR_RETURN(error);

	/* Get the first tag */
	memmove(tagrbuf[tags].tagid, m_rxMsg.data, TAGLEN_RSSI);
	tags = 1;
    while (tags < totaltags)
    {
		error = MSG_receiveMsgObj(&m_rxMsg);
		IF_ERROR_RETURN(error);
		memmove(tagrbuf[tags].tagid, m_rxMsg.data, TAGLEN_RSSI);
		tags ++;
	}

	// IF_ERROR_RETURN((clrtagbuf()));
	*tagcount = tags; 
	return (IREADER_SUCCESS);
}

int  IReader::IReaderGetTagCount(int *tagcount)
{
	unsigned char  buf[] = {0xff, 0x4, 0x22, 0, 0, 0x0, 0x50, 0, 0, 0, 0, 0, 0};
	u16  tags = 0;

    Int32 error;
    // int retval;
    // static int antseted = 0;

    buf[5] = m_tagserachtimeout/256;
    buf[6] = m_tagserachtimeout%256;
    *tagcount = 0;
    error = clrtagbuf();
	IF_ERROR_RETURN(error);

	error = sendmsg(buf);
	IF_ERROR_RETURN(error);
	if (m_rxMsg.dataLen == 0)
		return IREADER_SUCCESS;

	tags = m_rxMsg.data[0];

	*tagcount = tags; 
	return (IREADER_SUCCESS);
}

int  IReader::IReaderWriteTag(int timeout, unsigned char *tagid)
{
	unsigned char  buf[32] = {0xff, 0x10, 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Int32 error;

    buf[3] = timeout/256;
    buf[4] = timeout%256;

    memmove ((void *)&buf[7], (const void *)tagid, TAGLEN);
	error = sendmsg(buf);
	IF_ERROR_RETURN(error);
    //if ( (0 != m_rxMsg.status[0]) || (0 != m_rxMsg.status[1]))
    //    return ( IREADER_WRITE_TAG_FAIL );
    return (IREADER_SUCCESS);
}

int  IReader::IReaderReadTagsAll(void)
{
	return (IREADER_SUCCESS);
}

int  IReader::IReaderGetAntList(int *antCount, int *antList)
{
    int i, error;

    
    *antCount = 0;
    error = IReaderGetAntMap();
    if (error)
        return (error);    
    *antCount = m_antcount;
    for (i = 0; i < m_antcount; i++)
        antList[i] = m_antlist[i];
    return (IREADER_SUCCESS);
}

int  IReader::IReaderGetAntMap(void)
{
	int m, error;
    uint8_t map[40];
    // retrieve the list from mux again

	m_antcount = 0;
	// SelectModule(MUX_MODULE);
    error = GetAntMap(map);
    if (error)
        return (error);
	// SelectModule(READER_MODULE);
	for (m = 1; m <= 32*8; m++)
	{		
		if (map[31- (m - 1)/8] & (1 <<(m - 1)%8))
		{    
			m_antlist[m_antcount] = m;
			m_antcount++;
		}
	}
    return (IREADER_SUCCESS);
}

int  IReader::IReaderRescanSlave(Int32 chn)
{
    // retrieve the list from mux again
    Int32 error;
    
    error = RescanSlave(chn);
    return (error);
}

int  IReader::IReaderCreateMutex(void)
{
	m_hMutex = 	new OwMutex();
    return ( IREADER_SUCCESS );
}

int  IReader::IReaderCloseMutex(void)
{
    delete(m_hMutex);
    return ( IREADER_SUCCESS );
}

int  IReader::IReaderTakeMutex(void)
{
	int error = IREADER_SUCCESS;

	m_hMutex->take( PI_FOREVER );

	return (error);
}

int  IReader::IReaderGiveMutex(void)
{
	m_hMutex->give();

    return ( IREADER_SUCCESS );
}


int  IReader::StartApp(void)
{
	MsgObj msg;

#define MSG_OPCODE_BOOT_FIRMWARE       0x04

	msg.opCode   = MSG_OPCODE_BOOT_FIRMWARE;
	msg.dataLen  = 0;

	MSG_sendMsgObj(&msg);
	OwTask::sleep(600);
	return MSG_receiveMsgObj(&msg);
}

void IReader::docrc(unsigned char *buf, int len)
{
	u8   i, crcReg;

	crcReg = 0;
	for (i = 1; i < len - 5; i++)
		crcReg ^= buf[i + 2];
	buf[len - 3] = crcReg ;
}

int IReader::MSG_sendMsgObj(MsgObj *hMsg)
{
  u16   crcReg;
  char  buf[MSG_MAX_PACKET_LEN];
  u8    i;
  u8    idx = 0;

//	MuxApiDevSelectModule(muxh,READER_MODULE);

  // Calculate the CRC
  crcReg = MSG_calcCrcToDsp(hMsg);

  // Set up SOH in buffer
  buf[idx++] = (char)0xFF;

  // Send data length
  buf[idx++] = hMsg->dataLen & 0xFF;

  // Send OpCode
  buf[idx++] = hMsg->opCode;

  // Send Data
  for(i=0; i<hMsg->dataLen; i++)
  {
    buf[idx++] = hMsg->data[i] & 0xFF;
  }

  // Send CRC
  buf[idx++] = crcReg >> 8;
  buf[idx++] = crcReg & 0xFF;

  // Send the data to the serial port
 
  return (sendArray((unsigned char *)buf, idx));
  //mySD.SendData(buf, idx);
}

int IReader::MSG_receiveMsgObj(MsgObj *hMsg)
{
  s32 bytesRead;
  u8  soh;
  u8  i;
  u8  crc;
  bool retVal;
  u8  datalen[2];

  #define	GETCHAR(head) do {						\
	  int val; \
	if ((val = Ser2->getChar((head))) != 1)					\
	   return(val); \
  } while (0)

//	MuxApiDevSelectModule(muxh,READER_MODULE);
	while(1)
	{
		GETCHAR(&soh);
		bytesRead = 1;
    //bytesRead = mySD.ReadData(&soh, 1);
		soh &= 0xFF;

		if((bytesRead == 1) && (soh != HDR1))
		{
			continue;
		}
		GETCHAR(&soh);
		if (HDR2 != soh)
			continue;
		break;
	}

	crc = 0x00;

	GETCHAR(&datalen[0]);
 	GETCHAR(&datalen[1]);
	hMsg->dataLen = ((datalen[0] << 8) | datalen[1]) - 8;

	if ( MSG_MAX_DATA_LENGTH < hMsg->dataLen)
	{
		return (JUMBO_DATA_LEN);
	}
	crc = (datalen[0] ^ datalen[1]);

  	GETCHAR(&hMsg->opCode);
	crc ^= (hMsg->opCode);

  	for(i=0; i<hMsg->dataLen; i++)
  	{
		GETCHAR(&hMsg->data[i]);
		crc ^= (hMsg->data[i]);

    	//while(mySD.ReadData(&hMsg->data[i], 1) == 0);
  	}
	GETCHAR(&hMsg->crc);
	GETCHAR(&hMsg->cr);
	GETCHAR(&hMsg->lf);
	return (crc == hMsg->crc);
}


int IReader::sendmsg(unsigned char *buf)
{
	int len, retv, ntry;;

	len = ((buf[2] << 8) | buf[3]);
	docrc(buf, len);
    for (ntry = 0; ntry < 3; ntry++)
    {
    	Ser2->clearRcv();
        Ser2->sendArray((unsigned char *)buf, len);

	    retv = MSG_receiveMsgObj(&m_rxMsg);
        if (retv == SOCKET_ERROR)
            return (SOCKET_ERROR);
        else if (retv == 0)
            continue;
	    return (retv);
    }
	return (0);
}

int IReader::get_boot_firmware_ver(void)
{
	unsigned char buf[] = {HDR1, HDR2, 0x0, 0x08, 0x02, 0, 0x0D, 0x0A};
	return (sendmsg(buf));
}

int IReader::setpower1(void)
{
	unsigned char buf[] = {0xff, 0x1, 0x98, 1, 0, 0};
	return (sendmsg(buf));
}

int IReader::setregion0(void)
{
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x0A, 0x2C, 0x01, 0x08, 0, 0x0D, 0x0A};	 // CRC is 0x2F
	return (sendmsg(buf));
}

int IReader::setregion(int region)
{
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x0A, 0x2C, 0x00, region, 0, 0x0D, 0x0A};

	return sendmsg(buf);
}

int IReader::setprotocol(void)
{
	unsigned char buf[] = {0xff, 0x2, 0x93, 0, 0x5, 0,0};
	return sendmsg(buf);
}

int IReader::setantport(void)
{
	unsigned char buf[] = {0xff, 0x2, 0x91, 0x1, 0x1, 0,0};  // use antenna porrt 0
	// unsigned char buf[] = {0xff, 0x2, 0x91, 0x2, 0x2, 0,0};
	return sendmsg(buf);
}

int IReader::poweroff(void)
{
	unsigned char buf[] = {0xff, 0x2, 0x92, 0, 0, 0, 0};
	return sendmsg(buf);
}

int IReader::poweron(void)
{
	unsigned char buf[] = {0xff, 0x2, 0x92, 0x9, 0xc4, 0, 0};
	return sendmsg(buf);
}

int IReader::setpower(int power)
{
	int result;

	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x0E, RFID_CMD_SET_TXPOWER, 0x00, 0x00, power/256, power%256, 
							power/256, power%256, 0x00, 0x0D, 0x0A};

	result = sendmsg(buf);
	if ( CRC_OK != result )			// 1 is success, otherwise fail
		return (result);
	if ( (m_rxMsg.opCode != RFID_REPLY_SET_TXPOWER) || (m_rxMsg.data[0] != RFID_CMD_SUCCESS))
		return ( IREADER_SET_POWER_FAIL );
	return (result);

}

int IReader::setwritepower(int power)
{
	unsigned char buf[] = {0xff, 0x2, 0x94, power/256, power%256, 0, 0};

	return sendmsg(buf);
}

int IReader::clrtagbuf(void)
{
	unsigned char buf[] = {0xff, 0, 0x2a, 0, 0};
	return sendmsg(buf);
}

int IReader::ledon(void)
{
	unsigned char buf1[] = {0xff, 0x2, 0x96, 1, 0,0,0};
	return sendmsg(buf1);
}

int IReader::ledoff(void)
{
	unsigned char buf1[] = {0xff, 0x2, 0x96, 1, 1,0,0};
	return sendmsg(buf1);
}

void IReader::setbaud(void)
{
	unsigned char buf1[] = {0xff, 0x4, 0x6, 0, 1,0xc2,0,0,0};
	sendmsg(buf1);
}

///////////////////////////////////////////////////////////////////////////

#if 0
#define STATE_DONE  99
static char screen_buffer[4096];
static int tot_len;
static char newbaud[32];

int string_match(char *str);
int ScreenStateMachine(SOCKET so, int *state);

int string_match(char *str)
{
	int idx, i, len;
	int match = 0;
	len = strlen(str);
	for (i = 0; i < 6; i++)
	{
		idx = tot_len - len - i;
		if (idx < 0)
			break;
		if (!memcmp(&screen_buffer[idx], str, len))
		{
			match = 1;
			break;
		}
	}
	return (match);
}

int ScreenStateMachine(SOCKET so, int *state)
{
	int change = 0;
	char snd_buffer[32];

	switch (*state)
	{
	case 0:
		if (string_match("Mode") == 1)
		{
			snd_buffer[0] = 0xd;
			snd_buffer[1] = 0x0;
			send(so, snd_buffer, 2, 0); 
			*state = 1;
			change = 1;
		}
		break;
	case 1:
		if (string_match("?") == 1)
		{
			snd_buffer[0] = '1';
			snd_buffer[1] = 0xd;
			snd_buffer[2] = 0x0;
			send(so, snd_buffer, 3, 0); 
			*state = 2;
			change = 1;
		}
		break;
	case 2:
		if (string_match("?") == 1)
		{
			sprintf_s(snd_buffer, "%s\r", newbaud); 
			send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
			*state = 3;
			change = 1;
		}
		break;
	case 3:                         /* IF mode */
	case 4:                         /* FLOW */
	case 5:                         /* port No */
	case 6:                         /* Connect mode */
	case 7:                         /* Modem mode */
	case 8:                         /* ring mode */
	case 9:                         /* Inc source port */
	case 14:                         /* remote port */
	case 15:                         /* Disconn mode */
	case 16:                         /* flush mode */
	case 17:                         /* Disconn time */
	case 19:                         /* Send char 1 */
	case 20:                         /* Send char 2 */
		if (string_match("?") == 1)
		{
			sprintf_s(snd_buffer, "\r"); 
			send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
			*state += 1;
			change = 1;
		}
		break;
	case 10:                         /* remote IP */
	case 11:                         /* remote IP */
	case 12:                         /* remote IP */
	case 13:                         /* remote IP */
		if (string_match("000") == 1)
		{
			sprintf_s(snd_buffer, "\r"); 
			send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
			*state += 1;
			change = 1;
		}
		break;
	case 18:                         /* colon */
		if (string_match(":") == 1)
		{
			sprintf_s(snd_buffer, "\r"); 
			send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
			*state += 1;
			change = 1;
		}
		break;
	case 21:                         /* about to save and exit */
		if (string_match("?") == 1)
		{
			sprintf_s(snd_buffer, "9\r"); 
			send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
			*state = STATE_DONE;
			change = 1;
		}
		break;
	default:
		break;
	}
	return (change);
}
#endif

/* return 0 if success, -1 otherwise */
#if 0
int set_lantronic_baudrate (char *serv_ip, int baudrate)
{
	WSADATA 			wsaData;
	unsigned long addr;
	char cac_buffer[512];
    unsigned char send_cmd[] = {0x00, 0x00, 0x00, 0xFC, 0x53, 0x45, 0x54, 0x2D, 0x42, 0x41, 0x55, 0x44, 0x00, 0x02};
	SOCKET sock;
	struct sockaddr_in 	serv;
	int result;
	struct timeval tv;
	fd_set readset;
	int num_try;

    if (baudrate == 9600)
        send_cmd[13] = 0x02;
    else
        send_cmd[13] = 0x08;
	addr = inet_addr(serv_ip);

	memset(&serv,0,sizeof(serv));
	serv.sin_addr.s_addr = addr;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(0x77FE);

#if 1
	if ((result = WSAStartup(0x202,&wsaData)) != 0) {
		WSACleanup();
		return -1;
	}
#endif
	sock = socket(AF_INET,SOCK_DGRAM,0); /* Open a socket */
	if (sock == INVALID_SOCKET ) {
        WSACleanup();
		return -1;
	}
	if (connect(sock,(struct sockaddr*)&serv,sizeof(serv))
		== SOCKET_ERROR) {
            closesocket(sock);
			WSACleanup();
			return -1;
	}
	tv.tv_usec = 0;
	tv.tv_sec = 1;
	for (num_try = 0; num_try < 3; num_try++)
    {
        if (send(sock, (char *)send_cmd, 14, 0) != 14)
	    {
	        // closesocket(socket_handle1);
            closesocket(sock);
	        WSACleanup();
	        return (-1);
	    }
		FD_ZERO(&readset);
		FD_SET(sock, &readset);
		result = select(sock + 1, &readset, NULL, NULL, &tv);
		if (result == 0)
			continue;
	    result = recv(sock, (char *)cac_buffer, 128, 0);
        if (result == SOCKET_ERROR)
        {
            closesocket(sock);
	        WSACleanup();
	        return (-1);
        }
        break;
    }
	closesocket(sock);
	WSACleanup();
	return 0;
}
#endif

#if 0
int set_lantronic_baudrate (char *serv_ip, int baudrate)
{
	WSADATA 			wsaData;
	unsigned long addr;
	char cac_buffer[512];
	SOCKET sock;
	int retval, rcv_len;
	struct sockaddr_in 	serv;
	int result, state, change;
	struct timeval tv;
	fd_set readset;

	addr = inet_addr(serv_ip);
	sprintf_s(newbaud, "%d", baudrate); 

	memset(&serv,0,sizeof(serv));
	serv.sin_addr.s_addr = addr;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(9999);

#if 1
	if ((retval = WSAStartup(0x202,&wsaData)) != 0) {
		WSACleanup();
		return -1;
	}
#endif
	sock = socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
	if (connect(sock,(struct sockaddr*)&serv,sizeof(serv))
		== SOCKET_ERROR) {
			WSACleanup();
			return -1;
	}
	rcv_len = 0;
	screen_buffer[0] = 0;
	tot_len = 0;
	state = 0;

	while (1)
	{
		FD_ZERO(&readset);
		FD_SET(sock, &readset);
		tv.tv_sec = 1;  /* 1 Secs Timeout */
		tv.tv_usec = 0;
		result = select(sock + 1, &readset, NULL, NULL, &tv);
		if (result > 0) 
		{
			result = recv(sock, (char *)cac_buffer, 512, 0);
			if (result == 0 || result == SOCKET_ERROR) 
			{
				/* This means the other side closed the socket */
				// closesocket(socket_handle);
				return (-1);
			}
			else 
			{
				/* I leave this part to your own implementation */
				rcv_len = result;
				memmove(&screen_buffer[tot_len], cac_buffer, rcv_len);

				tot_len += rcv_len;
				screen_buffer[tot_len] = 0;
				change = ScreenStateMachine(sock, &state);
				if (state == STATE_DONE)
				{
					break;
				}
				if (change)
				{
					tot_len = 0;
					screen_buffer[0] = 0;
				}
			}
		}
		else
		{
			/* end of data, before save and exit */
			return (-1);
		}
	} /* while (1) */

	closesocket(sock);
	WSACleanup();
	return 0;

}
#endif

#if 0
void telnetStateMachine(SOCKET so, int *state)
{
	int change = 0;
	unsigned char snd_buffer[32];

	switch (*state)
    {
        case 0:
			snd_buffer[0] = 0xFF;
			snd_buffer[1] = 0xFC;
			snd_buffer[2] = 0x01;
			send(so, (const char *)snd_buffer, 3, 0); 
			*state = 1;
			change = 1;
            break;
        case 1:
			snd_buffer[0] = 0xFF;
			snd_buffer[1] = 0xFD;
			snd_buffer[2] = 0x2c;
			send(so, (const char *)snd_buffer, 3, 0); 
			*state = 2;
			change = 1;
            break;
        case 2:
        // FF FA 2C 01 00 00 25 80 FF F0
			snd_buffer[0] = 0xFF;
			snd_buffer[1] = 0xFA;
			snd_buffer[2] = 0x2c;
			snd_buffer[3] = 0x01;
			snd_buffer[4] = 0x00;
			snd_buffer[5] = 0x00;
			snd_buffer[6] = 0x25;
			snd_buffer[7] = 0x80;
			snd_buffer[8] = 0xFF;
			snd_buffer[9] = 0xF0;
			send(so, (const char *)snd_buffer, 10, 0); 
			*state = 3;
			change = 1;
            break;
        default:
            break;
                
    }
    
}

int set_lantronic_baudrate (char *serv_ip, int baudrate)
{
	WSADATA 			wsaData;
	unsigned long addr;
	char cac_buffer[512];
	SOCKET sock;
	int retval, rcv_len;
	struct sockaddr_in 	serv;
	int result, state, change;
	struct timeval tv;
	fd_set readset;

	addr = inet_addr(serv_ip);
	sprintf_s(newbaud, "%d", baudrate); 

	memset(&serv,0,sizeof(serv));
	serv.sin_addr.s_addr = addr;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(23);

#if 1
	if ((retval = WSAStartup(0x202,&wsaData)) != 0) {
		WSACleanup();
		return -1;
	}
#endif
	sock = socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
	if (connect(sock,(struct sockaddr*)&serv,sizeof(serv))
		== SOCKET_ERROR) {
			WSACleanup();
			return -1;
	}
	rcv_len = 0;
	screen_buffer[0] = 0;
	tot_len = 0;
	state = 0;

	while (1)
	{
        telnetStateMachine(sock, &state);
		FD_ZERO(&readset);
		FD_SET(sock, &readset);
		tv.tv_sec = 1;  /* 1 Secs Timeout */
		tv.tv_usec = 0;
		result = select(sock + 1, &readset, NULL, NULL, &tv);
		if (result > 0) 
		{
			result = recv(sock, (char *)cac_buffer, 512, 0);
			if (result == 0 || result == SOCKET_ERROR) 
			{
				/* This means the other side closed the socket */
				// closesocket(socket_handle);
				return (-1);
			}
			else 
			{
				/* I leave this part to your own implementation */
				rcv_len = result;
				memmove(&screen_buffer[tot_len], cac_buffer, rcv_len);

				tot_len += rcv_len;
				screen_buffer[tot_len] = 0;
				change = 1;
				if (state == 3)
				{
					break;
				}
				if (change)
				{
					tot_len = 0;
					screen_buffer[0] = 0;
				}
			}
		}
		else
		{
			/* end of data, before save and exit */
			return (-1);
		}
	} /* while (1) */

	closesocket(sock);
	WSACleanup();
	return 0;

}
#endif
