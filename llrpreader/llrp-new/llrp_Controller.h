//=============================================================================
///
/// @brief  AkController.h
/// Copyright (c) 2009 MTI Laboratory, Inc. USA
///
/// @brief  This file contains the declarations for AkController class
///
//=============================================================================

#ifndef LLRPCONTROLLER
#define LLRPCONTROLLER

#include <unistd.h>
#include "ltkcpp.h"

using namespace std;
using namespace LLRP;

#define MAX_NUM_ANTENNAS	256
#define ANT_DEFAULT_POWER	2500

enum EANTSTATE
{
	ANT_PLUGED = 1,
	ANT_UNPLUGED = 2
};

class LLRP_Controller
{
	public:
	// Called by AkMsgProcessor, return the result message
	virtual void	sendMessage( const char*  pcMessage )  = 0;
	virtual EResultCode	sendMessage( CMessage *   pMessage )  = 0;
	virtual EResultCode	addROSpecToList( CMessage * pMessage) = 0;

	virtual EResultCode enableROSpec (llrp_u32_t ROSpecID ) = 0;
	virtual EResultCode startROSpec (llrp_u32_t ROSpecID ) = 0;
	virtual EResultCode stopROSpec (llrp_u32_t ROSpecID ) = 0;
	virtual EResultCode deleteROSpec (llrp_u32_t ROSpecID ) = 0;
	virtual void	iReaderHasFailed (llrp_u32_t failedID) = 0;
	virtual void set_iReader_ipAddress(char *ip_addr, int len) {}
	virtual void get_iReader_ipAddress(char *ip_addr) {}
	virtual INT32 initialize(void) {return 0;}
	virtual void decKeepAliveSendCount(void) {}
	virtual void send_GET_ROSPEC_RESPONSE(const CMessage *pMsg){}

	// Called by AkMsgProcessor
	virtual void	disconnectSocket() = 0;
	virtual void 	closesocket(int fd) {close (fd);}

	void *			iReaderHandle;

	enum EANTSTATE	m_antState[MAX_NUM_ANTENNAS + 1];		// antID = 0 is not used
	UINT16			m_antPower[MAX_NUM_ANTENNAS + 1];		// antID = 0 is not used
	int				m_antCount;								// number of attached antennas
	int				m_antList[MAX_NUM_ANTENNAS];
	llrp_u1_t		m_enableRSSI;
	llrp_u1_t		m_enablePCBits;
	llrp_u1_t		m_enableCRCBits;
	EROReportTriggerType m_eROReportTrigger;
	llrp_u16_t		m_report_N;
};

//=============================================================================

#endif

//=============================================================================
