#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <process.h>
//#include <conio.h>

#include "define.h"
#include "muxclient.h"
#include "muxerror.h"

#define SERVER_CONTROL_PORT		6084
#define SERVER_DATA_PORT		10002

MuxClient::MuxClient(char *server_ip)
{
    unsigned long addr;

    socket_handle    = -1;
    socket_handle1    = -1;
	idx = tcp_len = idx_1 = tcp_len_1 = 0;
	memset(send_buffer, 0x0, 9);
	addr = inet_addr(server_ip);
	memset(&server,0,sizeof(server));
	server.sin_addr.s_addr = addr;
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_CONTROL_PORT);

	memset(&server1,0,sizeof(server1));
	server1.sin_addr.s_addr = addr;
	server1.sin_family = AF_INET;
	server1.sin_port = htons(SERVER_DATA_PORT);

	memset(&server2,0,sizeof(server2));
	server2.sin_addr.s_addr = addr;
	server2.sin_family = AF_INET;
	server2.sin_port = htons(30704);

}

MuxClient::MuxClient()
{
    socket_handle    = -1;
    socket_handle1   = -1;
    socket_handle2   = -1;
}

MuxClient::~MuxClient()
{
    if (socket_handle != -1)
        closesocket(socket_handle);
    socket_handle = -1;
    if (socket_handle1 != -1)
        closesocket(socket_handle1);
	 if (socket_handle2 != -1)
        closesocket(socket_handle2);
    socket_handle1 = -1;
    socket_handle2 = -1;
}

int  MuxClient::Connect(char *server_ip)
{
#if 0
	if ((retval = WSAStartup(0x202,&wsaData)) != 0) {
		WSACleanup();
		return MUX_CONNECT_FAIL;
	}
#endif

#ifdef HAVE_READER
	socket_handle2 = socket(AF_INET,SOCK_DGRAM,0); /* Open a socket */
	if (socket_handle2 == INVALID_SOCKET ) {
        //WSACleanup();
		return MUX_SOCKET_DGRM_FAIL;
	}
	if (connect(socket_handle2,(struct sockaddr*)&server2,sizeof(server2))
		== SOCKET_ERROR) {
		//WSACleanup();
		return MUX_CONNECT_FAIL;
	}
	if (SetIOActiveHigh() == -1)
	{
		//WSACleanup();
		return MUX_CONNECT_FAIL;
	}
#endif

	unsigned long addr;

	addr = inet_addr(server_ip);
	memset(&server,0,sizeof(server));
	server.sin_addr.s_addr = addr;
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_CONTROL_PORT);

	socket_handle = ::socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
	if (socket_handle == INVALID_SOCKET ) {
		//		WSAGetLastError());
        //WSACleanup();
		return MUX_SOCKET_STREAM_FAIL;
	}
    
	if (connect(socket_handle,(struct sockaddr*)&server,sizeof(server))
		== SOCKET_ERROR) {
		//WSACleanup();
		return MUX_CONNECT_STREAM_FAIL;
	}
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	setsockopt(socket_handle, IPPROTO_TCP, TCP_NODELAY,  (char*)&bOptVal, bOptLen);

#if 0
#ifdef USING_TCP
	socket_handle1 = ::socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
	if (socket_handle1 == INVALID_SOCKET ) {
		//		WSAGetLastError());
        //WSACleanup();
		return MUX_SOCKET_STREAM_FAIL;
	}
    
	if (::connect(socket_handle1,(struct sockaddr*)&server1,sizeof(server1))
		== SOCKET_ERROR) {
		//WSACleanup();
		return MUX_CONNECT_STREAM_FAIL;
	}

	::setsockopt(socket_handle1, IPPROTO_TCP, TCP_NODELAY,  (char*)&bOptVal, bOptLen);
#else
	socket_handle1 = ::socket(AF_INET,SOCK_DGRAM,0); /* Open a socket */
	if (socket_handle1 == INVALID_SOCKET ) {
        //WSACleanup();
		return MUX_SOCKET_DGRM_FAIL;
	}
	bind(socket_handle1,(struct sockaddr *)&server1,sizeof(server1));

#endif
#endif

    //SelectModule(MUX_MODULE);
	return MUX_SUCCESS;
}

int  MuxClient::sendChar(char ch)
{
    int retval;
	char Buffer[16];

	Buffer[0] = ch;

	retval = send(socket_handle,Buffer,1,0);
	if (retval == SOCKET_ERROR) 
	{
		// fprintf(stderr,"send() failed: error %d\n",WSAGetLastError());
		// closesocket(socket_handle);
		// WSACleanup();
		return -1;
	}
	if (retval == 0) {
		// printf("Server closed connection\n");
		// closesocket(socket_handle);
		// WSACleanup();
		return -1;
	}
	return 1;
}

int  MuxClient::sendArray(uint8_t *array, int len)
{
    int retval;
    int remain;

    
    flushRcvBuffer();    
    remain = len;
    while (remain)
    {        
	    retval = send(socket_handle,(const char *)array,remain,0);
	    if (retval == SOCKET_ERROR) 
	    {
		    // fprintf(stderr,"send() failed: error %d\n",WSAGetLastError());
		    // closesocket(socket_handle);
		    // WSACleanup();
		    return (SOCKET_ERROR);
	    }
	    if (retval == 0) {
		    // printf("Server closed connection\n");
		    // closesocket(socket_handle);
		    // WSACleanup();
		    return (SOCKET_ERROR);      /* big trouble */
	    }
        remain -= retval;
    }
	return len;
}

void MuxClient::flushRcvBuffer(void)
{
	int result;
	fd_set readset;
	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = 0;
	while (1)
	{
		// empty the intrnal TCP queue
		FD_ZERO(&readset);
		FD_SET(socket_handle, &readset);
		result = select(socket_handle + 1, &readset, NULL, NULL, &tv);
		if (result == 0 || result == SOCKET_ERROR)
			break;
		result = recv(socket_handle, (char *)cache_buffer, 1024, 0);
		if (result == 0 || result == SOCKET_ERROR)
			break;
	}
    idx = tcp_len = 0;
} 

int MuxClient::getChar(unsigned char *buff)
{
    int result;
	struct timeval tv;
	fd_set readset;

	if (idx < tcp_len)
	{
		buff[0] = cache_buffer[idx];
		idx++;
		if (idx >= tcp_len)
		{
			idx = 0;
			tcp_len = 0;
		}
		return 1;
	}
	FD_ZERO(&readset);
	FD_SET(socket_handle, &readset);
	tv.tv_sec = 1;  /* 1 Secs Timeout */
	tv.tv_usec = 0;
	// tv.tv_sec = 0;  /* 0 Secs Timeout */
	// tv.tv_usec = (300 * 1000);
	result = select(socket_handle + 1, &readset, NULL, NULL, &tv);
    if (result == SOCKET_ERROR)
        return (SOCKET_ERROR);
	if (result > 0) 
	{
		if (FD_ISSET(socket_handle, &readset)) 
		{
      		/* The socket_fd has data available to be read */
      		result = recv(socket_handle, (char *)cache_buffer, 1024, 0);
      		if ((result == SOCKET_ERROR) || (result < 0))
      		{
                return(SOCKET_ERROR);
      		}
      		if (result == 0)
      		{
         		/* This means the other side closed the socket */
				return (0);     /* Should not happen, if so, should return error */
      		}
      		else 
      		{
         		/* I leave this part to your own implementation */
				tcp_len = result;
				buff[0] = cache_buffer[0];
				idx = 1;
				if (idx >= tcp_len)
				{
					idx = 0;
					tcp_len = 0;
				}
				return 1;
      		}
   		}
		else
		{
			/* should not happen */
			return 0;
		}
	}
	/* select timeout, no data available */
	return 0;

#if 0
	retval = recv(socket_handle, Buffer, 1, 0 );
	if (retval == SOCKET_ERROR) {
		// When timeout receiving data, it will fall here (why)??
		// don't close the socket, as the iSwitch is loader mode, and not response to command 
		// that he does not understand. 
		// fprintf(stderr,"recv() failed: error %d\n",WSAGetLastError());
		// closesocket(socket_handle);
		// WSACleanup();
		return 0;
	}
	if (retval == 0) {
		// don't close the socket, as the iSwitch is loader mode, and not response to command 
		// that he does not understand. 
		// printf("Server closed connection\n");
		closesocket(socket_handle);
		WSACleanup();
		return 0;
	}
	buff[0] = Buffer[0];
	return 1;
#endif
}

#ifdef USING_TCP
int MuxClient::getChars_1(unsigned char *buff, int len)
{
    int result;
	struct timeval tv;
	fd_set readset;

	while (1)
	{
		if ((tcp_len_1 - idx_1) >= len)
		{
			for (int i = 0; i < len; i++)
			{
				buff[i] = cache_buffer_1[idx_1];
				idx_1++;
			}
		
			if (idx_1 >= tcp_len_1)
			{
				idx_1 = 0;
				tcp_len_1 = 0;
			}
			return (len);
		}
	
		FD_ZERO(&readset);
		FD_SET(socket_handle1, &readset);
		tv.tv_sec = 1;  /* 1 Secs Timeout */
		tv.tv_usec = 0;
		// tv.tv_sec = 0;  /* 0 Secs Timeout */
		// tv.tv_usec = (300 * 1000);
		result = select(socket_handle1 + 1, &readset, NULL, NULL, &tv);
    	if (result == SOCKET_ERROR)
        	return (SOCKET_ERROR);
		if (result > 0) 
		{
			if (FD_ISSET(socket_handle1, &readset)) 
			{
      			/* The socket_fd has data available to be read */
      			result = ::recv(socket_handle1, (char *)&cache_buffer_1[tcp_len_1], 1024 - tcp_len_1, 0);
      			if (result == SOCKET_ERROR)
                	return(SOCKET_ERROR);
      			if (result == 0)
      			{
         			/* This means the other side closed the socket */
					return (0);     /* Should not happen, if so, should return error */
      			}
      			else 
      			{
         			/* I leave this part to your own implementation */
					tcp_len_1 += result;
				}
      		}
			// go back to while (1)
   		}
		else
		{
			/* should not happen */
			return 0;
		}
	}
	/* select timeout, no data available */
	return 0;
}
#else
int MuxClient::getChars_1(unsigned char *buff, int len)
{
	int solen;
	int packet_len;
	struct sockaddr_in cliaddr;
	struct timeval tv;
	fd_set readset;
	int result;

	while (1)
	{
		if ((tcp_len_1 - idx_1) >= len)
		{
			for (int i = 0; i < len; i++)
			{
				buff[i] = cache_buffer_1[idx_1];
				idx_1++;
			}
		
			if (idx_1 >= tcp_len_1)
			{
				idx_1 = 0;
				tcp_len_1 = 0;
			}
			return (len);
		}

		FD_ZERO(&readset);
		FD_SET(socket_handle1, &readset);
		tv.tv_sec = 2;  /* 2 Secs Timeout */
		tv.tv_usec = 0;
		result = select(socket_handle1 + 1, &readset, NULL, NULL, &tv);
   		if (result == SOCKET_ERROR)
        	return (SOCKET_ERROR);

		if (result == 0)
		{
			return 0;
		}

		solen = sizeof(cliaddr);
		tcp_len_1 = recvfrom(socket_handle1 ,(char *)&cache_buffer_1[0], 512, 0, (struct sockaddr *)&cliaddr, &solen);
		packet_len = ((cache_buffer_1[2] << 8) | cache_buffer_1[3]) + 6;	// 2 bytes HDR, 2 bytes len, 2 bytes 1's complement len
		// XXX need to check the tcp_len_1 to make sure the packet len is correct
		if (tcp_len_1 != packet_len)
		{
			tcp_len_1 = 0;
		}
		idx_1 = 0;
	}

	/* select timeout, no data available */
	return 0;
}
#endif


void MuxClient::Disconnect(void)
{
	if (socket_handle != -1)
		closesocket(socket_handle);
	socket_handle = -1;

	if (socket_handle1 != -1)
		closesocket(socket_handle1);
	socket_handle1 = -1;

	if (socket_handle2 != -1)
		closesocket(socket_handle2);
	socket_handle2 = -1;

}

int MuxClient::SendCommand(void)
{
	int num_try, error;
    int result;
	fd_set readset;
	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = 0;
	while (1)
	{
		// empty the UDP queue
		FD_ZERO(&readset);
		FD_SET(socket_handle2, &readset);
		result = select(socket_handle2 + 1, &readset, NULL, NULL, &tv);
		if (SOCKET_ERROR == result)
			return (-1);
		if (result == 0)
			break;
		result = recv(socket_handle2, (char *)receive_buffer, 16, 0);
				if (SOCKET_ERROR == result)
			return (-1);
		if (result == 0)
			break;
	}
	tv.tv_sec = 3;
	for (num_try = 0; num_try < 3; num_try++)
	{
		if (send(socket_handle2, (char *)send_buffer, 9, 0) != 9)
		{
	    	// closesocket(socket_handle1);
	    	// WSACleanup();
	    	return (-1);
		}
		FD_ZERO(&readset);
		FD_SET(socket_handle2, &readset);
		result = select(socket_handle2 + 1, &readset, NULL, NULL, &tv);
		if (SOCKET_ERROR == result)
			return (-1);
		if (result == 0)
			continue;
		udp_len = recv(socket_handle2, (char *)receive_buffer, 16, 0);
		if (udp_len == SOCKET_ERROR) 
		{
#if 0
			error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK)
				continue;
			else
			{
				// closesocket(socket_handle1);
				// WSACleanup();
				return -1;
			}
#endif
			return (-1);
		}
		else
		{
			// confirm the response
			if ((udp_len == 5) && (receive_buffer[0] == send_buffer[0]))
				return 0;
		}
	}
	return -1;
}
