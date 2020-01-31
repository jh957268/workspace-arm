#define EXECUTOR_SRC
#include "iAgent_executor.h"
#include "debug_print.h"

// static uint8_t ttagrbuf[512];

uint8_t	read_antList[MAXANT];
int		read_antCount;

extern uint8_t	m_antpower[MAXANT];
extern uint8_t	m_antlist[];
extern int		m_antcount;

void executor_task(void);

//=============================================================================
// Constructor

IAgent_Executor::IAgent_Executor():
    OwTask( HIGH, 2048, "IAgent_Executor")
{
	DBG_PRINT( DEBUG_INFO, "IAgent_Executor Created" NL);

	for (int i = 0; i < MAX_TX_SOCKET; i++)
	{
		fd[i] = -1;
	}
	executor_start_flag = 0;

}

void
IAgent_Executor::main( OwTask * )
{
	uint32_t evt_flag;
	Int32 status;
	int tagCount;

	handle = IReader::getInstance();
	while (1)
	{
		// OSEvtPend( &executor_evt_flag, 0x01, &evt_flag, EVENT_ANY, OS_WAIT);
		if (!executor_start_flag)
		{
			OwTask::sleep(2);
			continue;
		}

		while (	executor_start_flag != 0 )
		{
#if 0			
			for (int i = 0; i < m_antcount; i++)
			{
				int antID = m_antlist[i];

				status = IReaderApiReadTagsMetaDataRSSI(handle, antID, m_antpower[antID], &tagCount, (struct taginfo_rssi *)&ttagrbuf[9]);
				if (status != IREADER_SUCCESS)
				{
					Console_Printf("Read Tags Fails"NL);
					OSSleep(200);
					continue;
				}
				if (tagCount == 0)
				{
					continue;
				}
				
				// Now report the tags through network interface.
				if (callbackFunctionPtr != 0)
				{
					callbackFunctionPtr(ttagrbuf, tagCount, antID );
				}
				if ((executor_start_flag & 0xf0000) != 0)
				{
					// This is from cli command, execute one loop
					executor_start_flag = 0;
					break;
				}				
			}
			OSSleep(2);
#endif
			for (int j = 0; j < MAX_TX_SOCKET; j++)
			{
				int iResult;

				if (fd[j] != -1)
				{
					iResult = ::send( fd[j], (const char *)ttagrbuf, strlen((char *)ttagrbuf), 0 );
				}

				if ( iResult == SOCKET_ERROR )
				{
					DBG_PRINT(DEBUG_INFO, "IAgent_executor sendMessage failed with error: %d" NL, iResult );

					fd[j] = -1;
				}
			}
		}
	}
}

void
IAgent_Executor::start_executor(int fd)
{

}

void
IAgent_Executor::stop_executor(int fd)
{
	
}

#if 0
void executorSetStartFlag(int start_flg)
{
	executor_start_flag = start_flg;
}

void printAntList(void)
{
	Console_Printf("Connected antennas List:"NL);
	Console_Printf("{ ");
	for (int i = 0; i < m_antcount; i++)
	{
		Console_Printf("%d ", m_antlist[i]);
	}
	Console_Printf(" }"NL);
}

void SetAntBitMap(int idx)
{
	int byte_pos, bit_pos;

	if ( 256 < idx )
		return;

	byte_pos = idx/8;
	bit_pos = idx%8;

	antBitMap[byte_pos] |= (1 << bit_pos);
}

void ClearAntBitMap(int idx)
{
	int byte_pos, bit_pos;

	if ( 256 < idx )
		return;

	byte_pos = idx/8;
	bit_pos = idx%8;

	antBitMap[byte_pos] &= ~(1 << bit_pos);
}

int GetAntBitMap(int idx)
{

	int byte_pos, bit_pos;

	if ( 256 < idx )
		return;

	byte_pos = idx/8;
	bit_pos = idx%8;

	if (((antBitMap[byte_pos] & (1 << bit_pos)) == 0 )
		return 0;
	return 1;

}
#endif
