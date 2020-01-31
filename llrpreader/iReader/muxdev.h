#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "define.h"

enum serial_parity  { spNONE,    spODD, spEVEN };

class Muxdev
{
	protected:
		uint8_t	cmd_received;
		uint8_t tx_buf[128];
		uint8_t tx_index;
		uint8_t rx_buf[128];
		uint8_t rx_buf_index;
		uint8_t cnt_bad_len;
		uint8_t cnt_bad_cksm;
		uint8_t cnt_inv_esc;
		uint8_t cnt_overrun;
		uint8_t cnt_framing;
		RXERRORS rx_error;
		RXSTATES rx_state;
		uint8_t hLength;
		uint8_t rx_length, rx_lengthx, rx_cksum, cksum_byte,rx_lengtho;
		uint8_t last_rx_esc_char;
		uint8_t sop_flag, timer_started;

		// void 	StartUartOutput(uint8_t *tx_buf, int tx_index);
		int 	RcvFromSlave(void);
		void 	ResetRxFlag(void);
		void 	RxStateMachine(uint8_t rx_byte);
		uint8_t CalcCksum( uint8_t tx_length );
		void 	SendCmdToUart( uint8_t *cmd_buf, uint8_t tx_length );
		
	public:
					  Muxdev();
		virtual		  ~Muxdev();
    	virtual int	  Connect(void) = 0;
    	virtual void  Disconnect(void) = 0;
    	virtual int   sendChar(char c) = 0;
    	virtual int   sendArray(uint8_t *array, int len) = 0;
    	virtual int   getChar(unsigned char *ch) = 0;
    	virtual void  flushRcvBuffer(void) = 0;
		
		void 	StartUartOutput(uint8_t *tx_buf, int tx_index);
		int 	SyncSendToSlave(uint8_t *data_buf, int len);
		void	GetRxBuffData(int *len, uint8_t *buff);

		Int32 	SetAntPort(Int32 port);
		Int32 	GetAntMap(uint8_t *map);
		Int32 	RescanSlave(Int32 chn);
		Int32 	GetMasterSoftVer(Int32 *ver);
		Int32 	GetSlaveSoftVer(Int32 rfout, Int32 slave_no, Int32 *ver);
		Int32 	GetSlaveStat(uint8_t *slaveStats);
		Int32	SetInitdFlag(UInt32 val);
		Int32	GetInitdFlag(UInt32 *val);
		Int32 	SetLedPort(Int32 ledid);
};

#endif
