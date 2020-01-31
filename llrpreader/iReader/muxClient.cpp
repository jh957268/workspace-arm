#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <process.h>
//#include <conio.h>

#include "muxclient.h"
#include "muxerror.h"
#include "gpio.h"

MuxClient::MuxClient(char *server_ip)
{
    unsigned long addr;

    socket_handle    = -1;
    socket_handle1    = -1;
	idx = tcp_len = 0;
	memset(send_buffer, 0x0, 9);
	addr = inet_addr(server_ip);
	memset(&server,0,sizeof(server));
	server.sin_addr.s_addr = addr;
	server.sin_family = AF_INET;
	server.sin_port = htons(10001);

	memset(&server1,0,sizeof(server1));
	server1.sin_addr.s_addr = addr;
	server1.sin_family = AF_INET;
	server1.sin_port = htons(30704);
}

MuxClient::~MuxClient()
{
    if (socket_handle != -1)
        closesocket(socket_handle);
    socket_handle = -1;
    if (socket_handle1 != -1)
        closesocket(socket_handle1);
    socket_handle1 = -1;
}

int  MuxClient::Connect(void)
{

#if 0
	if ((retval = WSAStartup(0x202,&wsaData)) != 0) {
		WSACleanup();
		return MUX_CONNECT_FAIL;
	}
#endif

#ifdef HAVE_READER
	socket_handle1 = socket(AF_INET,SOCK_DGRAM,0); /* Open a socket */
	if (socket_handle1 == INVALID_SOCKET ) {
        //WSACleanup();
		return MUX_CONNECT_FAIL;
	}
	if (connect(socket_handle1,(struct sockaddr*)&server1,sizeof(server1))
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

	socket_handle = socket(AF_INET,SOCK_STREAM,0); /* Open a socket */
	if (socket_handle == INVALID_SOCKET ) {
		//		WSAGetLastError());
        //WSACleanup();
		return MUX_CONNECT_FAIL;
	}
    
	if (connect(socket_handle,(struct sockaddr*)&server,sizeof(server))
		== SOCKET_ERROR) {
		//WSACleanup();
		return MUX_CONNECT_FAIL;
	}
	int bOptVal = TRUE;
	int bOptLen = sizeof(int);
	setsockopt(socket_handle, IPPROTO_TCP, TCP_NODELAY,  (void*)&bOptVal, bOptLen);

    SelectModule(READER_MODULE);
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
		recv(socket_handle, (char *)cache_buffer, 1024, 0);
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

void MuxClient::Disconnect(void)
{
	if (socket_handle != -1)
		closesocket(socket_handle);
	socket_handle = -1;
#ifdef HAVE_READER
	if (socket_handle1 != -1)
		closesocket(socket_handle1);
	socket_handle1 = -1;
#endif
}


int MuxClient::MuxActive(int flag, unsigned char *buff, int *ilen)
{
	int rcvlen, num_try;
    int result;
	fd_set readset;
	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = 0;
	while (1)
	{
		// empty the UDP queue
		FD_ZERO(&readset);
		FD_SET(socket_handle1, &readset);
		result = select(socket_handle1 + 1, &readset, NULL, NULL, &tv);
		if (result == 0)
			break;
		recv(socket_handle1, (char *)receive_buffer, 16, 0);
	}
	memset((char *)send_buffer,0,sizeof(send_buffer));
	tv.tv_sec = 3;
	send_buffer[0] = flag;
	for (num_try = 0; num_try < 3; num_try++)
	{
		if (send(socket_handle1, (char *)send_buffer, 9, 0) != 9)
		{
	    	// fprintf(stderr, "Error transmitting data.\n");
	    	// closesocket(socket_handle1);
	    	// WSACleanup();
	    	return (-1);
		}
		FD_ZERO(&readset);
		FD_SET(socket_handle1, &readset);
		result = select(socket_handle1 + 1, &readset, NULL, NULL, &tv);
		if (result == 0)
			continue;
		rcvlen = recv(socket_handle1, (char *)receive_buffer, 16, 0);
		if (rcvlen == SOCKET_ERROR) 
		{
			//error = WSAGetLastError();
			//if (error == WSAEWOULDBLOCK)
			//	continue;
			//else
			{
				// closesocket(socket_handle1);
				// WSACleanup();
				return -1;
			}
		}
		else
		{
			// confirm the response
			memmove(buff, receive_buffer, rcvlen);
			*ilen = rcvlen;
			return 0;
		}
	}
	return -1;
}

int MuxClient::SetIOActiveHigh(void)
{
	send_buffer[0] = 0x1A;
	send_buffer[1] = 0x07;
	send_buffer[5] = 0x00;
	if (SendCommand() == -1 || receive_buffer[1] != 0)
		return (-1);
	return 0;
}

int MuxClient::SelectModule(E_MODULE module)
{
	if (module ==  MUX_MODULE)
	{
		//GPIO_SET(0);
		GPIO::getInstance(0)->set_gpio_pin(0);
	}
	else
	{
		//GPIO_SET(1);
		GPIO::getInstance(0)->set_gpio_pin(1);
	}
	return 0;
}

int MuxClient::SendCommand(void)
{
	int num_try;
    int result;
	fd_set readset;
	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = 0;
	while (1)
	{
		// empty the UDP queue
		FD_ZERO(&readset);
		FD_SET(socket_handle1, &readset);
		result = select(socket_handle1 + 1, &readset, NULL, NULL, &tv);
		if (result == 0)
			break;
		recv(socket_handle1, (char *)receive_buffer, 16, 0);
	}
	tv.tv_sec = 3;
	for (num_try = 0; num_try < 3; num_try++)
	{
		if (send(socket_handle1, (char *)send_buffer, 9, 0) != 9)
		{
	    	// closesocket(socket_handle1);
	    	// WSACleanup();
	    	return (-1);
		}
		FD_ZERO(&readset);
		FD_SET(socket_handle1, &readset);
		result = select(socket_handle1 + 1, &readset, NULL, NULL, &tv);
		if (result == 0)
			continue;
		udp_len = recv(socket_handle1, (char *)receive_buffer, 16, 0);
		if (udp_len == SOCKET_ERROR) 
		{
			//error = WSAGetLastError();
			//if (error == WSAEWOULDBLOCK)
			//	continue;
			//else
			{
				// closesocket(socket_handle1);
				// WSACleanup();
				return -1;
			}
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
