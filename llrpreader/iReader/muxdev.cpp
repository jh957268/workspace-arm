

#define STRICT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <process.h>
//#include <conio.h>
//#include <windows.h>

#include "muxdev.h"
#include "muxerror.h"
#include "muxserial.h"

/* -------------------------------------------------------------------- */
/* -------------------------    muxdev    ----------------------------- */
/* -------------------------------------------------------------------- */
Muxdev::Muxdev()
{
}

/* -------------------------------------------------------------------- */
/* --------------------------    ~Tserial     ------------------------- */
/* -------------------------------------------------------------------- */
Muxdev::~Muxdev()
{
}

int Muxdev::SyncSendToSlave(uint8_t *data_buf, int len)
{
	uint8_t try_num;
	int status = ALC_SUCCESS;

	for (try_num = 0; try_num < 3; try_num++)			/* try 3 times before giving up */
	{						
		SendCmdToUart(data_buf, len);
		status = RcvFromSlave();

		if (status == ALC_SUCCESS)
			break;
	}
	return (status);
}

int Muxdev::RcvFromSlave(void)
{
	uint8_t rx_byte;
	int nbr;

	ResetRxFlag();
	rx_lengthx = 0;
	rx_lengtho = 0;
	cksum_byte = 0;
	while (1)
	{
		nbr = Ser1->getChar(&rx_byte);
		if (nbr == 0)
			return (ALC_FAIL);
		RxStateMachine(rx_byte);
		if (rx_state == RX_ERROR || rx_state == RX_UART_ERROR)
			return (ALC_FAIL);
		if ((cmd_received == TRUE) && (rx_error == RX_NO_ERROR))
			return (ALC_SUCCESS);
		else if ((cmd_received == TRUE) && (rx_error != RX_NO_ERROR))
			return (ALC_FAIL);
	}
}

void Muxdev::ResetRxFlag(void)
{
	cmd_received = FALSE;
	sop_flag = FALSE;
	timer_started = FALSE;
	rx_state = RX_SOP;
	rx_error = RX_NO_ERROR;
	rx_buf_index = 0;
}

void Muxdev::RxStateMachine(uint8_t rx_byte)
{
	switch(rx_state)
	{
		case RX_SOP:
			if( (rx_byte == SOP_BYTE) && !cmd_received )
			{
				rx_state= RX_LENGTH;
				sop_flag = TRUE;
			}
			break;

		case RX_LENGTH:
			if( rx_byte != SOP_BYTE )
			{ 
				rx_length= rx_byte;
				rx_lengtho= rx_byte;
				rx_cksum= rx_length;          /* save it first before mask out the high nibble */
				rx_state= RX_LENGTHX;
			}
			else
			{
				// cmd_received = TRUE;
				rx_state= RX_ERROR;
				rx_error = RX_BAD_LENGTH;
				cnt_bad_len++;
			}
			break;
		case RX_LENGTHX:
			rx_lengthx = rx_byte;
			rx_cksum^= rx_byte;
			if( (rx_byte == SOP_BYTE) ||  ((~rx_byte & 0x7f) != rx_length))
			{ 
				// cmd_received = TRUE;
				rx_state= RX_ERROR;
				rx_error = RX_BAD_LENGTH1;
				cnt_bad_len++;
			}
			else
			{
				rx_state = RX_CKSUM;
			}
			break;

		case RX_CKSUM:
			cksum_byte = rx_byte;
			rx_length--;
			rx_buf_index = 0;
			rx_state= RX_PAYLOAD;
			break;

		case RX_PAYLOAD:
			rx_cksum^= rx_byte;
			rx_length--;
			if( last_rx_esc_char )
			{ 
				if( rx_byte == ESC_SOP_BYTE )
					rx_buf[rx_buf_index++]= SOP_BYTE;
				else if( rx_byte == ESC_ESC_BYTE )
					rx_buf[rx_buf_index++]= ESC_BYTE;
				else
				{ 
					// cmd_received= TRUE;
					rx_state= RX_ERROR;
					rx_error= RX_INV_ESC;
					cnt_inv_esc++;
				}
			}
			else
			{
				if( rx_byte != ESC_BYTE )
					rx_buf[rx_buf_index++]= rx_byte;
			}
			if (rx_length == 0)
			{
				if( ((rx_cksum+cksum_byte) & 0xFF) == 0x00 )
				{
					cmd_received= TRUE;
				}
				else if (cksum_byte == CS_ESC_BYTE)
				{
					if( (((rx_cksum + SOP_BYTE) & 0xFF) == 0x00) || (((rx_cksum + ESC_BYTE) & 0xFF) == 0x00) )
					{
						cmd_received= TRUE;
					}
					else
					{
						rx_error= RX_BAD_CKSUM;
						cmd_received= TRUE;
						cnt_bad_cksm++;
					}
				}
				else
				{
					rx_error= RX_BAD_CKSUM;
					cmd_received= TRUE;
					cnt_bad_cksm++;
				}
			}
			break;
		case RX_ERROR:
		case RX_UART_ERROR:
			break;				/* stuck here until rx timeout */

		default:
			break;
	}
	last_rx_esc_char= (rx_byte == ESC_BYTE);
}

uint8_t Muxdev::CalcCksum( uint8_t tx_length )
{
	uint8_t i;
	uint8_t sum=0;

	for (i= TP_PAYLOAD_LEN_OFF; i<tx_length; i++)
	{ 
		sum^= tx_buf[i]; //xor sum
	}
	return (~sum)+1;  // 2's complement= 1's complement+1
}

void Muxdev::SendCmdToUart( uint8_t *cmd_buf, uint8_t tx_length )
{
	uint8_t i;

	tx_buf[TP_SOP_OFFSET]= SOP_BYTE;
	tx_index= TP_PAYLOAD_OFF;

	for( i=0; i<tx_length; i++)
	{ 
		if( cmd_buf[i] == SOP_BYTE )
		{ 
			tx_buf[tx_index++]= ESC_BYTE;
			tx_buf[tx_index++]= ESC_SOP_BYTE;
		}
		else if( cmd_buf[i] == ESC_BYTE )
		{ 
			tx_buf[tx_index++]= ESC_BYTE;
			tx_buf[tx_index++]= ESC_ESC_BYTE;
		}
		else
		{ 
			tx_buf[tx_index++]= cmd_buf[i];
		}
	}
	/* JOO, was 2 before, now remove the EOP byte, 1 is for cksum byte */
	hLength = tx_index - TRANSPORT_HEADER_SIZE + 1;
	tx_buf[TP_PAYLOAD_LEN_OFF] = hLength;
	tx_buf[TP_PAYLOAD_LENX_OFF] = (~hLength & 0x7f);
	tx_buf[TP_PAYLOAD_CKSUM_OFF]= 0;
	tx_buf[TP_PAYLOAD_CKSUM_OFF]= CalcCksum(tx_index);
	if (tx_buf[TP_PAYLOAD_CKSUM_OFF] == SOP_BYTE || tx_buf[TP_PAYLOAD_CKSUM_OFF] == ESC_BYTE)
		tx_buf[TP_PAYLOAD_CKSUM_OFF]= CS_ESC_BYTE;
	StartUartOutput(tx_buf, (int)tx_index);
}

void Muxdev::StartUartOutput(uint8_t *tx_buf, int tx_index)
{
#if 0
	int i;

	for (i = 0; i < tx_index; i++)
	{
		sendChar((unsigned char) tx_buf[i]);
	}
#endif
	Ser1->clearRcv();
	Ser1->sendArray(tx_buf, tx_index);
}

Int32 Muxdev::SetAntPort(Int32 port)
{
	int ret;
	unsigned char cmd_buff[64];
	cmd_buff[0] = ANTENNA_SEL;
	cmd_buff[1] = (unsigned char)port;
	ret = SyncSendToSlave(cmd_buff, 2);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	if (rx_buf[0] != ANTENNA_SEL)
		return (MUX_SLAVE_NO_RESPONSE);
	return ((Int32)rx_buf[1]);
}

Int32 Muxdev::SetLedPort(Int32 ledid)
{
	int ret;
	unsigned char cmd_buff[64];
	cmd_buff[0] = SET_LED_BAR_VAL;
	cmd_buff[1] = ~((unsigned char)ledid);
	cmd_buff[2] = ~((unsigned char)(ledid >> 8));
	ret = SyncSendToSlave(cmd_buff, 3);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	if (rx_buf[0] != SET_LED_BAR_VAL)
		return (MUX_SLAVE_NO_RESPONSE);
	return ((Int32)rx_buf[1]);
}

Int32 Muxdev::GetAntMap(uint8_t *map)
{
	int ret;
	unsigned char cmd_buff[32];

	cmd_buff[0] = ANTENNA_MAP_REQ;
	ret = SyncSendToSlave(cmd_buff, 1);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
#if 0
	// sprintf_s((char *)map, 32, "Antenna Map:\n");
	map[0] = 0;
	for (i = 0; i < (rx_buf_index - 1); i++)
	{
		sprintf_s(tmp_buff, 32, "%02x ", rx_buf[i + 1]);
		strcat((char *)map, tmp_buff);
	}
#else
	memmove((void *)map, (void *)&rx_buf[1], 32);
	map[32] = 0;
#endif
	return (MUX_SUCCESS);
}

Int32 Muxdev::RescanSlave(Int32 chn)
{
	int ret;
	unsigned char cmd_buff[64];
	cmd_buff[0] = RESCAN_SLAVE;
	cmd_buff[1] = (unsigned char)chn;
	ret = SyncSendToSlave(cmd_buff, 2);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	return (MUX_SUCCESS);
}

Int32 Muxdev::GetMasterSoftVer(Int32 *ver)
{
	int ret;
	unsigned char cmd_buff[16];
	cmd_buff[0] = GET_MASTER_VER;
	ret = SyncSendToSlave(cmd_buff, 1);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	*ver = rx_buf[1];
	return (MUX_SUCCESS);
}

Int32 Muxdev::GetSlaveSoftVer(Int32 rfout, Int32 slave_no, Int32 *ver)
{
	int ret;
	unsigned char cmd_buff[16];
	cmd_buff[0] = GET_SLAVE_VER;
	cmd_buff[1] = (uint8_t)rfout;
	cmd_buff[2] = (uint8_t)slave_no;
	ret = SyncSendToSlave(cmd_buff, 3);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	*ver = rx_buf[1];
	return (MUX_SUCCESS);
}

void Muxdev::GetRxBuffData(int *len, uint8_t *buff)
{
	*len = (int)rx_buf_index;
	memmove(buff, rx_buf, rx_buf_index);
}

Int32 Muxdev::GetSlaveStat(uint8_t *slaveStats)
{
	int ret;
	unsigned char cmd_buff[16];

	cmd_buff[0] = GET_SLAVE_STAT;
	ret = SyncSendToSlave(cmd_buff, 1);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	memmove((void *)slaveStats, (void *)&rx_buf[1], 32);
	slaveStats[32] = 0;
	return (MUX_SUCCESS);
}

Int32 Muxdev::SetInitdFlag(UInt32 val)
{
	int ret;
	unsigned char cmd_buff[16];
	cmd_buff[0] = SETINIT_FLAG;
	cmd_buff[1] = (unsigned char)(val & 0xFF);
	ret = SyncSendToSlave(cmd_buff, 2);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	if (rx_buf[0] != SETINIT_FLAG)
		return (MUX_SLAVE_NO_RESPONSE);
	return ((Int32)rx_buf[1]);
}

Int32 Muxdev::GetInitdFlag(UInt32 *val)
{
	int ret;
	unsigned char cmd_buff[16];
	cmd_buff[0] = GETINIT_FLAG;
	ret = SyncSendToSlave(cmd_buff, 2);
	if (ret < 0)
	{
		return (MUX_COMMAND_FAIL);
	}
	if (rx_buf[0] != GETINIT_FLAG)
		return (MUX_SLAVE_NO_RESPONSE);
    *val = rx_buf[1];
	return (MUX_SUCCESS);
}
