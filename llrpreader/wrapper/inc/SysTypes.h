//=============================================================================
//
// @file SysTypes.h
//
// This file is a system wide include file. It contains all the atomic data
// type definitions that must be used throughout the system. It may also
// contain enumeration of error codes to be used throughout the system.
//
//=============================================================================

#ifndef SYSTYPES_H
#define SYSTYPES_H

//=============================================================================

// By default in Windows the padding is for 8 bytes, not compatible
// with Linux, so ensure that packing is done for 4 bytes

#pragma pack (4)

//=============================================================================

#ifdef TESTER_APP

#include <windows.h>

#else

typedef int SOCKET;
typedef int WSADATA;

#define SOCKET_ERROR		-1
#define INVALID_SOCKET		-1

typedef char            CHAR;   // 8 bit for string
typedef char            INT8;   // 8 bit integer
typedef short           INT16;  // 16 bit integer
typedef unsigned short  UINT16; // 16 bit unsigned integer
typedef int             INT32;  // 32 bit integer
typedef unsigned int    UINT32; // 32 bit unsigned integer
typedef long long       INT64;  // 64 bit integer
typedef unsigned long long UINT64;  // 64 bit integer
typedef unsigned long long __time64_t;  // 64 bit integer
#ifdef __cplusplus
typedef bool            BOOL;   // Boolean value
#endif
typedef unsigned char   UINT8;  // 8 bit unsigned integer

//---------------------------------------------------------

typedef char            	BYTE;
typedef unsigned char   	UBYTE;
typedef short int         	WORD;
typedef unsigned short int 	UWORD;
typedef long int            DWORD;
typedef unsigned long int   UDWORD;

typedef long long			LL;

#ifdef __cplusplus
typedef volatile bool		VBOOL;
#endif

typedef volatile BYTE		VBYTE;
typedef volatile UBYTE		VUBYTE;
typedef volatile WORD		VWORD;
typedef volatile UWORD		VUWORD;
typedef volatile DWORD		VDWORD;
typedef volatile UDWORD		VUDWORD;
typedef volatile float		VFLOAT;
typedef volatile double     VDOUBLE;

typedef BYTE*            	PBYTE;
typedef UBYTE*   			PUBYTE;
typedef WORD*				PWORD;
typedef UWORD*				PUWORD;
typedef DWORD*				PDWORD;
typedef UDWORD*				PUDWORD;
typedef float*				PFLOAT;
typedef double*          	PDOUBLE;

typedef volatile BYTE*		PVBYTE;
typedef volatile UBYTE*  	PVUBYTE;
typedef volatile WORD*		PVWORD;
typedef volatile UWORD*		PVUWORD;
typedef volatile DWORD*		PVDWORD;
typedef volatile UDWORD*	PVUDWORD;
typedef volatile float*		PVFLOAT;
typedef volatile double*	PVDOUBLE;

#endif

//---------------------------------------------------------

enum  eCPRI_LineRate
{
	eCPRI_E6,	//  614.4 Mbps (Line Rate 1)
	eCPRI_E12,	// 1228.8 Mbps (Line Rate 2)
	eCPRI_E24,	// 2457.7 Mbps (Line Rate 3)
	eCPRI_E30,	// 3072.0 Mbps (Line Rate 4)
	eCPRI_E48,	// 4915.2 Mbps (Line Rate 5)
	eCPRI_E60,	// 6144.0 Mbps (Line Rate 6)
	eCPRI_E96	// 9830.4 Mbps (Line Rate 7)
};

//---------------------------------------------------------
// @brief Timestamp information

typedef struct CommonTimestamp
{
    INT64 tm_sec;   // seconds elapsed since 00:00:00 on Jan 1, 1970 */
    INT32 tm_msec;  // milliseconds elapsed since 00:00:00

} TM_STAMP;

//=============================================================================

#endif

//=============================================================================
