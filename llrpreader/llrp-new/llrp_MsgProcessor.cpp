//=============================================================================
///
/// @file AkMntServer.cpp
///
/// Copyright (c) 2009 MTI Laboratory Inc.
///
/// @brief < This file contains member function for AkMntServer class
//=============================================================================

#include  "llrp_MsgProcessor.h"
#include  "llrp_MsgTypes.h"
#include  "llrp_ROSpecExecutor.h"
#include  "llrp_MntAgent.h"

#include "iReaderapi.h"
#include "debug_print.h"

#include <time.h>
#include <iostream>

using namespace std;

#define strcpy_s(a, b ,c) strncpy(a, c, b)
#define _time64		time
//=============================================================================

LLRP_MsgProcessor* LLRP_MsgProcessor::spInstance = 0;

//=============================================================================
// Constructor

LLRP_MsgProcessor::LLRP_MsgProcessor()
{
}

LLRP_MsgProcessor::~LLRP_MsgProcessor()
{
}

//=============================================================================
// main

void
LLRP_MsgProcessor::processMessage( LLRP_Controller* controller, CMessage *  pMessage )
{
	if ( FALSE == pMessage->m_pType->m_bIsMessage )
	{
		DBG_PRINT(DEBUG_CRITICAL,"Message received is not a Message"NL);
		return;
	}

#if 0 // XXX JOO 12-10-2012
	if ( (0 == controller->iReaderHandle) && (LLRP_TYPE_SET_READER_CONFIG != pMessage ->m_pType -> m_TypeNum ))
	{
		((LLRP_MntAgent *)controller)->iReaderCannotConnect();
		delete pMessage;
		return;
	}
#endif
	switch ( pMessage ->m_pType -> m_TypeNum )
	{
		case LLRP_TYPE_SET_READER_CONFIG :
			processSetReaderConfig( controller, pMessage );
#if 0
			{
				CSET_READER_CONFIG_RESPONSE * pResp = new CSET_READER_CONFIG_RESPONSE;
				CLLRPStatus *pLLRPstatus = new CLLRPStatus;
				llrp_utf8v_t            ErrorDesc(32);

				strcpy_s((char *)ErrorDesc.m_pValue, 32, "This is very bad"); 
				ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);

				pLLRPstatus->setStatusCode(StatusCode_M_ParameterError);

				pLLRPstatus->setErrorDescription(ErrorDesc);

				pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list addSubParameterToAllList

				pResp->setMessageID(pMessage->getMessageID());
				DBG_PRINT(DEBUG_ALL,"Sending Response"NL);
				controller->sendMessage(pResp);

				delete pResp;
				// delete pLLRPstatus;				// No need to delete, will be deleted from CElement::~CElement which will be called 
													// when deleting Resp object.
			}
#endif

			break;

		case LLRP_TYPE_GET_READER_CAPABILITES :
			send_GET_READER_CAPABILITES_RESPONSE(controller, pMessage);
			break;

		case LLRP_TYPE_GET_READER_CONFIG :
			processGetReaderConfig( controller, pMessage );

			break;

		case LLRP_TYPE_KEEPALIVE_ACK :
			{
				controller->decKeepAliveSendCount();
			}
			break;

		case LLRP_TYPE_GET_REPORT :
			{
				((LLRP_MntAgent *)controller)->sendRO_Access_Report_OnDemand();
			}
			break;

		case LLRP_TYPE_ADD_ROSPEC :
			{
				EResultCode result = controller->addROSpecToList(pMessage);
				send_ADD_ROSPEC_RESPONSE(controller, (const CMessage *)pMessage, result);
			}
			return;
		case LLRP_TYPE_ENABLE_ROSPEC :
			{
				CENABLE_ROSPEC *pEnableROSpec = (CENABLE_ROSPEC *)pMessage;
				llrp_u32_t ROSpecID = pEnableROSpec->getROSpecID();
				EResultCode result = controller->enableROSpec(ROSpecID);
				send_ENABLE_ROSPEC_RESPONSE(controller, (const CMessage *)pMessage, result);
				
			}
			break;
		case LLRP_TYPE_START_ROSPEC :
			{
				CSTART_ROSPEC *pStartROSpec = (CSTART_ROSPEC *)pMessage;
				llrp_u32_t ROSpecID = pStartROSpec->getROSpecID();
				EResultCode result = controller->startROSpec(ROSpecID);
				send_START_ROSPEC_RESPONSE(controller, (const CMessage *)pMessage, result);

			}
			break;

		case LLRP_TYPE_STOP_ROSPEC :
			{
				CSTOP_ROSPEC *pStopROSpec = (CSTOP_ROSPEC *)pMessage;
				llrp_u32_t ROSpecID = pStopROSpec->getROSpecID();
				EResultCode result = controller->stopROSpec(ROSpecID);
				send_STOP_ROSPEC_RESPONSE(controller, (const CMessage *)pMessage, RC_OK/*result*/);

			}
			break;

		case LLRP_TYPE_GET_ROSPECS :
			{
				controller->send_GET_ROSPEC_RESPONSE((const CMessage *)pMessage);
			}
			break;

		case LLRP_TYPE_GET_ACCESSSPECS :
			{
				send_GET_ACCESSSPEC_RESPONSE(controller, (const CMessage *)pMessage);
			}
			break;

		default:
			{
				DBG_PRINT(DEBUG_WARNING,"Receive Unknown Message Type Number= %d"NL, pMessage ->m_pType -> m_TypeNum );
				//sendRO_Access_Report(controller);
			}
			break;
	}
	delete pMessage;

} // LLRP_MsgProcessor::processMessage

void
LLRP_MsgProcessor::processSetReaderConfig(LLRP_Controller* controller, CMessage *  pMessage)
{
	bool bInitFail = false;

	CSET_READER_CONFIG_RESPONSE * pResp = new CSET_READER_CONFIG_RESPONSE;
	CSET_READER_CONFIG *pSetReaderConfig = (CSET_READER_CONFIG *)pMessage;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(64);

#if 0
	strcpy((char *)ErrorDesc.m_pValue, "Success"); 
	ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);

	pLLRPstatus->setStatusCode(StatusCode_M_Success);
	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list addSubParameterToAllList
#endif

	for (
		std::list<CParameter *>::iterator cusElm = pSetReaderConfig->beginCustom();
		cusElm != pSetReaderConfig->endCustom();
		cusElm++ )
	{
		CCustom *pCustom = (CCustom *)*cusElm;

		llrp_u32_t vendorID = pCustom->getVendorIdentifier();
		llrp_u32_t subType = pCustom->getParameterSubtype();

		//llrp_bytesToEnd_t ipData(64);
		llrp_bytesToEnd_t ipData;

		ipData = pCustom->getData();
		//ipData.m_pValue[ipData.m_nValue - 1] = 0;

		if ((RFIDSPAN_VENDOR_ID == vendorID) && (IPADDR_SUBTYPE == subType))
		{
			// retrieve the iReader IP address
			controller->set_iReader_ipAddress((char *)ipData.m_pValue, ipData.m_nValue);
			if ( -1 == controller->initialize())
			{
				controller->set_iReader_ipAddress("0.0.0.0", sizeof("0.0.0.0"));
				// ((LLRP_MntAgent *)controller)->iReaderCannotConnect(); XXX JOO 12-10-2012
				bInitFail = true;

			}
		}
	}
	if ( true == bInitFail)
	{
		strcpy_s((char *)ErrorDesc.m_pValue, 64, "unknown custom extension"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		pLLRPstatus->setStatusCode(StatusCode_M_ParameterError);
		pLLRPstatus->setErrorDescription(ErrorDesc);
	}
	else
	{
		strcpy_s((char *)ErrorDesc.m_pValue, 64, "Success."); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		pLLRPstatus->setStatusCode(StatusCode_M_Success);
		pLLRPstatus->setErrorDescription(ErrorDesc);
	}
	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list addSubParameterToAllList

	for (
		std::list<CAntennaConfiguration *>::iterator antElm = pSetReaderConfig->beginAntennaConfiguration();
		antElm != pSetReaderConfig->endAntennaConfiguration();
		antElm++ )
	{
		CAntennaConfiguration *pAntennaConfiguration = *antElm;

		processAntennaConfigurationSettings(controller, pAntennaConfiguration);

#if 0
		llrp_u16_t antID = pAntennaConfiguration->getAntennaID();
		CRFTransmitter * pRFTransmitter = pAntennaConfiguration->getRFTransmitter();
		if ( 0 != pRFTransmitter )
		{
			llrp_u16_t txPower = pRFTransmitter->getTransmitPower();
			controller->m_antPower[antID] = txPower;

		}
#endif
	}

	CROReportSpec *pROReportSpec = pSetReaderConfig->getROReportSpec();
	if ( NULL != pROReportSpec)
		processReportSpecSettings(controller, pROReportSpec);

	pResp->setMessageID(pMessage->getMessageID());
	DBG_PRINT(DEBUG_ALL,"Sending CSET_READER_CONFIG_RESPONSE, errCode = %d"NL, pLLRPstatus->getStatusCode());
	controller->sendMessage(pResp);

	delete pResp;

}

void
LLRP_MsgProcessor::processGetReaderConfig(LLRP_Controller* controller, CMessage *  pMessage)
{
	CGET_READER_CONFIG_RESPONSE * pResp = new CGET_READER_CONFIG_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);

	strcpy((char *)ErrorDesc.m_pValue, "This is Success"); 
	ErrorDesc.m_nValue = 15;

	pLLRPstatus->setStatusCode(StatusCode_M_Success);

	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list addSubParameterToAllList

	CIdentification *pIdent = new CIdentification;

	EIdentificationType eIDType = IdentificationType_EPC;

	pIdent->setIDType(eIDType);

	llrp_u8v_t ReaderID(16);

	strcpy_s((char *)ReaderID.m_pValue, 16, "iReader-980"); 
	ReaderID.m_nValue = strlen((const char *)ReaderID.m_pValue);

	pIdent->setReaderID(ReaderID);

	pResp->setIdentification(pIdent);

	for (int i = 0; i < controller->m_antCount; i++)
	{
		CAntennaProperties *pAntPties = new CAntennaProperties;
		CAntennaConfiguration *pAntCfg = new CAntennaConfiguration;

		llrp_u16_t antID = controller->m_antList[i];

		pAntPties->setAntennaConnected(1);
		pAntPties->setAntennaID(antID);
		pAntPties->setAntennaGain(6);

		pAntCfg->setAntennaID(antID);

		CRFReceiver *cRF = new CRFReceiver;
		cRF->setReceiverSensitivity(103);
		pAntCfg->setRFReceiver(cRF);

		CRFTransmitter *cTX = new CRFTransmitter;
		llrp_u16_t power = controller->m_antPower[antID];
		cTX->setTransmitPower(power);
		// cTX->setChannelIndex(11);
		// cTX->setHopTableID(12);
		pAntCfg->setRFTransmitter(cTX);
		
		pResp->addAntennaProperties(pAntPties);
		pResp->addAntennaConfiguration(pAntCfg);

	}

	CCustom *pCustom = new CCustom;

	llrp_u32_t vendorID = RFIDSPAN_VENDOR_ID;
	pCustom->setVendorIdentifier(vendorID);
	llrp_u32_t subType = IPADDR_SUBTYPE;
	pCustom->setParameterSubtype(subType);

	llrp_bytesToEnd_t ipData(64);

	controller->get_iReader_ipAddress((char *)ipData.m_pValue);
	ipData.m_nValue = strlen((const char *)ipData.m_pValue);
	pCustom->setData(ipData);

	pResp->addCustom(pCustom);

	pResp->setMessageID(pMessage->getMessageID());
	DBG_PRINT(DEBUG_ALL,"Sending  CGET_READER_CONFIG_RESPONSE Response"NL);
	controller->sendMessage(pResp);

	delete pResp;
}

void
LLRP_MsgProcessor::sendRO_Access_Report(LLRP_Controller* controller, int tagCnt, int antID, struct taginfo_rssi *tagInfo)
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
		pROSpecID->setROSpecID(1);
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

	DBG_PRINT(DEBUG_ALL,"Sending  RO_ACCESS_REPORT Data"NL);
	controller->sendMessage(pRO_ACCESS_REPORT);
	delete pRO_ACCESS_REPORT;

}

void
LLRP_MsgProcessor::processReportSpecSettings(LLRP_Controller* controller, CROReportSpec *pROReportSpec)
{
	controller->m_eROReportTrigger = pROReportSpec->getROReportTrigger();
	controller->m_report_N = pROReportSpec->getN();
	controller->m_enableRSSI = pROReportSpec->getTagReportContentSelector()->getEnablePeakRSSI();

	for (
		std::list<CParameter *>::iterator param = pROReportSpec->getTagReportContentSelector()->beginAirProtocolEPCMemorySelector();
		param != pROReportSpec->getTagReportContentSelector()->endAirProtocolEPCMemorySelector();
		param++ )
	{
		CC1G2EPCMemorySelector *pC1G2EPCMemorySelector = (CC1G2EPCMemorySelector *)*param;

		controller->m_enablePCBits = pC1G2EPCMemorySelector->getEnablePCBits();
		controller->m_enableCRCBits = pC1G2EPCMemorySelector->getEnableCRC();
	}
}

void
LLRP_MsgProcessor::processAntennaConfigurationSettings(LLRP_Controller* controller, CAntennaConfiguration *pAntennaConfiguration)
{
	llrp_u16_t antID = pAntennaConfiguration->getAntennaID();
	CRFTransmitter * pRFTransmitter = pAntennaConfiguration->getRFTransmitter();

	if ( 0 != pRFTransmitter )
	{
		llrp_u16_t txPower = pRFTransmitter->getTransmitPower();

		if ( 0 == antID)
		{
			for (int i = 0; i <= MAX_NUM_ANTENNAS; i++)
			{
				controller->m_antPower[i] = txPower;
			}
		}
		else
		{
			if ( antID <= MAX_NUM_ANTENNAS)
			{
				controller->m_antPower[antID] = txPower;
			}
		}
	}
}

void
LLRP_MsgProcessor::processRospecAntennaConfigurationList(LLRP_Controller* controller, CROSpec * pROSpec)
{
	do
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

			CInventoryParameterSpec * pInventoryParamSpec;
			CAntennaConfiguration * pAntennaConfiguration = 0;

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
			
				for (
					std::list<CAntennaConfiguration *>::iterator antCfgBeginElm = pInventoryParamSpec->beginAntennaConfiguration();
					antCfgBeginElm != pInventoryParamSpec->endAntennaConfiguration();
					antCfgBeginElm++ )
				{
					// Not doing anything for now
					pAntennaConfiguration = *antCfgBeginElm;
					processAntennaConfigurationSettings (controller, pAntennaConfiguration);
				}
			}
		}
	} while (0);
}

void
LLRP_MsgProcessor::send_ADD_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode)
{

	CADD_ROSPEC_RESPONSE * pResp = new CADD_ROSPEC_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);
	EStatusCode				eStatus;

	if (RC_OK == errCode)
	{
		CROSpec * pROSpec;

		pROSpec = ((CADD_ROSPEC *)pMsg)->getROSpec();

		CROReportSpec *pROReportSpec = pROSpec->getROReportSpec();
		if ( NULL != pROReportSpec )
			processReportSpecSettings(controller, pROReportSpec);

		processRospecAntennaConfigurationList(controller, pROSpec);
		strcpy((char *)ErrorDesc.m_pValue, "Add ROSPEC Success"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_M_Success;
	}
	else
	{
		strcpy_s((char *)ErrorDesc.m_pValue, 32, "Fails - No iReader"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_R_DeviceError;
	}
	pLLRPstatus->setStatusCode(eStatus);
	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list

	pResp->setMessageID(pMsg->getMessageID());
	DBG_PRINT(DEBUG_ALL,"send_ADD_ROSPEC_RESPONSE, errCode = %d"NL, pLLRPstatus->getStatusCode());
	controller->sendMessage(pResp);
	delete pResp;
}

void
LLRP_MsgProcessor::send_GET_ACCESSSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg)
{
	CGET_ACCESSSPECS_RESPONSE * pResp = new CGET_ACCESSSPECS_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);
	EStatusCode				eStatus;

	
	strcpy((char *)ErrorDesc.m_pValue, "Success"); 
	ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
	eStatus = StatusCode_M_Success;

	pLLRPstatus->setStatusCode(eStatus);
	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list

	pResp->setMessageID(pMsg->getMessageID());
	DBG_PRINT(DEBUG_ALL,"Sending Response"NL);
	controller->sendMessage(pResp);
	delete pResp;
}

void
LLRP_MsgProcessor::send_ENABLE_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode)
{

	CENABLE_ROSPEC_RESPONSE * pResp = new CENABLE_ROSPEC_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);
	EStatusCode				eStatus;

	if (RC_OK == errCode)
	{
		strcpy((char *)ErrorDesc.m_pValue, "Enable ROSPEC Success"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_M_Success;
	}
	else
	{
		strcpy((char *)ErrorDesc.m_pValue, "Enable ROSPEC Fails"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_M_ParameterError;
	}
	pLLRPstatus->setStatusCode(eStatus);
	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list

	pResp->setMessageID(pMsg->getMessageID());
	DBG_PRINT(DEBUG_ALL,"send_ENABLE_ROSPEC_RESPONSE, errCode = %d"NL, pLLRPstatus->getStatusCode());
	controller->sendMessage(pResp);
	delete pResp;
}

void
LLRP_MsgProcessor::send_START_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode)
{

	CSTART_ROSPEC_RESPONSE * pResp = new CSTART_ROSPEC_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);
	EStatusCode				eStatus;

	if (RC_OK == errCode)
	{
		strcpy((char *)ErrorDesc.m_pValue, "Start ROSPEC Success"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_M_Success;
	}
	else
	{
		strcpy((char *)ErrorDesc.m_pValue, "Start ROSPEC Fails"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_M_ParameterError;
	}
	pLLRPstatus->setStatusCode(eStatus);
	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list

	pResp->setMessageID(pMsg->getMessageID());
	DBG_PRINT(DEBUG_ALL,"send_START_ROSPEC_RESPONSE, errCode = %d"NL, pLLRPstatus->getStatusCode());
	controller->sendMessage(pResp);
	delete pResp;
}

void
LLRP_MsgProcessor::send_STOP_ROSPEC_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg, llrp_u32_t errCode)
{

	CSTOP_ROSPEC_RESPONSE * pResp = new CSTOP_ROSPEC_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);
	EStatusCode				eStatus;

	if (RC_OK == errCode)
	{
		strcpy((char *)ErrorDesc.m_pValue, "Stop ROSPEC Success"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_M_Success;
	}
	else
	{
		strcpy((char *)ErrorDesc.m_pValue, "Stop ROSPEC Fails"); 
		ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);
		eStatus = StatusCode_M_ParameterError;
	}
	pLLRPstatus->setStatusCode(eStatus);
	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list

	pResp->setMessageID(pMsg->getMessageID());
	DBG_PRINT(DEBUG_ALL,"send_STOP_ROSPEC_RESPONSE, errCode = %d"NL, pLLRPstatus->getStatusCode());
	controller->sendMessage(pResp);
	delete pResp;
}

void
LLRP_MsgProcessor::send_GET_READER_CAPABILITES_RESPONSE(LLRP_Controller* controller, const CMessage *pMsg)
{
	CGET_READER_CAPABILITIES_RESPONSE * pResp = new CGET_READER_CAPABILITIES_RESPONSE;

	CLLRPStatus *pLLRPstatus = new CLLRPStatus;
	llrp_utf8v_t            ErrorDesc(32);
	EStatusCode				eStatus = StatusCode_M_Success;

	strcpy((char *)ErrorDesc.m_pValue, "Success"); 
	ErrorDesc.m_nValue = strlen((const char *)ErrorDesc.m_pValue);

	pLLRPstatus->setStatusCode(eStatus);
	pLLRPstatus->setErrorDescription(ErrorDesc);

	pResp->setLLRPStatus(pLLRPstatus);    // will add the object to the list

	CGeneralDeviceCapabilities * pGeneralDeviceCapabilities = new CGeneralDeviceCapabilities;

	pGeneralDeviceCapabilities->setMaxNumberOfAntennaSupported(256);
	pGeneralDeviceCapabilities->setCanSetAntennaProperties(0);
	pGeneralDeviceCapabilities->setHasUTCClockCapability(1);
	pGeneralDeviceCapabilities->setDeviceManufacturerName(0);
	pGeneralDeviceCapabilities->setModelName(0);

	llrp_utf8v_t readerFirmwareVersion(16);
	strcpy((char *)(readerFirmwareVersion.m_pValue), "1.0.0.1");
	readerFirmwareVersion.m_nValue = strlen((const char *)(readerFirmwareVersion.m_pValue));

	pGeneralDeviceCapabilities->setReaderFirmwareVersion(readerFirmwareVersion);

	for (int i = 0; i < 1; i++)
	{
		CReceiveSensitivityTableEntry *pReceiveSensitivityTableEntry = new CReceiveSensitivityTableEntry;
		pReceiveSensitivityTableEntry->setIndex(0);
		pReceiveSensitivityTableEntry->setReceiveSensitivityValue(-103);
		pGeneralDeviceCapabilities->addReceiveSensitivityTableEntry(pReceiveSensitivityTableEntry);
	}

	CGPIOCapabilities *pGPIOCapabilities = new CGPIOCapabilities;
	pGPIOCapabilities->setNumGPIs(0);
	pGPIOCapabilities->setNumGPOs(0);
	pGeneralDeviceCapabilities->setGPIOCapabilities(pGPIOCapabilities);

	for (int i = 0; i < 256; i++)
	{
		llrp_u8v_t protocolID(2);
		CPerAntennaAirProtocol *pPerAntennaAirProtocol = new CPerAntennaAirProtocol;
		pPerAntennaAirProtocol->setAntennaID(i+1);

		protocolID.m_nValue = 1;
		protocolID.m_pValue[0] = 1;
		pPerAntennaAirProtocol->setProtocolID(protocolID);
		pGeneralDeviceCapabilities->addPerAntennaAirProtocol(pPerAntennaAirProtocol);

	}

	pResp->setGeneralDeviceCapabilities(pGeneralDeviceCapabilities);

#if 1
	CLLRPCapabilities * pLLRPCapabilities = new CLLRPCapabilities;
	pLLRPCapabilities->setCanDoRFSurvey(0);
	pLLRPCapabilities->setCanDoTagInventoryStateAwareSingulation(1);
	pLLRPCapabilities->setCanReportBufferFillWarning(0);
	pLLRPCapabilities->setClientRequestOpSpecTimeout(300);
	pLLRPCapabilities->setMaxNumAccessSpecs(0);
	pLLRPCapabilities->setMaxNumInventoryParameterSpecsPerAISpec(1);
	pLLRPCapabilities->setMaxNumOpSpecsPerAccessSpec(1);
	pLLRPCapabilities->setMaxNumPriorityLevelsSupported(0);
	pLLRPCapabilities->setMaxNumROSpecs(10);
	pLLRPCapabilities->setMaxNumSpecsPerROSpec(1);
	pLLRPCapabilities->setSupportsClientRequestOpSpec(0);
	pLLRPCapabilities->setSupportsEventAndReportHolding(0);

	pResp->setLLRPCapabilities(pLLRPCapabilities);

	CRegulatoryCapabilities * pRegulatoryCapabilities = new CRegulatoryCapabilities;
	pRegulatoryCapabilities->setCommunicationsStandard(CommunicationsStandard_US_FCC_Part_15);
	pRegulatoryCapabilities->setCountryCode(1);

	//CUHFBandCapabilities * pUHFBandCapabilities = new CUHFBandCapabilities;
	//pUHFBandCapabilities->setFrequencyInformation();

	//pRegulatoryCapabilities->setUHFBandCapabilities(pUHFBandCapabilities);
	
	pResp->setRegulatoryCapabilities(pRegulatoryCapabilities);

	//CParameter * pAirProtocolLLRPCapabilities = new CC1G2LLRPCapabilities;
	CC1G2LLRPCapabilities * pAirProtocolLLRPCapabilities = new CC1G2LLRPCapabilities;
	pAirProtocolLLRPCapabilities->setCanSupportBlockErase(1);
	pAirProtocolLLRPCapabilities->setCanSupportBlockWrite(1);
	pAirProtocolLLRPCapabilities->setMaxNumSelectFiltersPerQuery(0);
	pResp->setAirProtocolLLRPCapabilities(pAirProtocolLLRPCapabilities);

#endif
	pResp->setMessageID(pMsg->getMessageID());
	DBG_PRINT(DEBUG_ALL,"send_STOP_ROSPEC_RESPONSE"NL);
	controller->sendMessage(pResp);
	delete pResp;



}
