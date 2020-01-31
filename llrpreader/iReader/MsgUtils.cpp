/*
 * File: MsgUtils.cpp
 * Author: SPaik
 *
 * 24 Jun 2004: Created.
 *
 * Copyright ThingMagic LLC 2004
 *
 * This file contains utilities for parsing and moving messges around the system
 */
//#include "stdafx.h"
#include "stdio.h"
#include "CrcUtils.h"
#include "HostTypes.h"
#include "MsgUtils.h"
#include "define.h"
#include "muxapi.h"



/** @fn bool MSG_receiveMsgObj(MsgObj *hMsg)
  * @brief Receives a valid message object from the serial port.
  *
  *         This function will print the contents of a valid message to the
  *         console.
  *
  *         Note: This function will block until enough characters are received
  *                 over the serial port.  If this is not desired, then this
  *                 function should be changed to include a timeout.
  *
  * @param hMsg   Pointer to a message object to fill in.
  * @return true if a valid message is received.
  */




/** @fn u16 MSG_calcCrcToDsp(const MsgObj *hMsg)
  * @brief Calculates the CRC on a message sent to the DSP.
  *
  *         The CRC calculations are slightly different between messages sent
  *         to the DSP and messages received from the DSP because of the status
  *         field.  The status field is not used when transmitting to the DSP.
  *
  * @param hMsg   Pointer to message to calculate CRC of.
  * @return CRC for the message object.
  */
u16 MSG_calcCrcToDsp(const MsgObj *hMsg)
{
  u16 calcCrc = MSG_CRC_INIT;
  u8  i;

  CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->dataLen);
  CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->opCode);


  for(i=0; i<hMsg->dataLen; i++)
  {
    CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->data[i]);
  }

  return(calcCrc);
}


/** @fn u16 MSG_calcCrcFromDsp(const MsgObj *hMsg)
  * @brief Calculates the CRC on a message received from the DSP.
  *
  *         The CRC calculations are slightly different between messages sent
  *         to the DSP and messages received from the DSP because of the status
  *         field.  The status field is not used when transmitting to the DSP.
  *
  * @param hMsg   Pointer to message to calculate CRC of.
  * @return CRC for the message object.
  */
u16 MSG_calcCrcFromDsp(const MsgObj *hMsg)
{
  u16 calcCrc = MSG_CRC_INIT;
  u8  i;

  CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->dataLen);
  CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->opCode);
  //CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->status[0]);
  //CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->status[1]);

  for(i=0; i<hMsg->dataLen; i++)
  {
    CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, hMsg->data[i]);
  }

  return(calcCrc);
}


/** @fn bool MSG_checkCrc(MsgObj *hMsg)
  * @brief Simple utility to validate CRC of message received from DSP.
  *
  * @param hMsg   Pointer to message to calculate CRC of.
  * @return true if CRC is correct.
  */
bool MSG_checkCrc(MsgObj *hMsg)
{
  u16 calcCrc;

  calcCrc = MSG_calcCrcFromDsp(hMsg);

  return(calcCrc == hMsg->crc);
}

