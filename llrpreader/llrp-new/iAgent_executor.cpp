#define EXECUTOR_SRC
#include "iAgent_executor.h"
#include "debug_print.h"
#include "iAgent.h"

// static uint8_t ttagrbuf[512];

IAgent_Executor* IAgent_Executor::spInstance = 0;
OwSemaphore*	 IAgent_Executor::m_hSem = 0;

//=============================================================================
// Constructor

IAgent_Executor::IAgent_Executor():
    OwTask( HIGH, 2048, "IAgent_Executor")
{
	DBG_PRINT( DEBUG_INFO, "IAgent_Executor Created" NL);

	for (int i = 0; i < MAX_TX_SOCKET; i++)
	{
		clientFd[i] = -1;
	}
	executor_start_flag = 0;
	m_antcount = 0;
	m_hSem = new OwSemaphore(1);

}

void
IAgent_Executor::main( OwTask * )
{
	Int32 status;
	int tagCount;
	int retval;

	handle = IReader::getInstance();
	while (1)
	{
		// OSEvtPend( &executor_evt_flag, 0x01, &evt_flag, EVENT_ANY, OS_WAIT);
		retval = semaphoreTake(PI_FOREVER);
		if (OK != retval)
		{
			OwTask::sleep(2);
			continue;
		}

		m_antcount = 1;  // test only
		m_antlist[0] = 1;
		m_antpower[1] = 2500;
		m_antpower[0] = 2500;
		while (	executor_start_flag != 0 )
		{

			for (int i = 0; i < m_antcount; i++)
			{
				int antID = m_antlist[i];

				status = IReaderApiReadTagsMetaDataRSSI(handle, antID, m_antpower[antID], &tagCount, (struct taginfo_rssi *)&ttagrbuf[9]);
				if (status != IREADER_SUCCESS)
				{
					DBG_PRINT(DEBUG_INFO,"Read Tags Fails" NL);
					OwTask::sleep(2);
					continue;
				}
				if (tagCount == 0)
				{
					continue;
				}
#if 0
				// Now report the tags through network interface.
				if (callbackFunctionPtr != 0)
				{
					callbackFunctionPtr(ttagrbuf, tagCount, antID );
				}
#endif
				if ((executor_start_flag & 0xf0000) != 0)
				{
					// This is from cli command, execute one loop
					executor_start_flag = 0;
					break;
				}				

				for (int j = 0; j < MAX_TX_SOCKET; j++)
				{
					int iResult;

					if (clientFd[j] == -1)
					{
						continue;
					}

					iResult = IAgent::iAgent_CallBack(clientFd[j], ttagrbuf, tagCount, antID);

					if ( iResult != tagCount )
					{
						DBG_PRINT(DEBUG_INFO, "IAgent_executor sendMessage failed with error: %d" NL, iResult );

						clientFd[j] = -1;
					}
				}
				OwTask::sleep(10);
			}
		}
	}
}

void
IAgent_Executor::start_executor(int fd)
{
	executor_start_flag = 1;

	for (int i = 0; i < MAX_TX_SOCKET; i++)
	{
		if (clientFd[i] == -1)
		{
			clientFd[i] = fd;
		}
	}
	semaphoreGive();
}

void
IAgent_Executor::stop_executor(int fd)
{
	executor_start_flag = 0;
	
	for (int i = 0; i < MAX_TX_SOCKET; i++)
	{
		if (clientFd[i] == fd)
		{
			clientFd[i] = -1;
		}
	}
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

IAgent_Executor
*IAgent_Executor::getInstance
(
	void
)
{
	if ( 0 == spInstance )
	{
		spInstance = new IAgent_Executor();
	}

	return( spInstance );

} // LLRP_MntServer:getInstance()

int
IAgent_Executor::semaphoreTake(int timeout)
{
	int error = IREADER_SUCCESS;

	m_hSem->take( timeout );

	return (error);
}

int
IAgent_Executor::semaphoreGive()
{
	int error = IREADER_SUCCESS;

	error = m_hSem->give();

	return (error);
}

