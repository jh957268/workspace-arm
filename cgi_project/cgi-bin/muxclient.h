#ifndef __MUXCLIENT_H__
#define __MUXLIENT_H__

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <winsock2.h>
// #include <afxwin.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

typedef int SOCKET;
typedef int WSADATA;

#define SOCKET_ERROR		-1
#define INVALID_SOCKET		-1
#endif

// #define HAVE_READER
#define USING_TCP

class MuxClient
{
    // -------------------------------------------------------- //
	protected:
		SOCKET            	socket_handle;                 
		SOCKET            	socket_handle1;
		SOCKET            	socket_handle2; 
		WSADATA 			wsaData;
		struct sockaddr_in 	server;
		struct sockaddr_in 	server1;
		struct sockaddr_in 	server2;
		unsigned char		send_buffer[16];
		unsigned char		receive_buffer[16];
		unsigned char		cache_buffer[1024];
		int					idx, tcp_len, udp_len;
		unsigned char		cache_buffer_1[1024];
		int					idx_1, tcp_len_1;

	// ++++++++++++++++++++++++++++++++++++++++++++++
	// .................. EXTERNAL VIEW .............
	// ++++++++++++++++++++++++++++++++++++++++++++++

	public:
					  MuxClient(char *server_ip);
					 ~MuxClient();
		int           Connect          (void);
		int           sendChar         (char ch);
    	int     	  sendArray        (uint8_t *buffer, int len);
		int			  getChar          (unsigned char *ch);
		int			  getChars_1       (unsigned char *buff, int len);
		void          Disconnect       (void);
		int			  SendCommand	   (void);
		void          flushRcvBuffer   (void);
};

#endif

