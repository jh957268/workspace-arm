#ifndef __SAGENT_H__
#define __SAGENT_H__

//============================================================
// for Linux BSD Socket library
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
//============================================================

#include "OwTask.h"
#include "iMsgUtils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <poll.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "iAgent_executor.h"
#include "CCLI.h"

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

class SAgent :
    public OwTask,
	public IAgent_Executor::callbackHandler	
{
public:
    /// Constructor
    ///
    		SAgent();
			~SAgent(){}

    /// Main program of the agent for a maintenance terminal.
    /// Inherited from OwTask
    ///
    void	main(OwTask*);

	void	setRunning()
	{
		idle = false;
	}
	void setConnection(SOCKET fd);
	bool	isIdle() { return idle; }
	unsigned int getObjectID(void )
	{
		return id;
	}
	void  disconnect();
	void  scgi_process(const int fd);
	char * scgi_getenv(char *r, const unsigned long rlen, const char * const name);
	void  TagEventCallback(const char *tag_data);
	
private:

    SOCKET			clientSocket;	///< Socket of TCP connection to the client
    volatile bool	isSocketClosed;
    bool	        idle;			///< True if this agent is idle
    unsigned int	id;				///< ID of the agent
    static unsigned int  nextId;		///< Next agent ID available
    int             parm[4];
    char            tagbuff[1024];
	char 			buf[1024];
	int 			finished;
	CCLI			cliObj;
	std::string		cgi_string;

};

#endif /* __SAGENT_H__ */
