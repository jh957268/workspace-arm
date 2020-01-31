/*
 * File: CrcUtils.h
 * Author: SPaik
 *
 * 22 Jun 2004: Created.
 *
 * Copyright ThingMagic LLC 2004
 *
 * This file contains a basic CRC calculator that works on a single byte at a 
 *  time.
 */

#ifndef CRCUTILS_H
#define CRCUTILS_H

#include "HostTypes.h"


#ifdef  __cplusplus
extern "C" {
#endif

void CRC_calcCrc8(u16 *crcReg, u16 poly, u16 u8Data);

#ifdef  __cplusplus
}
#endif


#endif // #ifndef CRCUTILS_H
