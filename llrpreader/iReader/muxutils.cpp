#include "conio.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 

#include <sys/stat.h> 

#include <fcntl.h> 
#include "define.h"
#include "muxclient.h"
#include "muxserial.h"
#include "muxerror.h"

unsigned char ReadBuffer[16 * 1024];
DWORD dwBytesRead = 0;
unsigned int slaveSize;
unsigned int masterSize;
int dwBytesLeft;
Muxdev *gDevHandle = 0;

static int rx_buf_index;
static uint8_t rx_buf[256];

int start_download(int flag);
void start_slave_download(int rfout, int slave_no, unsigned int size);
uint8_t  exe_command (uint8_t *buf);
int master_load (int argc, char **argv, void *ptr);
int slave_load (int argc, char **argv, void *ptr);
int get_master_ver (int argc, char **argv, void *ptr);
int get_slave_ver (int argc, char **argv, void *ptr);
int slave_loader (int argc, char **argv, void *ptr);
int master_loader (int argc, char **argv, void *ptr);
int get_slave_stat (int argc, char **argv, void *ptr);
int mux_active (int argc, char **argv, void *ptr);
int power_rfout (int argc, char **argv, void *ptr);
int test_reader (int argc, char **argv, void *ptr);
int set_baudrate (int argc, char **argv, void *ptr);
int diag_loop (int argc, char **argv, void *ptr);

#define MAP_SIZE			128
#define BLOCK_SIZE			64
#define PGM_MAGIC			0xAB
#define ACK_MAGIC			0xB5
#define VERIFY_MAGIC		0xD6
#define VERIFY_ACK_MAGIC	0x6D
#define NAK_ACK_MAGIC		0x5A
#define SLAVE_PWR_ON		0x8C
#define SLAVE_PGM_MODE		0x8D
#define MASTER_PGM_MODE		0x8F
#define DIAG_LOOP		    0xD9

#define MAGIC_LOC		0x0
#define PAGE_LOC		0x1
#define BLOCK_LOC		0x2
#define DATA_LOC		0x3

#define SLAVE_MAGIC_LOC		0x0
#define SLAVE_RFOUT_LOC		0x1
#define SLAVE_ADDR_LOC		0x2
#define SLAVE_PAGE_LOC		0x3
#define SLAVE_BLOCK_LOC		0x4
#define SLAVE_DATA_LOC		0x5

#define SLAVE_MAP_SIZE		64
#define SLAVE_BLOCK_SIZE	32

#define ECHO_REQ			0xB1
#define GET_MASTER_VER		0x1B
#define GET_SLAVE_VER		0x1C
#define RESCAN_SLAVE		0x1D
#define SET_IMAGE_SIZE		0x97
#define GET_SLAVE_STAT		0x98

#if 0
To program the controller board:
First byte : PGM_MAGIC
Second byte : Page num
Third bye : block number (0, 1)
Data : 64 byte 

Total packet length : 64 + 3

To program the slave board:
First byte : PGM_MAGIC
Second byte : rfout number	(0..7)
Third bye : slave number	(0..3)
fourth byte : Page num
fifth byte : block number (0, 1)
Data : 32 byte 

Total packet length : 32 + 5
#endif

#define END_OF_LINE(x)  (((x) == '\n') ? 1 : (((x) == '\r') ? 1 : 0))
#define NUMBER_OF_COMMAND_TABLES	20
#define	BUF_SIZE	128 

typedef struct _console_command
{
	const char *opcode;
	int (*c_func) (int argc, char **argv, void *cmds);	/* function to call */

} Console_Command_t;

void wait_user_input(void);
int  parse_command (uint8_t *buf, int *argc, char **argv);


char *retBuff;
char printBuf[512];

typedef void (__stdcall *FNPTR)(char *stringVar);
FNPTR CallBackPrint;
FNPTR CallBackMsg;

extern Int32 MuxHandleValid(UInt32 handle);

long __stdcall send_command(UInt32 handle, char *command, char result[256], unsigned long funct, unsigned long msgbox)
{
	retBuff = (char *)result;
	char tmp[128];
	Int32 ret;

	retBuff[0] = 0;

	CallBackPrint = (FNPTR)funct;
	CallBackMsg = (FNPTR)msgbox;

	ret = MuxHandleValid(handle);
	if (ret != MUX_SUCCESS)
	{
		sprintf_s(tmp, "Invalid Mux device handle.");
		strcat(retBuff, tmp);
		return (ret);
	}
	else
	{
		gDevHandle = (Muxdev *)handle;
		strcat(command, "\n");
		if (exe_command((uint8_t *)command) != TRUE)
		{
			sprintf_s(tmp, "Invalid command.");
			strcat(retBuff, tmp);
		}
	}
	return 1;
}

int start_download(int flag)
{
	int page, block, offset;
	unsigned char data_buff[256], cmd_buff[128], magic;
	int ret;

	dwBytesLeft = (int)masterSize;

	page = 0;
	while (dwBytesLeft > 0)
	{
		if (flag == 0)
		{
			sprintf_s (printBuf,"Downloading page %d...\n", page);
			CallBackPrint(printBuf);
			magic = PGM_MAGIC;
		}
		else
		{
			sprintf_s (printBuf, "Verifying page %d...\n", page);
			CallBackPrint(printBuf);
			magic = VERIFY_MAGIC;
		}
		offset = (page * MAP_SIZE);
		memmove(data_buff, &ReadBuffer[offset], MAP_SIZE);
		for (block = 0; block < 2; block++)
		{
			cmd_buff[MAGIC_LOC] = magic;
			cmd_buff[PAGE_LOC] = page;
			cmd_buff[BLOCK_LOC] = block;
			memmove(&cmd_buff[DATA_LOC], &data_buff[(block * 64)], MAP_SIZE/2);
			ret = gDevHandle->SyncSendToSlave(cmd_buff, BLOCK_SIZE + 3);
			if (ret < 0)
			{
				sprintf_s(printBuf, "File download failed\n");
				CallBackPrint(printBuf);
				return (-1);
			}
			gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
			sprintf_s(printBuf, "rx_buf_idx = %d, %02x %02x %02x\n", rx_buf_index, rx_buf[MAGIC_LOC], 
				rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
			CallBackPrint(printBuf);
			if ((rx_buf[MAGIC_LOC] != ACK_MAGIC && rx_buf[MAGIC_LOC] != VERIFY_ACK_MAGIC) || 
					rx_buf[PAGE_LOC] != page || rx_buf[BLOCK_LOC] != block)
			{
				sprintf_s(printBuf,"File download recived failed\n");
				CallBackPrint(printBuf);
				return (-1);
			}
		}
		page++;
		dwBytesLeft -= MAP_SIZE;
	}

	sprintf_s(printBuf,"File download succeed.\n");
	CallBackPrint(printBuf);
	return 0;
}

int start_download_to_slave(int rfout, int slave_no, int flag)
{
	int page, block, offset;
	unsigned char data_buff[256], cmd_buff[128], magic;
	int ret;

	dwBytesLeft = (int)slaveSize;

	page = 0;
	while (dwBytesLeft > 0)
	{
		if (flag == 0)
		{
			sprintf_s (printBuf,"Downloading page %d...\n", page);
			CallBackPrint(printBuf);
			magic = PGM_MAGIC;
		}
		else
		{
			sprintf_s (printBuf, "Verifying page %d...\n", page);
			CallBackPrint(printBuf);
			magic = VERIFY_MAGIC;
		}
		offset = (page * SLAVE_MAP_SIZE);
		memmove(data_buff, &ReadBuffer[offset], SLAVE_MAP_SIZE);

		for (block = 0; block < 2; block++)
		{
			// Sleep(1000);
			cmd_buff[SLAVE_MAGIC_LOC] = magic;
			cmd_buff[SLAVE_RFOUT_LOC] = rfout;
			cmd_buff[SLAVE_ADDR_LOC] = slave_no;
			cmd_buff[SLAVE_PAGE_LOC] = page;
			cmd_buff[SLAVE_BLOCK_LOC] = block;
			memmove(&cmd_buff[SLAVE_DATA_LOC], &data_buff[(block * 32)], SLAVE_MAP_SIZE/2);
			ret = gDevHandle->SyncSendToSlave(cmd_buff, SLAVE_BLOCK_SIZE + 5);
			if (ret < 0)
			{
				sprintf_s(printBuf, "File download failed\n");
				CallBackPrint(printBuf);
				return (-1);
			}
			gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
			sprintf_s(printBuf, "rx_buf_idx = %d, %02x %02x %02x\n", rx_buf_index, 
				rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
			CallBackPrint(printBuf);

			if ((rx_buf[MAGIC_LOC] != ACK_MAGIC && rx_buf[MAGIC_LOC] != VERIFY_ACK_MAGIC) || 
					rx_buf[PAGE_LOC] != page || rx_buf[BLOCK_LOC] != block)
			{
				sprintf_s(printBuf, "File download recived failed\n");
				CallBackPrint(printBuf);
				return (-1);
			}
		}
		page++;
		dwBytesLeft -= SLAVE_MAP_SIZE;
	}

	sprintf_s(printBuf, "Slave File download succeed.\n");
	CallBackPrint(printBuf);

	return 0;
}

void start_slave_download(int rfout, int slave_no, unsigned int size)
{
	unsigned char cmd_buff[128];
	int ret, c;

	cmd_buff[0] = SLAVE_PWR_ON;
	cmd_buff[1] = (unsigned char)rfout;
	cmd_buff[2] = (unsigned char)slave_no;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 3);
	if (ret < 0)
	{
		sprintf_s(printBuf, "Command failed : SLAVE_PWR_ON\n");
		CallBackPrint(printBuf);
		return;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(printBuf, "rx_buf_idx = %d, %02x %02x %02x\n", rx_buf_index, 
		rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	CallBackPrint(printBuf);
	sprintf_s(printBuf, "Slave download !, press any key to continue.\n");
	CallBackMsg(printBuf);

	sprintf_s(printBuf, "Start Download to Slave\n");
	CallBackMsg(printBuf);
	start_download_to_slave(rfout, slave_no, 0);
	start_download_to_slave(rfout, slave_no, 1);

	sprintf_s(printBuf, "Program the Image Size...\n");
	CallBackPrint(printBuf);
	cmd_buff[0] = SET_IMAGE_SIZE;
	cmd_buff[1] = (unsigned char)rfout;
	cmd_buff[2] = (unsigned char)slave_no;
	cmd_buff[3] = (unsigned char)((size >> 8) & 0xff);
	cmd_buff[4] = (unsigned char)(size & 0xff);
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 5);
	if (ret < 0)
	{
		sprintf_s(printBuf, "Command failed : SLAVE_IMAGE_SIZE\n");
		CallBackPrint(printBuf);
		return;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(printBuf, "Success, rx_buf_idx = %d, %02x %02x %02x\n", 
		rx_buf_index, rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	CallBackPrint(printBuf);
}

const Console_Command_t Commands_Table[]= {
//	{"echo", echo_test},
	{"master", master_load},
	{"slave", slave_load},
	{"getmver", get_master_ver},
	{"getsver", get_slave_ver},
	{"sload", slave_loader},
	{"mload", master_loader},
	{"getslave", get_slave_stat},
	{"active", mux_active},
	{"pwr", power_rfout},
	{"reader", test_reader},
	{"diag", diag_loop},
	{0, 0}
};

#define PAR_NUM   10 
uint8_t  exe_command (uint8_t *buf)
{
	char  *argv[PAR_NUM];
	int argc, not_found = 1;
	Console_Command_t *cmds, *gl_cmds;
	uint8_t ret;
	// char tmp[128];
	if (parse_command(buf, &argc, argv) == TRUE)
	{
		// strcat(retBuff, "\n");
		// strcat(retBuff, "OK");
		gl_cmds =  (Console_Command_t *)&Commands_Table[0];
		for (cmds = &gl_cmds[0]; cmds->opcode; cmds++)
		{
			//sprintf(tmp,"\nargc=%d", argc);
			//strcat(retBuff, tmp);
			// strcat(retBuff, cmds->opcode);
			//strcat(retBuff, "\n");
			//strcat(retBuff, argv[1]);
			not_found = strcmp(cmds->opcode, argv[0]);
			if (!not_found)
			{
				// printf("\n\r");
				ret = (*cmds->c_func)(argc, argv, cmds);
				//strcat(retBuff, "\n");
				//strcat(retBuff, "BREAK");
				break;  
			}
		}

		if (not_found)
		{
			return (FALSE);
		}
	}
	return(TRUE);
}

int  parse_command (uint8_t *buf, int *argc, char **argv) 
{
	int   i, end = 0, cnt = 0, empty_line = 1;
	char  ch;
#define CMD_DELIMETER(x) ((x == ' ') || (x == ':') || (x == ';') || (x == ',') || (x == '.'))
  
	argv[0] = (char *)buf;
	*argc = 0;
  
	/* replace indentation chars with zeros */
	for (i = 0; i < BUF_SIZE; i++)
	{
		ch = *buf;
		if (!END_OF_LINE(ch))
		{
			empty_line = 0; /* got at least one character on the line */
			if (CMD_DELIMETER(ch))
			{
				cnt++;
				*buf = 0;
				/* find next nondelimeter char */
				do
				{
					ch = *++buf;
					if (END_OF_LINE(ch))
					{
						*buf = 0;
						end = 1;
						break;
					}
				} while(CMD_DELIMETER(ch));
				argv[cnt] = (char *)buf;
			}
			else
			{
				buf++;
			}
      
			if (end)
			break;
		}
		else
		{ /* replace end of line with 0 */
			*buf = 0; 
			break;
		}
	}
  
	*argc = cnt + 1;
	return((!empty_line) ? TRUE : FALSE);
}

int open_file(char *filename, int flag)
{
	HANDLE hFile;
	unsigned int slave_size;
	int i;

	hFile = CreateFile((LPCSTR)filename,      // file to open
				   GENERIC_READ,          // open for reading
				   FILE_SHARE_READ,       // share for reading
				   NULL,                  // default security
				   OPEN_EXISTING,         // existing file only
				   FILE_ATTRIBUTE_NORMAL, // normal file
				   NULL);                 // no attr. template

	if (hFile == INVALID_HANDLE_VALUE) 
	{ 
		sprintf_s(printBuf, "Could not open file (error %d)\n", GetLastError());
		CallBackPrint(printBuf);
		return (-1); 
	}

	sprintf_s(printBuf,"Open file master.bin for read OK\n");
	CallBackPrint(printBuf);

	memset(ReadBuffer, 0x00, (16 * 1024));
	if( FALSE == ReadFile(hFile, ReadBuffer, (16 * 1024), &dwBytesRead, NULL) )
	{
		sprintf_s(printBuf,"Could not read from file (error %d)\n", GetLastError());
		CallBackPrint(printBuf);
		CloseHandle(hFile);
		return(-1); 
	}

	if (dwBytesRead <= 0)
	{
		sprintf_s(printBuf,"End of file or empty\n");
		CallBackPrint(printBuf);
		CloseHandle(hFile);
		return (-1); 
	}

	sprintf_s(printBuf, "The size of the file is %d\n",  dwBytesRead);
	CallBackPrint(printBuf);

	if (flag == 1)
	{
		unsigned char csum; 
		masterSize = (unsigned int)dwBytesRead;
		masterSize += (MAP_SIZE - 1);
		masterSize &= ~(MAP_SIZE - 1);
		csum = 0;
		for (i = 0; i < int(masterSize -1); i++)
		{
			csum += ReadBuffer[i];
		}
		csum = (~csum + 1);
		ReadBuffer[masterSize - 1] = csum;
		sprintf_s(printBuf, "Master code size is %d. cksum = 0x%02x\n", masterSize, csum);
		CallBackPrint(printBuf);
	}

	if (flag == 2)
	{
		unsigned char csum; 
		slaveSize = (unsigned int)dwBytesRead;
		slaveSize += (SLAVE_MAP_SIZE - 1);
		slaveSize &= ~(SLAVE_MAP_SIZE - 1);
		csum = 0;
		for (i = 0; i < int(slaveSize -1); i++)
		{
			csum += ReadBuffer[i];
		}
		csum = (~csum + 1);
		ReadBuffer[slaveSize - 1] = csum;
		sprintf_s(printBuf, "Slave code size is %d. cksum = 0x%02x\n", slaveSize, csum);
		CallBackPrint(printBuf);
	}
	CloseHandle(hFile);
	return (0); 
}

void flush_rcv_buffer(void)
{
	uint8_t rx_byte;
	int nbr;
	while (1)
	{
		nbr = gDevHandle->getChar(&rx_byte);
		if (rx_byte == 'R')
			break;
	}
	Sleep(1000);
}

int master_load (int argc, char **argv, void *ptr)
{
	HANDLE hFile;
    unsigned char c;
	unsigned char cmd_buff[128];
	int ret;
	// sprintf(printBuf, "%s\n", __FUNCTION__);
	// CallBackPrint(printBuf);

	ret = open_file("master.bin", 1);
	if (ret != 0)
		return 0;
    sprintf_s(printBuf, "Start downloading file to ALC-600\n");
    CallBackPrint(printBuf);
	// Sleep(1000);
	for (;;)
	{
		gDevHandle->getChar(&c);
		if ((char)c == 'S')
		{
			gDevHandle->sendChar('G');
			break;
		}
	}
	flush_rcv_buffer();
	start_download(0);
	start_download(1);

	sprintf_s(printBuf, "Program the Image Size...\n");
	CallBackPrint(printBuf);
	cmd_buff[0] = SET_IMAGE_SIZE;
	cmd_buff[1] = (unsigned char)((masterSize >> 8) & 0xff);
	cmd_buff[2] = (unsigned char)(masterSize & 0xff);
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 3);
	if (ret < 0)
	{
		sprintf_s(printBuf, "Command failed : MASTER_IMAGE_SIZE\n");
		CallBackPrint(printBuf);
		return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(printBuf, "Success, rx_buf_idx = %d, %02x %02x %02x\n", rx_buf_index, 
		rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	CallBackPrint(printBuf);
	return (0);
}

int slave_load (int argc, char **argv, void *ptr)
{
	HANDLE hFile;
	int ret;
	int rfout, slave_no;
	// sprintf(printBuf, "%s\n", __FUNCTION__);
	// CallBackPrint(printBuf);

	if (argc != 3)
	{
		sprintf_s(printBuf, "Invalid number of arguments\n");
    	CallBackPrint(printBuf);
		return 0;
	}
	ret = open_file("slave.bin", 2);
	if (ret != 0)
		return 0;

	rfout = atoi(argv[1]);
	slave_no = atoi(argv[2]);

	sprintf_s(printBuf, "Start downloading file to Slave board, rfout = %d, slave_no = %d\n", rfout, slave_no);
    CallBackPrint(printBuf);
	start_slave_download(rfout, slave_no, (unsigned int)slaveSize);

	return (0);
}

int get_slave_stat (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	int ret, i;

	if (argc != 1)
	{
		sprintf_s(printBuf, "invalid number of arguments\n");
    	CallBackPrint(printBuf);
		return 0;
	}
	cmd_buff[0] = GET_SLAVE_STAT;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 1);
	if (ret < 0)
	{
		sprintf_s(printBuf, "Command fails.\n");
    	CallBackPrint(printBuf);
		return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(printBuf, "Slave installed status (32 bytes):\n");
    CallBackPrint(printBuf);
	for (i = 0; i < (rx_buf_index - 1); i++)
	{
		sprintf_s(printBuf, "%02x ", rx_buf[i + 1]);
    	CallBackPrint(printBuf);
	}
	return 0;
}

int mux_active (int argc, char **argv, void *ptr)
{
	int ret, flag;
	MuxClient *clientHandle = 0;
	unsigned char buffer[32];
	int len;

	if (argc != 2)
	{
		sprintf_s(printBuf, "invalid number of arguments\n");
    	CallBackPrint(printBuf);
		return 0;
	}
	flag = atoi(argv[1]) + 0x10;
	clientHandle = (MuxClient *)gDevHandle;
	if (flag == 0x17)
	{
		ret = clientHandle->SelectModule(MUX_MODULE);
		if (ret < 0)
		{
			sprintf_s(printBuf, "Command fails.\n");
		}
		else
		{
			sprintf_s(printBuf, "select module succeeds.\n"); 
		}
	}
	else if (flag == 0x18)
	{
		ret = clientHandle->SelectModule(READER_MODULE);
		if (ret < 0)
		{
			sprintf_s(printBuf, "Command fails.\n");
		}
		else
		{
			sprintf_s(printBuf, "select module succeeds.\n"); 
		}
	}
	else
	{
		ret = clientHandle->MuxActive(flag, (unsigned char *)buffer, &len);
	 	if (ret < 0)
		{
			sprintf_s(printBuf, "Command fails.\n");
		}
		else
		{
			sprintf_s(printBuf, "Command succeeds. %02x %02x %02x %02x %02x\n", 
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
		}
	}
    CallBackPrint(printBuf);
	return 0;
}


#if 0
int echo_test (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	int ret, i;
	uint8_t rfout, slave_no;

	if (argc != 3)
	{
		printf("invalid number of arguments\n");
		return 0;
	}
	
	rfout = atoi(argv[1]);
	slave_no = atoi(argv[2]);
	cmd_buff[0] = ECHO_REQ;
	cmd_buff[1] = rfout;
	cmd_buff[2] = slave_no;
	cmd_buff[3] = 0xaa;
	ret = sync_send_to_slave(cmd_buff, 4);
	if (ret < 0)
	{
		printf("Command failed\n");
		return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	printf("rx_buf_idx = %d\n", rx_buf_index);
	for (i = 0; i < (rx_buf_index - 1); i++)
		printf("0x%02x ", rx_buf[i + 1]);
	printf("\n");
	return 0;
}
#endif

int get_master_ver (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	char tmp[128];
	int ret;
	int i;
	if (argc != 1)
	{
		strcat(retBuff, "Invalid argument");
		return 0;
	}
	cmd_buff[0] = GET_MASTER_VER;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 1);
	if (ret < 0)
	{
		strcat(retBuff, "Command fails.");
		return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
#if 0
	sprintf_s(tmp, "\nrx_buf_idx = %d, %02x %02x %02x", rx_buf_index, rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	strcat(retBuff, tmp);
#endif
	sprintf_s(tmp,"Master Firmare Version = 0x%02x", rx_buf[PAGE_LOC]);
	strcat(retBuff, tmp);
	return 0;
}


int get_slave_ver (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	char tmp[128];
	int ret;
	int i;
	uint8_t rfout, slave_no;
	if (argc != 3)
	{
		strcat(retBuff, "Invalid arguments.");
		return 0;
	}
	rfout = atoi(argv[1]);
	slave_no = atoi(argv[2]);
	cmd_buff[0] = GET_SLAVE_VER;
	cmd_buff[1] = rfout;
	cmd_buff[2] = slave_no;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 3);
	if (ret < 0)
	{
		strcat(retBuff, "Command fails.");
		return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
#if 0
	sprintf_s(tmp, "\nrx_buf_idx = %d, %02x %02x %02x", rx_buf_index, rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	strcat(retBuff, tmp);
	sprintf_s(tmp,"\nReturn Code = 0x%02x", rx_buf[PAGE_LOC]);
#endif
	sprintf_s(tmp,"Slave %d %d Firmare Version = 0x%02x", rfout, slave_no, rx_buf[PAGE_LOC]);
	strcat(retBuff, tmp);
	return 0;
}

int slave_loader (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	char tmp[128];
	int ret, i;
	uint8_t rfout, slave_no;

	if (argc != 3)
	{
		strcat(retBuff, "Invalid arguments");
		return 0;
	}
	
	rfout = atoi(argv[1]);
	slave_no = atoi(argv[2]);
	cmd_buff[0] = SLAVE_PGM_MODE;
	cmd_buff[1] = rfout;
	cmd_buff[2] = slave_no;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 3);
	if (ret < 0)
	{
		strcat(retBuff, "Command failed");
		return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(tmp, "rx_buf_idx = %d, %02x %02x %02x", rx_buf_index, rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	strcat(retBuff, tmp);
	sprintf_s(tmp,"\nReturn Code = 0x%02x", rx_buf[PAGE_LOC]);
	strcat(retBuff, tmp);
	return 0;
}

int master_loader (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	char tmp[128];
	int ret, i;

	if (argc != 1)
	{
		strcat(retBuff, "Invalid argument.");
		return 0;
	}
	
	cmd_buff[0] = MASTER_PGM_MODE;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 1);
	if (ret < 0)
	{
		strcat(retBuff, "Command fails.");
		return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(tmp, "\nrx_buf_idx = %d, %02x %02x %02x", rx_buf_index, rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	strcat(retBuff, tmp);
	sprintf_s(tmp,"\nReturn Code = 0x%02x", rx_buf[PAGE_LOC]);
	strcat(retBuff, tmp);
	return 0;
}

int power_rfout (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	int ret;
	int rfout, slave_no;
	if (argc != 2)
	{
		strcat(retBuff, "Invalid argument");
		return 0;
	}
	rfout = atoi(argv[1]);
	cmd_buff[0] = SLAVE_PWR_ON;
	cmd_buff[1] = (unsigned char)rfout;
	cmd_buff[2] = (unsigned char)0;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 3);
	if (ret < 0)
	{
		sprintf_s(printBuf, "Command failed : SLAVE_PWR_ON\n");
		CallBackPrint(printBuf);
	    return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(printBuf, "rx_buf_idx = %d, %02x %02x %02x\n", rx_buf_index, 
		rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	CallBackPrint(printBuf);
	return 0;
}

int diag_loop (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[64];
	int ret;
	int num;
	if (argc != 2)
	{
		strcat(retBuff, "Invalid argument");
		return 0;
	}
	num = atoi(argv[1]);
	cmd_buff[0] = DIAG_LOOP;
	cmd_buff[1] = (unsigned char)num;
	ret = gDevHandle->SyncSendToSlave(cmd_buff, 2);
	if (ret < 0)
	{
		sprintf_s(printBuf, "Command failed : SLAVE_PWR_ON\n");
		CallBackPrint(printBuf);
	    return 0;
	}
	gDevHandle->GetRxBuffData(&rx_buf_index, rx_buf);
	sprintf_s(printBuf, "rx_buf_idx = %d, %02x %02x %02x\n", rx_buf_index, 
		rx_buf[MAGIC_LOC], rx_buf[PAGE_LOC], rx_buf[BLOCK_LOC]);
	CallBackPrint(printBuf);
	return 0;
}

int test_reader (int argc, char **argv, void *ptr)
{
	unsigned char cmd_buff[128];
	int ret;
	if (argc != 1)
	{
		strcat(retBuff, "Invalid argument");
		return 0;
	}
	cmd_buff[0] = 0xff;
	cmd_buff[1] = 0x00;
	cmd_buff[2] = 0x04;
	cmd_buff[3] = 0x1d;
	cmd_buff[4] = 0x0b;
	gDevHandle->StartUartOutput(cmd_buff, 5);
	return 0;
}

#if DELETED
#define STATE_DONE  99
char screen_buffer[4096];
int tot_len;
char newbaud[32];
int string_match(char *str)
{
    int idx, i, len;
    int match = 0;
    len = strlen(str);
    for (i = 0; i < 6; i++)
    {
        idx = tot_len - len - i;
        if (idx < 0)
            break;
        if (!memcmp(&screen_buffer[idx], str, len))
        {
            match = 1;
            break;
        }
    }
    return (match);
}

int ScreenStateMachine(SOCKET so, int *state)
{
    int change = 0;
    char snd_buffer[32];
    int idx, i;
    
    switch (*state)
    {
        case 0:
            if (string_match("Mode") == 1)
            {
                snd_buffer[0] = 0xd;
                snd_buffer[1] = 0x0;
                send(so, snd_buffer, 2, 0); 
                *state = 1;
                change = 1;
            }
            break;
        case 1:
            if (string_match("?") == 1)
            {
                snd_buffer[0] = '1';
                snd_buffer[1] = 0xd;
                snd_buffer[2] = 0x0;
                send(so, snd_buffer, 3, 0); 
                *state = 2;
                change = 1;
            }
            break;
        case 2:
            if (string_match("?") == 1)
            {
	            sprintf_s(snd_buffer, "%s\r", newbaud); 
                send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
                *state = 3;
                change = 1;
            }
            break;
        case 3:                         /* IF mode */
        case 4:                         /* FLOW */
        case 5:                         /* port No */
        case 6:                         /* Connect mode */
        case 7:                         /* Modem mode */
        case 8:                         /* ring mode */
        case 9:                         /* Inc source port */
        case 14:                         /* remote port */
        case 15:                         /* Disconn mode */
        case 16:                         /* flush mode */
        case 17:                         /* Disconn time */
        case 19:                         /* Send char 1 */
        case 20:                         /* Send char 2 */
            if (string_match("?") == 1)
            {
	            sprintf_s(snd_buffer, "\r"); 
                send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
                *state += 1;
                change = 1;
            }
            break;
        case 10:                         /* remote IP */
        case 11:                         /* remote IP */
        case 12:                         /* remote IP */
        case 13:                         /* remote IP */
            if (string_match("000") == 1)
            {
	            sprintf_s(snd_buffer, "\r"); 
                send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
                *state += 1;
                change = 1;
            }
            break;
        case 18:                         /* colon */
            if (string_match(":") == 1)
            {
	            sprintf_s(snd_buffer, "\r"); 
                send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
                *state += 1;
                change = 1;
            }
            break;
        case 21:                         /* about to save and exit */
            if (string_match("?") == 1)
            {
	            sprintf_s(snd_buffer, "9\r"); 
                send(so, snd_buffer, strlen(snd_buffer)+1, 0); 
                *state = STATE_DONE;
                change = 1;
            }
            break;
        default:
            break;
    }
    return (change);
}

int set_baudrate (int argc, char **argv, void *ptr)
{
	WSADATA 			wsaData;
    unsigned long addr;
    char cac_buffer[512];
    SOCKET sock;
	int retval, rcv_len;
	struct sockaddr_in 	serv;
    int result, state, change;
	struct timeval tv;
	fd_set readset;

	addr = inet_addr("192.168.111.101");
    strcpy(newbaud, argv[1]);

	memset(&serv,0,sizeof(serv));
	serv.sin_addr.s_addr = addr;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(9999);

#if 0
	if ((retval = WSAStartup(0x202,&wsaData)) != 0) {
		WSACleanup();
		strcat(retBuff, "WSAStartup Fail.");
		return 0;
	}
#endif
	sock = socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
	if (connect(sock,(struct sockaddr*)&serv,sizeof(serv))
		== SOCKET_ERROR) {
		WSACleanup();
		strcat(retBuff, "Connect to server:9999 fail.");
		return 0;
	}
    rcv_len = 0;
    screen_buffer[0] = 0;
    tot_len = 0;
    state = 0;

    while (1)
    {
	    FD_ZERO(&readset);
	    FD_SET(sock, &readset);
	    tv.tv_sec = 1;  /* 1 Secs Timeout */
	    tv.tv_usec = 0;
	    result = select(sock + 1, &readset, NULL, NULL, &tv);
	    if (result > 0) 
	    {
      		result = recv(sock, (char *)cac_buffer, 512, 0);
      		if (result == 0 || result == SOCKET_ERROR) 
      		{
         		/* This means the other side closed the socket */
         		// closesocket(socket_handle);
				return (-1);
      		}
      		else 
      		{
         		/* I leave this part to your own implementation */
				rcv_len = result;
	            sprintf_s(printBuf, "rx len = %d\n", rcv_len); 
	            CallBackPrint(printBuf);
                //memmove(printBuf, cac_buffer, rcv_len);
                //printBuf[rcv_len] = 0;
	            // CallBackPrint(printBuf);
                memmove(&screen_buffer[tot_len], cac_buffer, rcv_len);
                
                tot_len += rcv_len;
                screen_buffer[tot_len] = 0;
                change = ScreenStateMachine(sock, &state);
                if (state == STATE_DONE)
                {
		            strcat(retBuff, "Set baudrate done.");
                    break;
                }
                if (change)
                {
                    tot_len = 0;
                    screen_buffer[0] = 0;
                }
      		}
        }
        else
        {
	        sprintf_s(printBuf, "Final State = %d\n", state); 
	        CallBackPrint(printBuf);
		    strcat(retBuff, "End of Data.");
            break;
        }
    } /* while (1) */
    
    closesocket(sock);
}
#endif