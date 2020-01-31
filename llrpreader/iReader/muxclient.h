#ifndef __MUXCLIENT_H__
#define __MUXLIENT_H__

#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <winsock2.h>
//#include <afxwin.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "muxdev.h"
#include "muxapi.h"
#include "SysTypes.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define HAVE_READER

class MuxClient : public Muxdev
{
    // -------------------------------------------------------- //
	protected:
		SOCKET            	socket_handle;                 
		SOCKET            	socket_handle1;                 
		WSADATA 			wsaData;
		struct sockaddr_in 	server;
		struct sockaddr_in 	server1;
		unsigned char		send_buffer[16];
		unsigned char		receive_buffer[16];
		unsigned char		cache_buffer[1024];
		int					idx, tcp_len, udp_len;

	// ++++++++++++++++++++++++++++++++++++++++++++++
	// .................. EXTERNAL VIEW .............
	// ++++++++++++++++++++++++++++++++++++++++++++++

	public:
					  MuxClient(char *server_ip);
		virtual		  ~MuxClient();
		int           Connect          (void);
		int           sendChar         (char ch);
    	int     	  sendArray        (uint8_t *buffer, int len);
		int			  getChar          (unsigned char *ch);
		void          Disconnect       (void);
		int			  MuxActive		   (int flag, unsigned char *buff, int *len);
		int			  SetIOActiveHigh  (void);
		int			  SelectModule     (E_MODULE module);
		int			  SendCommand	   (void);
		void          flushRcvBuffer   (void);

		void		WSACleanup(void){}
		int 		WSAStartup(int a,int *b){return 0;}
		void 		closesocket(int fd) {close(fd);}
};

#endif

