#ifndef __IAGENT_H__
#define __IAGENT_H__

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

#define DATABASE_MAGIC		0xFFEE
#define DATABASE_USER_MAGIC	0xFFDD

class IAgent :
    public OwTask   
{
public:
    /// Constructor
    ///
    		IAgent();

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
	
	static int iAgent_CallBack(int fd, uint8_t *tReadBuf, int itReadCnt, int antID);
	static int Sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName);

private:

   void	socketDeliver();

    SOCKET			clientSocket;	///< Socket of TCP connection to the client
    volatile bool	isSocketClosed;
    bool	        idle;			///< True if this agent is idle
    unsigned int	id;				///< ID of the agent
    static unsigned int  nextId;		///< Next agent ID available
    iMsgObj  		t_rxMsg;
    int             parm[4];
    char            tagbuff[1024];

    int  iAgent_receiveMsgObj(iMsgObj *hMsg);
    int  iAgent_sendMsgObj(iMsgObj *hMsg);
    int  iAgent_ProcessMsgObj(iMsgObj *hMsg);

    void iAgent_SetAntPort(iMsgObj *hMsg);
    void iAgent_GetAntBitMap(iMsgObj *hMsg);
    void iAgent_ReadTags(iMsgObj *hMsg);
    void iAgent_ReadTagsRSSI(iMsgObj *hMsg);
    void iAgent_SetTxPower(iMsgObj *hMsg);
    void iAgent_StartExecutor(iMsgObj *hMsg);
    void iAgent_GetRegion(iMsgObj *hMsg);
    void iAgent_GetSearchTimeout(iMsgObj *hMsg);
	void iAgent_SetSearchTimeout(iMsgObj *hMsg);
    void iAgent_Sqlite_Select(iMsgObj *hMsg);
    void iAgent_Sqlite_Insert(iMsgObj *hMsg);
    void iAgent_Rescan_Salve(iMsgObj *hMsg);
    void iAgent_ReadEquipTemp(iMsgObj *hMsg);
    void iAgent_SetEquipTempProtect(iMsgObj *hMsg);
    int sendMessage(const char *msg);
    int sendMessage(uint8_t *buff, int len);
	int sendResponse(uint8_t resp_code, int msg_len, uint8_t *msg);
    void disconnect(void);

    int get_bytes(uint8_t *buff, int req_len);

};

#endif /* __IAGENT_H__ */
