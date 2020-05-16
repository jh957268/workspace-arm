#ifndef __IREADER_H__
#define __IREADER_H__

#include <unistd.h>
#include "muxclient.h"

#include "AsciiDriver.h"
#include "CrcUtils.h"
#include "MsgUtils.h"
#include "HostTypes.h"
#include "OwMutex.h"
#include "OwTask.h"
#define RFID_CMD_SUCCESS		0x01
#define RFID_CMD_FAIL			0x00
#define CRC_OK					0x01
#define	JUMBO_DATA_LEN			0xff

#define MAX_TAGS_PER_ANTENNA	50
#define DEFAULT_TAG_SEARCH_TIME	80		// 100 ms
#define DEFAULT_TX_POWER		2500	// 25 dBm

#define RFID_CMD_SET_TXPOWER	0x10
#define RFID_REPLY_SET_TXPOWER	0x11
#define CMD_CONTINUE_INVENTORY	0x82
#define CMD_ONE_INVENTORY		0x81
#define CMD_ANT_WORKING_TIME	0x4A


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

#define TAGLEN		12
#define TAGLEN_RSSI	17
#define MAXANT      256
#define TAGBUFLEN 	512

// Glue Define for windows

struct taginfo
{
	char tagid[TAGLEN];
};

struct taginfo_rssi
{
	// first two bytes are PC BITS, 12 bytes EPC data, two bytes RSSI, total 16 bytes
	char tagid[TAGLEN_RSSI];
};

class IReader : public MuxClient
{
	protected:
		int				m_antlist[MAXANT];
		int				m_antpower[MAXANT];
		int				m_antcount;
		int				m_region;
		int				m_tagserachtimeout;
		MsgObj  		m_rxMsg;
		OwMutex			*m_hMutex;

	// ++++++++++++++++++++++++++++++++++++++++++++++
	// .................. EXTERNAL VIEW .............
	// ++++++++++++++++++++++++++++++++++++++++++++++

	public:
		IReader(char *server_ip): MuxClient(server_ip)
		{
    		memset((void *)m_antlist, 0x00, sizeof(m_antlist));
			m_region = 2;
			m_tagserachtimeout = DEFAULT_TAG_SEARCH_TIME;
		}
					 ~IReader();
		static 		 IReader*	getInstance(void);
		int           IReaderInit(char *ipaddr, int region);
		int           IReaderConnect(void);
		int           IReaderDisconnect(void);
		int           IReaderSetRegion(int region);
		int           IReaderGetRegion(void){return m_region;}
		int           IReaderGetSearchTimeout(void){return m_tagserachtimeout;}
		int           IReaderSetPowerLevel(int antid, int pwr, int doset);
		int           IReaderSetWritePowerLevel(int pwr);
		int           IReaderGetPowerLevel(int antid, int *pwr);
		int 		  IReaderTagSearchTimeout(int timeout);
		int  		  IReaderReadTags(int *tagcount, struct taginfo *tagrbuf);
		int  		  IReaderReadTagsMetaDataRSSI(int *tagcount, struct taginfo_rssi *tagrbuf);
		int  		  IReaderGetTagCount(int *tagcount);
		int  		  IReaderWriteTag(int timeout, unsigned char *tagid);
		int           IReaderReadTagsAll(void);
		int  		  IReaderGetAntList(int *antCount, int *antList);
		int           IReaderGetAntMap(void);
		int           IReaderRescanSlave(Int32 chn);
		int           IReaderCreateMutex(void);
		int           IReaderCloseMutex(void);
		int           IReaderTakeMutex(void);
		int           IReaderGiveMutex(void);

		// ThingMagic reader stuff
		int			StartApp(void);
		int 		MSG_sendMsgObj(MsgObj *hMsg);
		int 		MSG_receiveMsgObj(MsgObj *hMsg);
		int 		sendmsg(unsigned char *buf);
		void 		docrc(unsigned char *buf, int len);
		int 		setpower1(void);
		int			get_boot_firmware_ver(void);
		int 		setregion0(void);
		int 		setregion(int region);
		int 		setprotocol(void);
		int 		setantport(void);
		int 		poweroff(void);
		int 		poweron(void);
		int 		setpower(int power);
		int 		setwritepower(int power);
		int 		clrtagbuf(void);
		int 		ledon(void);
		int  		ledoff(void);
		void 		setbaud(void);
		int 		setAntPower(int rId, int antId, int power);
	private:
	    static 		IReader* spInstance; ///< Points to the instance
};

#endif
