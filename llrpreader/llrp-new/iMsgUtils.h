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

// Format of command or response data unit message
/*
	uint8_t buff [] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};
	command or response and real data starts at buff[6]
	buff[6] : command or response code
	buff[7] : real data starts from here
	BTW (in the future): buff[7] should be status, and real data should start at buff[8]
	data len is two bytes : count command or response and real data, that is 1(command or response)  + read data length
	buff[2] : high bye of data len
	buff[3] : low byte of data len
	buff[4] : ~high bye of data len (bit reverse)
	buff[5] : ~low byte of data len	 (bit reverse)

*/

// datalen: total len in the data array + 1 (opcode)
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
