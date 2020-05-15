#ifndef __IREADER_H__
#define __IREADER_H__

#include <unistd.h>
#include "muxclient.h"

#include "MsgUtils.h"
#include "HostTypes.h"

#define RFID_CMD_SUCCESS		0x01
#define RFID_CMD_FAIL			0x00

#define CRC_OK					0x01
#define	JUMBO_DATA_LEN			0xff

#define MAX_TAGS_PER_ANTENNA	50
#define DEFAULT_TAG_SEARCH_TIME	200		// 100 ms
#define DEFAULT_TX_POWER		2500	// 25 dBm

#define RFID_CMD_SET_TXPOWER	0x10
#define RFID_REPLY_SET_TXPOWER	0x11
#define CMD_CONTINUE_INVENTORY	0x82
#define CMD_ONE_INVENTORY		0x81
#define CMD_ANT_WORKING_TIME	0x4A
#define CMD_GET_RETURN_LOSS		0x32
#define CMD_GET_RETURN_LOSS_REPLY 0x33
#define RFID_CMD_READ_DATA		0x84
#define RFID_CMD_READ_DATA_RESP  0x85
#define RFID_CMD_WRITE_DATA		0x86
#define RFID_CMD_WRITE_DATA_RESP  0x87

#define MAX_IREADER_DEVICES		4

#define HDR1	0xA5
#define HDR2	0x5A

#define REGION_CHINA1	0x01
#define REGION_CHINA2	0x02
#define REGION_EUROPE	0x04
#define REGION_USA		0x08
#define REGION_KOREA	0x16
#define REGION_JAPAN	0x32

#define NO_ANTENNA		0
#define HAS_ANTENNA		1

#define WINNIX
#define TAGLEN		12
#ifdef WINNIX
#define TAGLEN_RSSI	17
#else
#define TAGLEN_RSSI	13
#endif
#define MAXANT      256

#define TAGBUFLEN 	512

struct taginfo
{
	char tagid[TAGLEN];
};

struct taginfo_rssi
{
	// first two bytes are PC BITS, 12 bytes EPC data, two bytes RSSI, total 16 bytes
	char tagid[TAGLEN_RSSI];
}__attribute__((packed));

class IReader : public MuxClient
{
	protected:
		int				m_antlist[MAXANT];
		int				m_antpower[MAXANT];
		int				m_antcount;
		int				m_region;
		int				m_tagserachtimeout;
		MsgObj  		m_rxMsg;
		MsgObj  		m_rxMsg_1;
		void			*m_hMutex;

	// ++++++++++++++++++++++++++++++++++++++++++++++
	// .................. EXTERNAL VIEW .............
	// ++++++++++++++++++++++++++++++++++++++++++++++

	public:
		IReader(char *server_ip): MuxClient(server_ip)
		{
    		memset((void *)m_antlist, 0x00, sizeof(m_antlist));
			m_region = 0;
			m_hMutex = 0;
			m_antcount = 0;
			m_tagserachtimeout = DEFAULT_TAG_SEARCH_TIME;

		}
					 IReader();
					 ~IReader();
		static 		 IReader*	getInstance(int inst);
		int           IReaderInit(void);
		int           IReaderConnect(char *remote);
		int           IReaderDisconnect(void);
		int           IReaderSetRegion(int region);
		int           IReaderGetRegion(int *region);
		int           IReaderDBSelectAll(int limit, int offset, int table);
		int  		  IReaderDBInsertTag(char *tag_str);
		int           IReaderGetSearchTimeout(int *timeout);
		int           IReaderSetPowerLevel(int antid, int pwr, int doset);
		int           IReaderSetWritePowerLevel(int pwr);
		int           IReaderGetPowerLevel(int antid, int *pwr);
		int 		  IReaderTagSearchTimeout(int timeout);
		int  		  IReaderReadTags(int antid, int *tagcount, struct taginfo *tagrbuf);
		int  		  IReaderReadTagsMetaDataRSSI(int antID, int pwr, int *tagcount, struct taginfo_rssi *tagrbuf);
		int  		  IReaderGetTagsMetaDataRSSI(int *antID, int *tagcount, struct taginfo_rssi *tagrbuf);
		int  		  IReaderGetTags(int *antID, int *tagcount, struct taginfo *tagrbuf);
		int           IReaderGetTagDBRecord(char *DBRecord, int *cnt);
		int  		  IReaderStartExecutor(int flag, int fd = 0);
		int  		  IReaderGetTagCount(int *tagcount);
		int  		  IReaderWriteTag(int timeout, unsigned char *tagid);
		int  		  IReaderWriteTagData(int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd);
		int  		  IReaderReadTagData(int membank, Uint32 addr, unsigned char *data, int datalen, unsigned char *tagid, unsigned char *passwd);
		int           IReaderReadTagsAll(void);
		int  		  IReaderGetAntList(int *antCount, int *antList);
		int  		  IReaderGetScanAntList(int *antCount, int *antList);
		int           IReaderGetAntMap(void);
		int           IReaderGetAntMap(char *map);
		int           IReaderSetAntScanMap(uint8_t *ant_map);
		int           IReaderGetAntScanMap(uint8_t *ant_map);
		int           IReaderRescanSlave(Int32 chn);
		int           IReaderCreateMutex(void);
		int           IReaderCloseMutex(void);
		int           IReaderTakeMutex(void);
		int           IReaderGiveMutex(void);

		// ThingMagic reader stuff
		int			StartApp(void);
		int 		MSG_sendMsgObj(MsgObj *hMsg);
		int 		MSG_receiveMsgObj(MsgObj *hMsg);
		int 		MSG_receiveMsgObj_1(MsgObj *hMsg);
		int 		sendmsg(unsigned char *buf);
		void 		docrc(unsigned char *buf, int len);
		int 		setpower1(void);
		int			get_boot_firmware_ver(void);
		int 		setregion0(void);
		int 		setregion(int region);
		int 		setprotocol(void);
		int 		setantport(int id);
		int			setscanantid(int id, int flag);
		int 		GetEquipTemp(int *temp);
		int			SetEquipTempProtect(int flag);
		int 		poweroff(void);
		int 		poweron(void);
		int 		setpower(int power);
		int 		setwritepower(int power);
		int 		getportreturnloss(int *vswr);
		int 		clrtagbuf(void);
		int 		ledon(void);
		int  		ledoff(void);
		void 		setbaud(void);
		int 		setAntPower(int rId, int antId, int power);
	private:
	    static 		IReader* spInstance[MAX_IREADER_DEVICES]; ///< Points to the instance
};

#endif
