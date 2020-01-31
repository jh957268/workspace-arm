//=============================================================================
///
/// @file  AkMntServer.h
///
/// Copyright (c) 2009 MTI Laboratory, Inc. USA
///
/// @brief  This file contains the declarations of AkMntServer object.
///
//=============================================================================

#ifndef	LLRMSGPROCESSOR_H
#define	LLRPMSGPROCESSOR_H

//============================================================
// for Linux BSD Socket library
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//============================================================

#include "OwTask.h"
#include "llrp_Controller.h"

#include "ltkcpp.h"

using namespace std;
using namespace LLRP;

#define RFIDSPAN_VENDOR_ID	1234
#define IPADDR_SUBTYPE		4

///
/// This object establishes a TCP server for maintenance terminals.
/// It listen to the TCP port for connection from the client.  Once
/// connected, it hand over the connection to AkMntAgent, which
/// continues message exchanges with the client.
///
class LLRP_MsgProcessor
{
public:

    /// Get the instance of the object
    ///
    static LLRP_MsgProcessor*	getInstance()
	{
		if ( 0 == spInstance )
		{
			spInstance = new LLRP_MsgProcessor();
		}
		return  spInstance;
	};

	void   processMessage( LLRP_Controller* controller, CMessage *  pMessage );
	void sendRO_Access_Report(LLRP_Controller* controller, int tagCnt, int antID, struct taginfo_rssi *tagInfo);
  
private:

    /// Private constructor
    ///
    //LLRP_MntServer();
	LLRP_MsgProcessor();
	~LLRP_MsgProcessor();

    static LLRP_MsgProcessor* spInstance; ///< Points to the instance

	void processGetReaderConfig(LLRP_Controller* controller, CMessage *  pMessage);
	void processSetReaderConfig(LLRP_Controller* controller, CMessage *  pMessage);

	void send_ADD_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode);
	void send_ENABLE_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode);
	void send_START_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode);
	void send_STOP_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode);
	void send_DELETE_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode);
	void send_GET_READER_CAPABILITES_RESPONSE(LLRP_Controller * controller, const CMessage *pMsg);
	void send_GET_ACCESSSPEC_RESPONSE(LLRP_Controller * controller, const CMessage *pMsg);
	void processReportSpecSettings(LLRP_Controller* controller, CROReportSpec *pROReportSpec);
	void processAntennaConfigurationSettings(LLRP_Controller* controller, CAntennaConfiguration *pAntennaConfiguration);
	void processRospecAntennaConfigurationList(LLRP_Controller* controller, CROSpec * pROSpec);
};

//=============================================================================

#endif

//=============================================================================

