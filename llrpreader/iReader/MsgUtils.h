/*
 * File: MsgUtils.h
 * Author: SPaik
 *
 * 21 Jun 2004: Created.
 *
 * Copyright ThingMagic LLC 2004
 *
 * This file contains utilities for parsing and moving messges around the system
 */

#ifndef MSGUTILS_H
#define MSGUTILS_H

#include "HostTypes.h"



#ifdef  __cplusplus
extern "C" {
#endif

#define MSG_MAX_PACKET_LEN    	256
#define MSG_MAX_DATA_LENGTH		122
#define MSG_CRC_INIT			0xFF
#define MSG_CCITT_CRC_POLY		0x1021

typedef struct MsgObj
{
	u16	dataLen;
	u8  opCode;
	u8 	crc;
	u8  cr;
	u8  lf;
	u8  data[MSG_MAX_DATA_LENGTH];
} MsgObj;

int  MSG_receiveMsgObj(Uint32 muxh, MsgObj *hMsg);
void MSG_sendMsgObj(Uint32 muxh, MsgObj *hMsg);
bool MSG_checkCrc(MsgObj *hMsg);
u16  MSG_calcCrcFromDsp(const MsgObj *hMsg);
u16  MSG_calcCrcToDsp(const MsgObj *hMsg);


#ifdef  __cplusplus
}
#endif


#endif // #ifndef MSGUTILS_H
