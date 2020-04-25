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
#define MSG_MAX_DATA_LENGTH		512
#define MSG_CRC_INIT			0xFF
#define MSG_CCITT_CRC_POLY		0x1021

// Format of command or response data unit message
/*
	uint8_t buff [] = {HDR1, HDR2, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00};
	command or response and real data starts at buff[6]
	buff[6] : command or response code
	buff[7] : real data starts from here
	BTW (in the future): buff[7] should be status, and real data should start at buff[8]
	data len is two bytes : count command or response and real data
	buff[2] : high bye of data len
	buff[3] : low byte of data len
	buff[4] : ~high bye of data len (bit reverse)
	buff[5] : ~low byte of data len	 (bit reverse)

*/

// datalen: total len in the data array + 1 (opcode)

typedef struct MsgObj
{
	u16	dataLen;
	u8  opCode;
	u8 	dummy;
	u8  data[MSG_MAX_DATA_LENGTH];
} MsgObj;

#ifdef  __cplusplus
}
#endif


#endif // #ifndef MSGUTILS_H
