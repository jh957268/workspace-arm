#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include <vector>
#include "OwTask.h"
#include "OwSemaphore.h"
#include "OwMutex.h"
#include "iReaderapi.h"

#ifndef TAGIDLEN
#define TAGIDLEN	12
#endif

#define	MAX_TAGS_REPORT	1024

#if 0
struct llrp_taginfo
{
	UINT8		tagid[TAGIDLEN];
    INT32		count;
    INT32		antid;
    INT16		RSSI;
	UINT16		pc_bits;
	__time64_t	firstSeenTime;
	__time64_t	lastSeenTime;
};
#endif

#define MAX_TX_SOCKET   4

class IAgent_Executor:
    public OwTask
{
public:
 
	IAgent_Executor();
    ~IAgent_Executor(){}
	
	struct callbackHandler
	{
		virtual void	TagEventCallback(uint8_t *tag_data, int tag_cnt, int ant_id);
	};

    /// The main program for the ROSpecExecutor.  Inherited from OwTask.
    ///
    void	main(OwTask*);
    static  IAgent_Executor*	getInstance(void);
    static	int				semaphoreTake(int timeout);
    static	int				semaphoreGive();
    void    start_executor(int fd);
    void    stop_executor(int fd);
	void 	SetCallbackHandler( callbackHandler*  handler );
	void 	RemoveCallbackHandler( callbackHandler*  handler );
	void	Do_Callback(int tag_cnt, int ant_id);
	int     TakeMutex(void);
	int     GiveMutex(void);
	int 	IAgent_Executor::HasClients(void);

protected:
    static IAgent_Executor* spInstance; ///< Points to the instance
    static OwSemaphore 		*m_hSem;
    int     clientFd[MAX_TX_SOCKET];
    IReader *handle;                    // point to Ireader object
	std::vector<callbackHandler*>	moRegistry;	// Registers event handler objects
    int     executor_start_flag;
    uint8_t ttagrbuf[512];
    //int		m_antcount;
    //uint16_t m_antlist[MAX_ANT_CNT];
    //uint16_t m_antpower[MAX_ANT_CNT];
	OwMutex			*m_hMutex;
};

void printAntList(void);

#endif
