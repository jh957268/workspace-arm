//=============================================================================
//
/// @file  SPrintf.h
/// @brief  SPrint function processing
//
//=============================================================================

#ifndef _SPRINTF_H_
#define _SPRINTF_H_

//=============================================================================

#include  <stdio.h>
#include  "PiTypes.h"

#define  NL "\n"

//=============================================================================

//void	SPrintf( char const *fmt, ... );
void	clipPrintf( char const *fmt, ... );

#define SPrintf(fmt, ...) do {  \
	                               printf(fmt, ## __VA_ARGS__); \
	                             } while (0)

//=============================================================================

#ifdef VERBOSE_SPRINTF
#define SPRINTF(fmt, ...) SPrintf("%s:%d: " fmt, __PRETTY_FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define SPRINTF SPrintf
#endif

extern bool  gbTimestampIsEnabled;  // To turn on/off timestamp

//=============================================================================

#endif

//=============================================================================
