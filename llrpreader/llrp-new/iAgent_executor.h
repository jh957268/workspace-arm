#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include "OwTask.h"
#include "iReaderapi.h"

#ifndef TAGIDLEN
#define TAGIDLEN	12
#endif

#define	MAX_TAGS_REPORT	1024

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

#define MAX_TX_SOCKET   4

class IAgent_Executor:
    public OwTask
{
public:
 
	IAgent_Executor();
    ~IAgent_Executor(){}

    /// The main program for the ROSpecExecutor.  Inherited from OwTask.
    ///
    void	main(OwTask*);
    static  IAgent_Executor*	getInstance(void);
    void    start_executor(int fd);
    void    stop_executor(int fd);

protected:
    static IAgent_Executor* spInstance; ///< Points to the instance

    int     fd[MAX_TX_SOCKET];
    IReader *handle;                    // point to Ireader object
    int     executor_start_flag;
    uint8_t ttagrbuf[512];
};

void printAntList(void);

#endif
