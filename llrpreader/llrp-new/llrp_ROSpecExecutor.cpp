//=============================================================================
///
/// @file AkMntServer.cpp
///
/// Copyright (c) 2009 MTI Laboratory Inc.
///
/// @brief < This file contains member function for AkMntServer class
//=============================================================================

#include  "llrp_ROSpecExecutor.h"
#include  "llrp_MsgProcessor.h"

#include  "iReaderapi.h"
#include  "debug_print.h"

#include <iostream>

using namespace std;

unsigned int LLRP_ROSpecExecutor::nextId = 0;

#define IREADER_TESTING 1
#define _time64		time

//=============================================================================
// Constructor

LLRP_ROSpecExecutor::LLRP_ROSpecExecutor(LLRP_Controller *pController, CMessage *pMsg):
    OwTask( HIGH, 2048, "LLRP_ROSpecExecutor"),
	id( ++nextId )
{
	DBG_PRINT( DEBUG_INFO, "LLRP_ROSpecExecutor[%d]:: Created"NL, id );

	m_pController = pController;
	m_pCMessage = pMsg;
	m_bIReaderFault = FALSE;
	m_totalTagsCount = 0;
	m_totalReadCount = 0;
}

void
LLRP_ROSpecExecutor::initialize(void)
{
	m_bIReaderFault = FALSE;
	m_totalTagsCount = 0;
	m_totalReadCount = 0;
}

//=============================================================================
// Thread Main entry point

void
LLRP_ROSpecExecutor::main( OwTask * )
{
	CROSpec * pROSpec;
	CROBoundarySpec *pROBoundarySpec;
	CROSpecStopTrigger * pROSpecStopTrigger;
	bool	bStopTrigger = FALSE;

	initialize();
	if ( TRUE == m_bIReaderFault )
		return;

	pROSpec = ((CADD_ROSPEC *)m_pCMessage)->getROSpec();
	m_uiROSpecID = pROSpec->getROSpecID();

	OwTask::sleep(2);

	// Send ROSpec start notification event
	send_ROSpec_Start_Notification();

	pROBoundarySpec = pROSpec->getROBoundarySpec();
	pROSpecStopTrigger = pROBoundarySpec->getROSpecStopTrigger();

	EROReportTriggerType eROReportTrigger = ROReportTriggerType_Upon_N_Tags_Or_End_Of_AISpec;

	CROReportSpec *pROReportSpec= pROSpec->getROReportSpec();

	if (pROReportSpec != NULL)
	{
		eROReportTrigger = pROReportSpec->getROReportTrigger();
	}

    while ( true )
    {
		CAISpec *  pAISpec;
		CParameter * pParameter;

		for (
			std::list<CParameter *>::iterator param = pROSpec->beginSpecParameter();
			param != pROSpec->endSpecParameter();
			param++)
		{
			const CTypeDescriptor *     pType;

			pParameter = *param;

			pType = pParameter->m_pType;

  			if(&CAISpec::s_typeDescriptor != pType)
				continue;

			pAISpec = ( CAISpec *)pParameter;

			// Now process the AISpec
			// llrp_u16v_t antennaIDs(16);
			llrp_u16v_t antennaIDs; // No need to declare size since the copy constructor will delete 
									// the one you allocate and allocate a new one.
			CInventoryParameterSpec * pInventoryParamSpec;
			CAntennaConfiguration * pAntennaConfiguration = 0;
			CAISpecStopTrigger *     pAISpecStopTrigger;

			pAISpecStopTrigger = pAISpec->getAISpecStopTrigger();
			antennaIDs = pAISpec->getAntennaIDs();

#if 0
			for (
				std::list<CInventoryParameterSpec *>::iterator elm = pAISpec->beginInventoryParameterSpec();
				elm != pAISpec->endInventoryParameterSpec();
				elm++ )
			{
				//elm = pAISpec->beginInventoryParameterSpec();
				// pInventoryParamSpec = *(pAISpec->beginInventoryParameterSpec());
				pInventoryParamSpec = *elm;

				llrp_u16_t inventoryParmSpecID = pInventoryParamSpec->getInventoryParameterSpecID();
				EAirProtocols inventoryProtocolID = pInventoryParamSpec->getProtocolID();

				if (AirProtocols_EPCGlobalClass1Gen2 != inventoryProtocolID)
				{
					continue;
				}

			}
#endif
			//std::list<CAntennaConfiguration *>::iterator antCfgBeginElm = pInventoryParamSpec->beginAntennaConfiguration();
			//std::list<CAntennaConfiguration *>::iterator antCfgEndElm = pInventoryParamSpec->endAntennaConfiguration();
			// pAntennaConfiguration = *(pInventoryParamSpec->beginAntennaConfiguration());

			//if (antCfgBeginElm != antCfgEndElm)
			//	pAntennaConfiguration = *antCfgBeginElm;
			

			pAISpecStopTrigger = pAISpec->getAISpecStopTrigger();
			EAISpecStopTriggerType eAISpecStopTriggerType = pAISpecStopTrigger->getAISpecStopTriggerType();

			llrp_u32_t tagSearchTimeout = pAISpecStopTrigger->getDurationTrigger();

			// IReaderApiTagSearchTimeout(m_pController->iReaderHandle, tagSearchTimeout);

			if (antennaIDs.m_pValue[0] == 0)
			{
				// scan all the antennas
#if IREADER_TESTING
				for (int i = 0; i < m_pController->m_antCount; i++)
				{
					int tagCount = 0;
					int power = m_pController->m_antPower[m_pController->m_antList[i]];

					INT32 status = IReaderApiReadTagsMetaDataRSSI(m_pController->iReaderHandle, m_pController->m_antList[i], power, 
																	&tagCount,  (struct taginfo_rssi *)m_tagInfo);
					if (IREADER_SUCCESS != status)
					{
						cout << "iReader read tags fails" << endl;
						m_bIReaderFault = TRUE;
						break;
					}
					updateTagsArray_RSSI(tagCount, m_pController->m_antList[i] /* antID */, (struct taginfo_rssi *)m_tagInfo);
					if ( tagCount)
					{
						// sendRO_Access_Report(tagCount, m_pController->m_antList[i], (struct taginfo_rssi *)m_tagInfo);
						sendTagsReport();
					}

					// check the stop trigger condition here to stop inventory
					if ( ROSpecStopTriggerType_Null == pROSpecStopTrigger->getROSpecStopTriggerType())
					{
						if (ROSpecState_Active != pROSpec->getCurrentState())
						{
							bStopTrigger = TRUE;
							// send_ROSpec_End_Notification();
							break;
						}
					}

				} // end for
#endif
			}
			else
			{
				int antCnt = antennaIDs.m_nValue;

				for (int i = 0; i < antCnt; i++)
				{
					int tagCount = 0;
					int antID = antennaIDs.m_pValue[i];
					int power = m_pController->m_antPower[antID];

					INT32 status = IReaderApiReadTagsMetaDataRSSI(m_pController->iReaderHandle, antID, power, &tagCount,  (struct taginfo_rssi *)m_tagInfo);
					if (IREADER_SUCCESS != status)
					{
						cout << "iReader read tags fails" << endl;
						m_bIReaderFault = TRUE;
						break;
					}
					updateTagsArray_RSSI(tagCount, antID, (struct taginfo_rssi *)m_tagInfo);

					if ( tagCount)
					{
						// sendRO_Access_Report(tagCount, antennaIDs.m_pValue[i], (struct taginfo_rssi *)m_tagInfo);
						sendTagsReport();
				
					}

					// check the stop trigger condition here to stop inventory
					if ( ROSpecStopTriggerType_Null == pROSpecStopTrigger->getROSpecStopTriggerType())
					{
						if (ROSpecState_Active != pROSpec->getCurrentState())
						{
							bStopTrigger = TRUE;
							send_ROSpec_End_Notification();
							break;
						}
					}

				}  // end for

			} // end else
			if (TRUE == m_bIReaderFault || TRUE == bStopTrigger)
				break;

			break;			// We only allow 1 ParameterSpec for Now, same as RIFIDI Emulator

		} // for all AiSpec loop

		if ( TRUE == m_bIReaderFault )
		{
			m_pController->iReaderHasFailed(m_uiROSpecID);
			break;
		}
		// check the ROSpec stop trigger condition, either restart the ROSpec or break, return
#if 0
		if ( ROSpecStopTriggerType_Null == pROSpecStopTrigger->getROSpecStopTriggerType())
		{
			if (ROSpecState_Active != pROSpec->getCurrentState())
			{
				send_ROSpec_End_Notification();
				break;
			}
		}
#endif
		if (TRUE == bStopTrigger)
		{
			pROSpec->setCurrentState(ROSpecState_Inactive);
			send_ROSpec_End_Notification();
			break;		// Thread exit
		}
		taskYield();
    }	// while( true )

	idle = true;		// Thread exit
}

void
LLRP_ROSpecExecutor::send_ROSpec_Start_Notification(void)
{
	CREADER_EVENT_NOTIFICATION *pEvtNotification = new CREADER_EVENT_NOTIFICATION;
	CReaderEventNotificationData * pEvtNotificationData = new CReaderEventNotificationData;

	CROSpecEvent * pROSpecEvent = new CROSpecEvent;
	pROSpecEvent->setEventType(ROSpecEventType_Start_Of_ROSpec);
	pROSpecEvent->setROSpecID(m_uiROSpecID);				// assume one for testing, should retrieve from ADD_ROSPEC

	pEvtNotificationData->setROSpecEvent(pROSpecEvent);

	CUTCTimestamp *pcTimestamp = new CUTCTimestamp;
	__time64_t time64us = _time64(0) * (__time64_t)1e6;
	pcTimestamp->setMicroseconds(time64us);

	pEvtNotificationData->setTimestamp(pcTimestamp);

	pEvtNotification->setReaderEventNotificationData(pEvtNotificationData);
	m_pController->sendMessage(pEvtNotification);

	delete pEvtNotification;
}

void
LLRP_ROSpecExecutor::send_ROSpec_End_Notification(void)
{
	CREADER_EVENT_NOTIFICATION *pEvtNotification = new CREADER_EVENT_NOTIFICATION;
	CReaderEventNotificationData * pEvtNotificationData = new CReaderEventNotificationData;

	CROSpecEvent * pROSpecEvent = new CROSpecEvent;
	pROSpecEvent->setEventType(ROSpecEventType_End_Of_ROSpec);
	pROSpecEvent->setROSpecID(m_uiROSpecID);				// assume one for testing, should retrieve from ADD_ROSPEC

	pEvtNotificationData->setROSpecEvent(pROSpecEvent);

	CUTCTimestamp *pcTimestamp = new CUTCTimestamp;
	__time64_t time64us = _time64(0) * (__time64_t)1e6;
	pcTimestamp->setMicroseconds(time64us);

	pEvtNotificationData->setTimestamp(pcTimestamp);

	pEvtNotification->setReaderEventNotificationData(pEvtNotificationData);
	m_pController->sendMessage(pEvtNotification);

	delete pEvtNotification;
}

void
LLRP_ROSpecExecutor::sendRO_Access_Report(int tagCnt, int antID, struct taginfo_rssi *tagInfo)
{
	CRO_ACCESS_REPORT *  pRO_ACCESS_REPORT = new CRO_ACCESS_REPORT;

	CTagReportData *pTagReportData;

	for (int i = 0; i < tagCnt; i++)
	{
		pTagReportData = new CTagReportData;

		CEPC_96  *pEPC_96;

        pEPC_96 = new CEPC_96;
		llrp_u96_t my_u96;

		for (int j = 0; j < 12; j++)
			my_u96.m_aValue[j] = tagInfo[i].tagid[j + 1];		// fill in the EPC tag data

		pEPC_96->setEPC(my_u96);

		pTagReportData->setEPCParameter(pEPC_96);

		CAntennaID *pAntID = new CAntennaID;

		pAntID->setAntennaID(antID);

		pTagReportData->setAntennaID(pAntID);

		CTagSeenCount *pTagSeenCount = new CTagSeenCount;

		pTagSeenCount->setTagCount(1);

		pTagReportData->setTagSeenCount(pTagSeenCount);

		CPeakRSSI * pPeakRSSI = new CPeakRSSI;
		pPeakRSSI->setPeakRSSI(tagInfo[i].tagid[0]);
		pTagReportData->setPeakRSSI(pPeakRSSI);

		CROSpecID * pROSpecID = new CROSpecID;
		pROSpecID->setROSpecID(m_uiROSpecID);
		pTagReportData->setROSpecID(pROSpecID);

		CLastSeenTimestampUTC * pLastSeenTimestampUTC = new CLastSeenTimestampUTC;

		__time64_t time64 = _time64(0) * (__time64_t)1e6;

		pLastSeenTimestampUTC->setMicroseconds(time64);
		pTagReportData->setLastSeenTimestampUTC(pLastSeenTimestampUTC);

		//CAirProtocolTagData * pAirProtocolTagData = new CAirProtocolTagData;
		// pTagReportData->addAirProtocolTagData(pAirProtocolTagData);

		CC1G2_CRC *pC1G2_CRC = new CC1G2_CRC;
		pC1G2_CRC->setCRC(0x1234);
		pTagReportData->addAirProtocolTagData(pC1G2_CRC);

		CC1G2_PC *pC1G2_PC = new CC1G2_PC;
		pC1G2_PC->setPC_Bits(0x5678);
		pTagReportData->addAirProtocolTagData(pC1G2_PC);

		CC1G2ReadOpSpecResult *pC1G2ReadOpSpecResult = new CC1G2ReadOpSpecResult;
		pC1G2ReadOpSpecResult->setResult(C1G2ReadResultType_Nonspecific_Tag_Error);
		pC1G2ReadOpSpecResult->setOpSpecID(16);

		llrp_u16v_t readData(16);

		readData.m_pValue[0] = 0x1234;
		readData.m_nValue = 1;
		pC1G2ReadOpSpecResult->setReadData(readData);
		pTagReportData->addAccessCommandOpSpecResult(pC1G2ReadOpSpecResult);

		pRO_ACCESS_REPORT->addTagReportData(pTagReportData);
	}
	pRO_ACCESS_REPORT->setMessageID(100);

	DBG_PRINT(DEBUG_ALL, "Sending  RO_ACCESS_REPORT Data"NL);
	m_pController->sendMessage(pRO_ACCESS_REPORT);
	delete pRO_ACCESS_REPORT;

}

void
LLRP_ROSpecExecutor::sendRO_Access_Report(void)
{
	CRO_ACCESS_REPORT *  pRO_ACCESS_REPORT = new CRO_ACCESS_REPORT;

	CTagReportData *pTagReportData;

	for (int i = 0; i < m_totalTagsCount; i++)
	{
		pTagReportData = new CTagReportData;

		CEPC_96  *pEPC_96;

        pEPC_96 = new CEPC_96;
		llrp_u96_t my_u96;

		for (int j = 0; j < 12; j++)
			my_u96.m_aValue[j] = m_tagsArray[i].tagid[j];		// fill in the EPC tag data

		pEPC_96->setEPC(my_u96);

		pTagReportData->setEPCParameter(pEPC_96);

		CAntennaID *pAntID = new CAntennaID;

		pAntID->setAntennaID(m_tagsArray[i].antid);

		pTagReportData->setAntennaID(pAntID);

		CTagSeenCount *pTagSeenCount = new CTagSeenCount;

		pTagSeenCount->setTagCount(m_tagsArray[i].count);

		pTagReportData->setTagSeenCount(pTagSeenCount);

		if ( m_pController->m_enableRSSI == 1)
		{
			CPeakRSSI * pPeakRSSI = new CPeakRSSI;
			pPeakRSSI->setPeakRSSI(m_tagsArray[i].RSSI);
			pTagReportData->setPeakRSSI(pPeakRSSI);
		}

		CROSpecID * pROSpecID = new CROSpecID;
		pROSpecID->setROSpecID(m_uiROSpecID);
		pTagReportData->setROSpecID(pROSpecID);

		CFirstSeenTimestampUTC * pFirstSeenTimestampUTC = new CFirstSeenTimestampUTC;

		pFirstSeenTimestampUTC->setMicroseconds(m_tagsArray[i].firstSeenTime);
		pTagReportData->setFirstSeenTimestampUTC(pFirstSeenTimestampUTC);

		CLastSeenTimestampUTC * pLastSeenTimestampUTC = new CLastSeenTimestampUTC;

		pLastSeenTimestampUTC->setMicroseconds(m_tagsArray[i].lastSeenTime);
		pTagReportData->setLastSeenTimestampUTC(pLastSeenTimestampUTC);

		//CAirProtocolTagData * pAirProtocolTagData = new CAirProtocolTagData;
		// pTagReportData->addAirProtocolTagData(pAirProtocolTagData);

		if ( 1 == m_pController->m_enableCRCBits)
		{
			CC1G2_CRC *pC1G2_CRC = new CC1G2_CRC;
			pC1G2_CRC->setCRC(0x1234);
			pTagReportData->addAirProtocolTagData(pC1G2_CRC);
		}

		if ( m_pController->m_enablePCBits == 1)
		{
			CC1G2_PC *pC1G2_PC = new CC1G2_PC;
			pC1G2_PC->setPC_Bits(m_tagsArray[i].pc_bits);
			pTagReportData->addAirProtocolTagData(pC1G2_PC);
		}

		CC1G2ReadOpSpecResult *pC1G2ReadOpSpecResult = new CC1G2ReadOpSpecResult;
		pC1G2ReadOpSpecResult->setResult(C1G2ReadResultType_Nonspecific_Tag_Error);
		pC1G2ReadOpSpecResult->setOpSpecID(16);

		llrp_u16v_t readData(16);

		readData.m_pValue[0] = 0x1234;
		readData.m_nValue = 1;
		pC1G2ReadOpSpecResult->setReadData(readData);
		pTagReportData->addAccessCommandOpSpecResult(pC1G2ReadOpSpecResult);

		pRO_ACCESS_REPORT->addTagReportData(pTagReportData);
	}
	pRO_ACCESS_REPORT->setMessageID(100);

	DBG_PRINT(DEBUG_ALL, "Sending  RO_ACCESS_REPORT Data, id = %d"NL, getObjectID());
	m_pController->sendMessage(pRO_ACCESS_REPORT);
	m_totalReadCount = 0;
	m_totalTagsCount = 0;
	delete pRO_ACCESS_REPORT;

}

void
LLRP_ROSpecExecutor::updateTagsArray_RSSI(int tagCount, int antID, struct taginfo_rssi *tagInfo)
{
	INT16 rssi;
	UINT16 pc_bits;
	UINT8 tagid[TAGIDLEN];

	for (int i = 0; i < tagCount; i++)
	{
		bool found = false;

		rssi = tagInfo[i].tagid[14];
		rssi = (rssi << 8) | ((tagInfo[i].tagid[15]) & 0xff);
		pc_bits = ((tagInfo[i].tagid[0]) << 8) | ((tagInfo[i].tagid[1]) & 0xff);
		memcpy(tagid, (const void *)&(tagInfo[i].tagid[2]), TAGIDLEN);

        for (int j = 0; j < m_totalTagsCount; j++)
        {
			if ((m_tagsArray[j].antid == antID) && (isTagEqual ((const char *)m_tagsArray[j].tagid, (const char *)tagid, TAGIDLEN) == true))
            {
                m_tagsArray[j].count++;
                m_tagsArray[j].RSSI = INT32(rssi/10);
				m_tagsArray[j].lastSeenTime =  _time64(0) * (__time64_t)1e6;
                found = true;
                break;
            }
        }
        if ((found == false) && (m_totalTagsCount < MAX_TAGS_REPORT))
        {
            memcpy( m_tagsArray[m_totalTagsCount].tagid, (const void *)tagid, TAGIDLEN); 

            m_tagsArray[m_totalTagsCount].count = 1;
            m_tagsArray[m_totalTagsCount].antid = antID;
            m_tagsArray[m_totalTagsCount].RSSI = INT32(rssi/10);
            m_tagsArray[m_totalTagsCount].pc_bits = pc_bits;
			m_tagsArray[m_totalTagsCount].firstSeenTime = m_tagsArray[m_totalTagsCount].lastSeenTime = _time64(0) * (__time64_t)1e6;

            m_totalTagsCount++;
        }
	}
	m_totalReadCount += tagCount;
}

void
LLRP_ROSpecExecutor::sendTagsReport(void)
{
	CROSpec * pROSpec;

	pROSpec = ((CADD_ROSPEC *)m_pCMessage)->getROSpec();

	CROReportSpec *pROReportSpec = pROSpec->getROReportSpec();

	if ( ROReportTriggerType_None == m_pController->m_eROReportTrigger )
		return;

	if ( m_totalReadCount < (m_pController->m_report_N))
		return;

	// if no ReportSepc, send the tags report anyway
	sendRO_Access_Report();
	//m_totalReadCount = 0;
	//m_totalTagsCount = 0;
}



