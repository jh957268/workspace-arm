#ifndef __DEFINE_H__
#define __DEFINE_H__

#define FALSE 0
#define TRUE  1

#define	ALC_SUCCESS			0
#define	ALC_FAIL			-1

/* Transport Protocol defines */
#define TRANSPORT_HEADER_SIZE 4
#define TP_SOP_OFFSET         0
#define TP_PAYLOAD_LEN_OFF    1
#define TP_PAYLOAD_LENX_OFF   2
#define TP_PAYLOAD_CKSUM_OFF  3
#define TP_PAYLOAD_OFF        4
#define SOP_BYTE		0xEB
#define EOP_BYTE		0x55
#define ESC_BYTE		0x7D
#define ESC_SOP_BYTE	0x5E
#define ESC_ESC_BYTE	0x5F
#define CS_ESC_BYTE		0xA7

typedef enum
{ 
	RX_SOP,
	RX_LENGTH,
	RX_LENGTHX,
	RX_CKSUM,
	RX_PAYLOAD,
	RX_ERROR,
	RX_UART_ERROR
} RXSTATES;

typedef enum
{ RX_NO_ERROR,
  RX_UNKNOWN_CMD,       // only for return error code alignment
  RX_BAD_CKSUM,
  RX_BAD_LENGTH,
  RX_CANT_EXE,          // only for return error code alignment
  RX_BAD_FRAMING,
  RX_OVERRUN,
  RX_INV_ESC,
  RX_BAD_CCOMMAND,
  RX_BAD_LENGTH1
} RXERRORS;

#define REAL_LENGTH_MASK        0x3F
#define PROCTECT_BITS_MASK      0x03
#define PROCTECT_BITS_HIGH_MASK 0xc0
#define PROCTECT_BITS_SHIFT     0x06

#define GET_MASTER_VER			0x1B
#define GET_SLAVE_VER			0x1C
#define RESCAN_SLAVE			0x1D
#define ANTENNA_MAP_REQ			0x8B
#define ANTENNA_SEL				0x0A
#define GET_SLAVE_STAT			0x98
#define SETINIT_FLAG			0x9D
#define GETINIT_FLAG			0x9E
#define INIT_LED_BAR			0xDA
#define SET_LED_BAR				0xDB
#define SET_LED_BAR_VAL			0xDC

#endif

