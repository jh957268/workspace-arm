//=============================================================================
///
/// @file  AkMntServer.h
///
/// Copyright (c) 2009 MTI Laboratory, Inc. USA
///
/// @brief  This file contains the declarations of AkMntServer object.
///
//=============================================================================

#ifndef	LLRPROSPECEXECUTER_H
#define	LLRPROSPECEXECUTER_H

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

///
/// This object establishes a TCP server for maintenance terminals.
/// It listen to the TCP port for connection from the client.  Once
/// connected, it hand over the connection to AkMntAgent, which
/// continues message exchanges with the client.
///
#ifndef TAGIDLEN
#define TAGIDLEN	12
#endif

#define	MAX_TAGS_REPORT	1024

struct llrp_taginfo
{
	UINT8		tagid[TAGIDLEN];
    INT32		count;
    INT32		antid;
    INT16		RSSI;
	UINT16		pc_bits;
	__time64_t	firstSeenTime;
	__time64_t	lastSeenTime;
};

class LLRP_ROSpecExecutor:
    public OwTask
{
public:
 
	LLRP_ROSpecExecutor(LLRP_Controller *pController, CMessage *pMsg);

    /// The main program for the ROSpecExecutor.  Inherited from OwTask.
    ///
    void	main(OwTask*);

protected:

	LLRP_Controller		*m_pController;					// Back pointer to LLRP Controller

	CMessage			*m_pCMessage;					// pointer to the RO_SPEC
	UINT8				m_tagInfo[8192];
	bool				m_bIReaderFault;
	llrp_u32_t			m_uiROSpecID;
	INT32				m_totalTagsCount;
	INT32				m_totalReadCount;
	struct llrp_taginfo	m_tagsArray[MAX_TAGS_REPORT];

public:

	inline void
    setController (LLRP_Controller *pController)
	{
        m_pController = pController;
    }

 	inline void
    setROSpec (CMessage *pCMessage)
	{
		m_pCMessage = pCMessage;

	}

	void	setRunning() 
	{
		idle = false;
	}

	bool	isIdle() { return idle; }


	static bool isTagEqual(const char *src, const char *dst, int len)
	{
		int i;

		for (i = 0; i < len; i++)
		{
			if (src[i] != dst[i])
				return false;
		}
		return true;
	}

	void sendRO_Access_Report(void);

	unsigned int getObjectID(void )
	{
		return id;
	}

 
private:
 
    /// Private constructor
    ///
    //LLRP_MntServer();
	//LLRP_ROSpecExecutor(LLRP_Controller *pController);

	void initialize(void);
	void send_ROSpec_Start_Notification(void);
	void send_ROSpec_End_Notification(void);
	void sendRO_Access_Report(int tagCnt, int antID, struct taginfo_rssi *tagInfo);
	// void sendRO_Access_Report(void);
	void sendTagsReport(void);
	void updateTagsArray_RSSI(int tagCount, int antID, struct taginfo_rssi *tagInfo);

    unsigned int	id;				///< ID of the agent
    bool			idle;			///< True if is idle

	static unsigned int  nextId;		///< Next agent ID available

};

#endif

//=============================================================================

