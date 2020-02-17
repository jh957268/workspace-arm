#include "iReader.h"
#include "iReadererror.h"

IReader* IReader::spInstance[MAX_IREADER_DEVICES] = {0, 0, 0, 0};

static char debug_buffer[128];

#define	IF_ERROR_RETURN(errcode) do {		\
	if ((errcode) != CRC_OK)						\
	{										\
		if ((errcode) == 0)					\
	   		return(IREADER_TIMEOUT_1); 		\
		else if ((errcode) == SOCKET_ERROR)	\
	   		return(IREADER_SOCKET_ERROR); 	\
		else								\
			return ((errcode));				\
	}										\
  } while (0)

#define OutputDebugString


IReader::IReader()
{
    memset((void *)m_antlist, 0x00, sizeof(m_antlist));
}


IReader::~IReader()
{
}

IReader
*IReader::getInstance
(
	int inst
)
{
	if (MAX_IREADER_DEVICES < inst)
	{
		return (NULL);
	}
	if ( 0 == spInstance[inst] )
	{
		spInstance[inst] = new IReader((char *)"0.0.0.0");
	}

	return( spInstance[inst] );

} // LLRP_MntServer:getInstance()

int  IReader::IReaderInit(void)
{

	for(int i = 0; i < MAXANT; i++)
		m_antpower[i] = DEFAULT_TX_POWER;

    return (IREADER_SUCCESS);
}

int  IReader::IReaderConnect(char *remote)
{
    int ret;
    
    ret = Connect(remote);
    return (ret);
}

int  IReader::IReaderDisconnect(void)
{
    Disconnect();
    return (IREADER_SUCCESS);
}

int  IReader::IReaderSetRegion(int region)
{
	int ret;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x02, 0xFF, 0xFD, 0x2C, 0x00};  // use antenna porrt 0
	buf[7] = (unsigned char)region;
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x2C)
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
	
		return (m_rxMsg.data[0]);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}
}

int  IReader::IReaderGetRegion(int *region)
{
	int ret;
	int reg;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x01, 0xFF, 0xFE, 0x91};  // 0x91 get region
    // retrieve the list from mux again

	ret = sendmsg(buf);

	if (m_rxMsg.opCode != 0x91 || ret != RFID_CMD_SUCCESS)
	{
		return (IREADER_COMMAND_FAIL);
	}

	reg = (m_rxMsg.data[0] << 8) | (m_rxMsg.data[1]);
	*region = reg;

    return (IREADER_SUCCESS);
}

int  IReader::IReaderGetSearchTimeout(int *timeout)
{
	int ret;
	int reg;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x01, 0xFF, 0xFE, 0x92};  // 0x92 get search timeout
    // retrieve the list from mux again

	ret = sendmsg(buf);

	if (m_rxMsg.opCode != 0x92 || ret != RFID_CMD_SUCCESS)
	{
		return (IREADER_COMMAND_FAIL);
	}

	reg = (m_rxMsg.data[0] << 8) | (m_rxMsg.data[1]);
	*timeout = reg;

    return (IREADER_SUCCESS);
}


int  IReader::IReaderSetPowerLevel(int antid, int pwr, int doset)
{
	int ret;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x04, 0xFF, 0xFB, 0x84, 0x00, 0x00, 0x00};  // use antenna porrt 0

	buf[7] = (unsigned char)antid;
	if ( pwr < 500)
		pwr = 500;
	else if (pwr > 3000)
		pwr = 3000;
	buf[8] = (unsigned char)(pwr >> 8);
	buf[9] = (unsigned char)(pwr & 0xFF);
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x84)
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
		return (m_rxMsg.data[0]);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}
}

int  IReader::IReaderSetWritePowerLevel(int pwr)
{
    Int32 error;

	error = setpower(pwr);
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
	int ret;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x02, 0xFF, 0xFD, 0x85, 0x00, 0x00, 0x00};  // use antenna porrt 0

	buf[7] = (unsigned char)antid;
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x85)
	{
		return (IREADER_COMMAND_FAIL);
	}
	*pwr = (((int)(m_rxMsg.data[0])) << 8) | m_rxMsg.data[1];
	return (ret);
}

int  IReader::IReaderReadTags(int antid, int *tagcount, struct taginfo *tagrbuf)
{
	unsigned char  buf[] = {HDR1, HDR2, 0x00, 0x02, 0xFF, 0xFD, 0x82, 0x00};
    int totaltags, tags;
    //char *ptagdata;
    //int taglen;
	int ret;
	struct taginfo_rssi *pTaginfo_rssi;
	
    *tagcount = 0;
	buf[7] = (unsigned char)antid;

	ret = sendmsg(buf);
	if ((m_rxMsg.opCode != 0x82) || (ret != RFID_CMD_SUCCESS))
	{
		return (IREADER_COMMAND_FAIL);
	}

	totaltags = m_rxMsg.data[1];		// data[0] is the antenna ID

    if (totaltags > MAX_TAGS_PER_ANTENNA)
    {
        return (IREADER_INVALID);
    }
	if ((m_rxMsg.dataLen) == 3 || (totaltags == 0))	  // 3: opcode, antid, tagcount	minimum 3 bytes
	{
		return IREADER_SUCCESS;
	}
	pTaginfo_rssi = (struct taginfo_rssi *)&m_rxMsg.data[2];
	for (tags = 0; tags <  totaltags; tags++)
	{
#ifdef WINNIX
		memmove(tagrbuf[tags].tagid, &(pTaginfo_rssi->tagid[2]), TAGLEN);
#else
		memmove(tagrbuf[tags].tagid, &(pTaginfo_rssi->tagid[1]), TAGLEN);
#endif
		pTaginfo_rssi++;
	}

	*tagcount = tags; 
	return (IREADER_SUCCESS);
}

int  IReader::IReaderReadTagsMetaDataRSSI(int antID, int pwr, int *tagcount, struct taginfo_rssi *tagrbuf)
{
	unsigned char  buf[] = {HDR1, HDR2, 0x00, 0x03, 0xFF, 0xFC, 0x83, 0x00, 0x00};
    int totaltags, tags;
	int ret;
	struct taginfo_rssi *pTaginfo_rssi;
	
    *tagcount = 0;
	buf[7] = antID;
	buf[8] = pwr;

	ret = sendmsg(buf);
	if ((m_rxMsg.opCode != 0x83) || (ret != RFID_CMD_SUCCESS))
	{
		return (IREADER_COMMAND_FAIL);
	}

	totaltags = m_rxMsg.data[1];		// data[0] is the antenna ID

    if (totaltags > MAX_TAGS_PER_ANTENNA)
    {
        return (IREADER_INVALID);
    }
	if ((m_rxMsg.dataLen) == 3 || (totaltags == 0))	  // 3: opcode, antid, tagcount	minimum 3 bytes
	{
		return IREADER_SUCCESS;
	}
	pTaginfo_rssi = (struct taginfo_rssi *)&m_rxMsg.data[2];
	for (tags = 0; tags <  totaltags; tags++)
	{
		memmove(tagrbuf[tags].tagid, &(pTaginfo_rssi->tagid[0]), TAGLEN_RSSI);
		pTaginfo_rssi++;
	}

	*tagcount = totaltags; 
	return (IREADER_SUCCESS);
}

int  IReader::IReaderGetTagsMetaDataRSSI(int *antID, int *tagcount, struct taginfo_rssi *tagrbuf)
{
    int totaltags, tags;
	int retv;
	struct taginfo_rssi *pTaginfo_rssi;
	
    *tagcount = 0;

	retv = MSG_receiveMsgObj_1(&m_rxMsg_1);
    if (retv == SOCKET_ERROR)
    {
        return (SOCKET_ERROR);
    }
	if ( 0 == retv )
	{
		return IREADER_SUCCESS;		// timout and no data available
	}

	if ((m_rxMsg_1.opCode != 0x88) || (retv != RFID_CMD_SUCCESS))
	{
		return (IREADER_COMMAND_FAIL);
	}
	*antID = m_rxMsg_1.data[0];
	totaltags = m_rxMsg_1.data[1];		// data[0] is the antenna ID

    if (totaltags > MAX_TAGS_PER_ANTENNA)
    {
        return (IREADER_INVALID);
    }
	if ((m_rxMsg_1.dataLen) == 3 || (totaltags == 0))	  // 3: opcode, antid, tagcount	minimum 3 bytes
	{
		return IREADER_SUCCESS;
	}

	pTaginfo_rssi = (struct taginfo_rssi *)&m_rxMsg_1.data[2];
	for (tags = 0; tags <  totaltags; tags++)
	{
		memmove(tagrbuf[tags].tagid, &(pTaginfo_rssi->tagid[0]), TAGLEN_RSSI);
		pTaginfo_rssi++;
	}

	*tagcount = totaltags; 
	return (IREADER_SUCCESS);
}

int  IReader::IReaderGetTags(int *antID, int *tagcount, struct taginfo *tagrbuf)
{
    int totaltags, tags;
	int retv;
	struct taginfo_rssi *pTaginfo_rssi;
	
    *tagcount = 0;

	retv = MSG_receiveMsgObj_1(&m_rxMsg_1);
    if (retv == SOCKET_ERROR)
    {
        return (SOCKET_ERROR);
    }
	if ( 0 == retv )
	{
		return IREADER_SUCCESS;		// timout and no data available
	}

	if ((m_rxMsg_1.opCode != 0x88) || (retv != RFID_CMD_SUCCESS))
	{
		return (IREADER_COMMAND_FAIL);
	}
	*antID = m_rxMsg_1.data[0] + 1;
	totaltags = m_rxMsg_1.data[1];		// data[0] is the antenna ID

    if (totaltags > MAX_TAGS_PER_ANTENNA)
    {
        return (IREADER_INVALID);
    }

	if ((m_rxMsg_1.dataLen) == 3 || (totaltags == 0))	  // 3: opcode, antid, tagcount	minimum 3 bytes
	{
		return IREADER_SUCCESS;
	}

	pTaginfo_rssi = (struct taginfo_rssi *)&m_rxMsg_1.data[2];
	for (tags = 0; tags <  totaltags; tags++)
	{
#ifdef WINNIX
		memmove(tagrbuf[tags].tagid, &(pTaginfo_rssi->tagid[2]), TAGLEN);
#else
		memmove(tagrbuf[tags].tagid, &(pTaginfo_rssi->tagid[1]), TAGLEN);
#endif
		pTaginfo_rssi++;
	}

	*tagcount = totaltags; 
	return (IREADER_SUCCESS);
}

// start or stop the executor
int  IReader::IReaderStartExecutor(int flag)
{
	int ret;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x02, 0xFF, 0xFD, 0x87, 0x00};  // use antenna porrt 0
	// unsigned char buf[] = {0xff, 0x2, 0x91, 0x2, 0x2, 0,0};
	buf[7] = (unsigned char)flag;
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != buf[6])
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
		return (m_rxMsg.data[0]);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}
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
	{
		return IREADER_SUCCESS;
	}
	tags = m_rxMsg.data[0];

	*tagcount = tags; 
	return (IREADER_SUCCESS);
}

int  IReader::IReaderWriteTagData(int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd)
{
	unsigned char  buf[64] = {HDR1, HDR2, 0, 0, RFID_CMD_WRITE_DATA, 0, 0, 0, 0, 0x30, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Int32 result;
	int totallen = 0x1f + datalen;
	int dataleninwords = (datalen >> 1);

	buf[3] = (unsigned char)totallen;

	memcpy((void *)&buf[5], (void *)passwd, 4);
	memcpy((void *)&buf[11], (void *)tagid, 12);
	buf[23] = (unsigned char)membank;				// membank;
	buf[24] = (unsigned char )((addr >> 8) & 0xff);
	buf[25] = (unsigned char )(addr & 0xff);
	buf[26] = (unsigned char )((dataleninwords >> 8) & 0xff);
	buf[27] = (unsigned char )(dataleninwords & 0xff);
	memcpy((void *)&buf[28], (void *)data, datalen);

	buf[totallen - 3] = 0;									// preset CRC to 0;

	result = sendmsg(buf);
	if ( CRC_OK != result )			// 1 is success, otherwise fail
	{
		return (result);
	}
	if ( (m_rxMsg.opCode != RFID_CMD_WRITE_DATA_RESP) || (m_rxMsg.data[0] != RFID_CMD_SUCCESS))
	{
		return ( IREADER_SET_POWER_FAIL );
	}
	return (IREADER_SUCCESS);
}

int  IReader::IReaderReadTagData(int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd)
{
	unsigned char  buf[64] = {HDR1, HDR2, 0, 0x1f, RFID_CMD_READ_DATA, 0, 0, 0, 0, 0x30, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Int32 result;
	int dataleninwords = (datalen >> 1);

	memcpy((void *)&buf[5], (void *)passwd, 4);
	memcpy((void *)&buf[11], (void *)tagid, 12);
	buf[23] = (unsigned char)membank;				// membank;
	buf[24] = (unsigned char )((addr >> 8) & 0xff);
	buf[25] = (unsigned char )(addr & 0xff);
	buf[26] = (unsigned char )((dataleninwords >> 8) & 0xff);
	buf[27] = (unsigned char )(dataleninwords & 0xff);
	buf[28] = 0;									// preset CRC to 0;

	result = sendmsg(buf);
	if ( CRC_OK != result )			// 1 is success, otherwise fail
	{
		return (result);
	}
	if ( (m_rxMsg.opCode != RFID_CMD_READ_DATA_RESP) || (m_rxMsg.data[0] != RFID_CMD_SUCCESS))
	{
		return ( IREADER_SET_POWER_FAIL );
	}
	memmove((void *)data, (void *)&m_rxMsg.data[4], datalen);			// datalen is length in bytes

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
    {
        return (error);
    }
    *antCount = m_antcount;
    for (i = 0; i < m_antcount; i++)
    {
        antList[i] = m_antlist[i];
    }
    return (IREADER_SUCCESS);
}

int  IReader::IReaderGetAntMap(void)
{
	int m, ret;
    uint8_t map[40];
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x01, 0xFF, 0xFE, 0x8B};  // use antenna porrt 0
    // retrieve the list from mux again

	m_antcount = 0;
	// SelectModule(MUX_MODULE);
	ret = sendmsg(buf);

	if (m_rxMsg.opCode != 0x8B || ret != RFID_CMD_SUCCESS)
	{
		return (IREADER_COMMAND_FAIL);
	}

	memmove(map, &m_rxMsg.data[0], 32);
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

int  IReader::IReaderGetAntMap(char *map)
{
	int m, ret;
  	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x01, 0xFF, 0xFE, 0x8B};  // use antenna porrt 0
    // retrieve the list from mux again

	m_antcount = 0;
	// SelectModule(MUX_MODULE);
	ret = sendmsg(buf);

	if (m_rxMsg.opCode != 0x8B || ret != RFID_CMD_SUCCESS)
	{
		return (IREADER_COMMAND_FAIL);
	}

	memmove(map, &m_rxMsg.data[0], 256);
    return (IREADER_SUCCESS);
}


int  IReader::IReaderSetAntScanMap(uint8_t *ant_map)
{
	int m, error, ret;
    uint8_t map[40];
	unsigned char buf[64] = {HDR1, HDR2, 0x00, 33, 0xFF, (unsigned char)~33, 0x88};  // use antenna porrt 0
    // retrieve the list from mux again

	// SelectModule(MUX_MODULE);
	memcpy(&buf[7], ant_map, 32);
	ret = sendmsg(buf);

	if (m_rxMsg.opCode != 0x88 || ret != RFID_CMD_SUCCESS)
	{
		return (IREADER_COMMAND_FAIL);
	}

    return (IREADER_SUCCESS);
}

int  IReader::IReaderGetAntScanMap(uint8_t *ant_map)
{
	int m, error, ret;
    uint8_t map[40];
	unsigned char buf[64] = {HDR1, HDR2, 0x00, 0x01, 0xFF, 0xFE, 0x89};  // use antenna porrt 0
    // retrieve the list from mux again

	// SelectModule(MUX_MODULE);
	memcpy(&buf[7], ant_map, 32);
	ret = sendmsg(buf);

	if (m_rxMsg.opCode != 0x89 || ret != RFID_CMD_SUCCESS)
	{
		return (IREADER_COMMAND_FAIL);
	}
	memcpy(ant_map, &m_rxMsg.data[0], 32);

    return (IREADER_SUCCESS);
}

int  IReader::IReaderGetScanAntList(int *antCount, int *antList)
{
    int error;
	uint8_t scan_map[48];
    
    *antCount = 0;
    error = IReaderGetAntScanMap(scan_map);
    if (error)
    {
        return (error);    
    }
	int acount = 0;
	for (int m = 1; m <= 32*8; m++)
	{		
		if (scan_map[31- (m - 1)/8] & (1 <<(m - 1)%8))
		{    
			antList[acount] = m;
			acount++;
		}
	}
	*antCount = acount;
    return (IREADER_SUCCESS);
}

int  IReader::IReaderRescanSlave(Int32 chn)
{
    // retrieve the list from mux again
    Int32 error = 0;
    
    //error = RescanSlave(chn);
    return (error);
}

int  IReader::IReaderCreateMutex(void)
{
    return ( IREADER_SUCCESS );
}

int  IReader::IReaderCloseMutex(void)
{
     return ( IREADER_SUCCESS );
}

int  IReader::IReaderTakeMutex(void)
{
	int error = IREADER_SUCCESS;

 	return (error);
}

int  IReader::IReaderGiveMutex(void)
{
    return ( IREADER_SUCCESS );
}


int  IReader::StartApp(void)
{
	return 0;
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
   return 0;
 
}

int IReader::MSG_receiveMsgObj(MsgObj *hMsg)
{
  s32 bytesRead;
  u8  soh;
  u8  i;
  //bool retVal;
  u8  datalen[2];
  u16 xdataLen;

  #define	GETCHAR(head) do {						\
		int val; \
		if ((val = getChar((head))) != 1)					\
		{ \
  			sprintf(debug_buffer, "GETCHAR: val = %d", val);	\
			return(val); \
		} \
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

	GETCHAR(&datalen[0]);
 	GETCHAR(&datalen[1]);
	hMsg->dataLen = ((datalen[0] << 8) | datalen[1]);
	GETCHAR(&datalen[0]);
 	GETCHAR(&datalen[1]);
	xdataLen = ((datalen[0] << 8) | datalen[1]);
	xdataLen = ~xdataLen;

	if (xdataLen != hMsg->dataLen)
	{
		sprintf(debug_buffer, "Data length fail: xdatalen = %d, msdatalen = %d",xdataLen, hMsg->dataLen );
		//OutputDebugString(debug_buffer);
		return RFID_CMD_FAIL;
	}

	if ( MSG_MAX_DATA_LENGTH < hMsg->dataLen)
	{
		sprintf(debug_buffer, "Max Data length fail: datalen = %d",hMsg->dataLen );
		//OutputDebugString(debug_buffer);
		return (RFID_CMD_FAIL);
	}

#if 0
  	GETCHAR(&hMsg->opCode);

  	for(i=0; i<hMsg->dataLen - 1; i++)
  	{
		GETCHAR(&hMsg->data[i]);
    	//while(mySD.ReadData(&hMsg->data[i], 1) == 0);
  	}
#endif

	bytesRead = getChar(&hMsg->opCode);
	if ( 1 != bytesRead)
	{
		sprintf(debug_buffer, "Fails get opCode: %d", bytesRead);
		//OutputDebugString(debug_buffer);
		return (bytesRead);
	}
 	for(i=0; i<hMsg->dataLen - 1; i++)   // -1 because opcode is read where opcode is included in len
  	{
		bytesRead = getChar(&hMsg->data[i]);
		if ( 1 != bytesRead)
		{
			sprintf(debug_buffer, "Fails get data: %d", bytesRead);
			//OutputDebugString(debug_buffer);
			return (bytesRead);
		}
  	}

	return (RFID_CMD_SUCCESS);
}

int IReader::MSG_receiveMsgObj_1(MsgObj *hMsg)
{
  int bytesRead;
  u8  soh;
  bool retVal;
  u8  datalen[2];
  u16 xdataLen;

//	MuxApiDevSelectModule(muxh,READER_MODULE);
	while(1)
	{
		bytesRead = getChars_1(&soh, 1);
		if (  bytesRead != 1)
		{
			return (0);
		}

		if(soh != HDR1)
		{
			continue;
		}
		bytesRead = getChars_1(&soh, 1);
		if (  bytesRead != 1)
		{
 			sprintf(debug_buffer, "GETCHAR: val = %d", bytesRead);
			OutputDebugString(debug_buffer);
			return (0);
		}

		if (HDR2 != soh)
			continue;
		break;
	}
	bytesRead = getChars_1(datalen, 2);
	if (  bytesRead != 2)
	{
		sprintf(debug_buffer, "Get Data len Fails: val = %d", bytesRead);
		OutputDebugString(debug_buffer);
		return (0);
	}

	hMsg->dataLen = ((datalen[0] << 8) | datalen[1]);
	bytesRead = getChars_1(datalen, 2);
	if (  bytesRead != 2)
	{
		sprintf(debug_buffer, "Get xData len Fails: val = %d", bytesRead);
		OutputDebugString(debug_buffer);
		return (0);
	}
	xdataLen = ((datalen[0] << 8) | datalen[1]);
	xdataLen = ~xdataLen;

	if (xdataLen != hMsg->dataLen)
	{
		sprintf(debug_buffer, "Data len unmatch");
		OutputDebugString(debug_buffer);
		return RFID_CMD_FAIL;
	}

	if ( MSG_MAX_DATA_LENGTH < hMsg->dataLen)
	{
		sprintf(debug_buffer, "Data Max len Fails");
		OutputDebugString(debug_buffer);
		return (RFID_CMD_FAIL);
	}

  	bytesRead = getChars_1(&hMsg->opCode, 1);
	if (  bytesRead != 1)
	{
		sprintf(debug_buffer, "Get opcode Fails: val = %d", bytesRead);
		OutputDebugString(debug_buffer);
		return (0);
	}
  	bytesRead = getChars_1(hMsg->data, hMsg->dataLen - 1);
	if (  bytesRead != (hMsg->dataLen - 1))
	{
		sprintf(debug_buffer, "Get rest bytes Fails: val = %d", bytesRead);
		OutputDebugString(debug_buffer);
		return (0);
	}
	return (RFID_CMD_SUCCESS);
}


int IReader::sendmsg(unsigned char *buf)
{
	int len, retv, ntry;;

	len = ((buf[2] << 8) | buf[3]) + 6;		   // 2 bytes HDR, 2 bytes len, 2 bytes 1's complement len
	// docrc(buf, len);
	memset(&m_rxMsg, 0x0, sizeof(m_rxMsg));
    for (ntry = 0; ntry < 3; ntry++)
    {
        retv = sendArray((unsigned char *)buf, len);
        if (retv == SOCKET_ERROR)
            return (SOCKET_ERROR);
	    retv = MSG_receiveMsgObj(&m_rxMsg);
        if (retv == SOCKET_ERROR)
            return (SOCKET_ERROR);
        else if (retv == 0)
            continue;
	    return (RFID_CMD_SUCCESS);
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
	int ret;

	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x02, 0xFF, 0xFD, 0x2C, region};  // set region command

	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x2C)
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
		return (m_rxMsg.data[0]);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}

}

int IReader::setprotocol(void)
{
	unsigned char buf[] = {0xff, 0x2, 0x93, 0, 0x5, 0,0};
	return sendmsg(buf);
}

int IReader::setantport(int id)
{
	int ret;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x02, 0xFF, 0xFD, 0x81, 0x02};  // use antenna porrt 0
	// unsigned char buf[] = {0xff, 0x2, 0x91, 0x2, 0x2, 0,0};
	buf[7] = (unsigned char)id;
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x81)
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
		return (m_rxMsg.data[0]);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}
}

int IReader::setscanantid(int id, int flag)
{
	int ret;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x03, 0xFF, 0xFC, 0x8A, 0x00, 0x00};  // use antenna porrt 0
	// unsigned char buf[] = {0xff, 0x2, 0x91, 0x2, 0x2, 0,0};
	buf[7] = (unsigned char)id;
	buf[8] = (unsigned char)flag;
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x8A)
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
		return (m_rxMsg.data[0]);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}
}

int IReader::GetEquipTemp(int *temp)
{
	int ret, temperature;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x01, 0xFF, 0xFE, 0x34};  // use antenna porrt 0
	// unsigned char buf[] = {0xff, 0x2, 0x91, 0x2, 0x2, 0,0};
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x34)
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
		temperature = (int)(m_rxMsg.data[0]);
		temperature <<= 8;

		temperature |= m_rxMsg.data[1];
		*temp = temperature;

		return (IREADER_SUCCESS);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}
}

int IReader::SetEquipTempProtect(int flag)
{
	int ret;
	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x02, 0xFF, 0xFD, 0x38, 0x00};  // use antenna porrt 0
	// unsigned char buf[] = {0xff, 0x2, 0x91, 0x2, 0x2, 0,0};
	buf[7] = (unsigned char)flag;
	ret = sendmsg(buf);
	if (m_rxMsg.opCode != 0x38)
	{
		return (IREADER_COMMAND_FAIL);
	}

	if (ret == RFID_CMD_SUCCESS)
	{
	
		return (m_rxMsg.data[0]);
	}
	else
	{
		return (IREADER_COMMAND_FAIL);
	}
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

int IReader::getportreturnloss(int *vswr)
{
	int result;
	short tmp;

	unsigned char buf[] = {HDR1, HDR2, 0x00, 0x08, CMD_GET_RETURN_LOSS, 0x00, 0x0D, 0x0A}; 

	result = sendmsg(buf);
	if ( CRC_OK != result )			// 1 is success, otherwise fail
		return (result);
	if ( (m_rxMsg.opCode != CMD_GET_RETURN_LOSS_REPLY) || (m_rxMsg.data[0] != RFID_CMD_SUCCESS))
		return ( IREADER_SET_POWER_FAIL );
	tmp = (m_rxMsg.data[1] << 8) |	m_rxMsg.data[2];
	*vswr = tmp;
	return (result);

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



