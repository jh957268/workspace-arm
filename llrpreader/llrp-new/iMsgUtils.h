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

#ifndef IMSGUTILS_H
#define IMSGUTILS_H

#ifdef  __cplusplus
extern "C" {
#endif

#define IMSG_MAX_PACKET_LEN    	256
#define IMSG_MAX_DATA_LENGTH		122
#define IMSG_CRC_INIT			0xFF
#define IMSG_CCITT_CRC_POLY		0x1021

typedef struct iMsgObj
{
	u_int16_t	dataLen;
	u_int8_t  	opCode;
	u_int8_t 	dummy;
	u_int8_t  	data[IMSG_MAX_DATA_LENGTH];
} iMsgObj;

int  iAgent_receiveMsgObj(iMsgObj *hMsg);
void iAgent_sendMsgObj(iMsgObj *hMsg);


#ifdef  __cplusplus
}
#endif


#endif // #ifndef MSGUTILS_H
