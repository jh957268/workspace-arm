/*
 * File: AsciiDriver.h
 * Author: SPAIK
 *
 * Translates between bits and ASCII.
 * 
 * 26 Apr 2004: Created.
 *
 * Copyright ThingMagic LLC 2004
 */


#ifndef ASCIIDRIVER_H
#define ASCIIDRIVER_H

#include "HostTypes.h"

class AsciiDriver
{
public:
  AsciiDriver(void){};
  ~AsciiDriver(void){};

  char  Nibble2Ascii(u32 binData, u8 nibblePos);
  char  Nibble2Ascii(u16 binData, u8 nibblePos);
  char  Nibble2Ascii(u8  binData, u8 nibblePos);
  bool  Word2Ascii(u16 binData, char *asciiOut);
  s8    Ascii2Nibble(char asciiData);
  bool  Ascii2Byte(char *asciiData, u8 *byteOut);
};

#endif // #ifndef ASCIIDRIVER_H