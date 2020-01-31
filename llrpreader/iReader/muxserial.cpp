#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */
#include <errno.h>   /* ERROR Number Definitions           */

#include "muxserial.h"

/* -------------------------------------------------------------------- */
/* -------------------------    Muxserial   ----------------------------- */
/* -------------------------------------------------------------------- */

#define INVALID_HANDLE_VALUE	-1

Muxserial::Muxserial(char *comPort)
{
    parityMode       = spNONE;
    strncpy(port, comPort, 12);
    rate             = 115200;
    serial_handle    = INVALID_HANDLE_VALUE;

	mpMutex = new OwMutex();

    Connect();
}

/* -------------------------------------------------------------------- */
/* --------------------------    ~Muxserial     ------------------------- */
/* -------------------------------------------------------------------- */
Muxserial::~Muxserial()
{
    if (serial_handle!=INVALID_HANDLE_VALUE)
        close(serial_handle);
    serial_handle = INVALID_HANDLE_VALUE;
}
/* -------------------------------------------------------------------- */
/* --------------------------    disconnect   ------------------------- */
/* -------------------------------------------------------------------- */
void Muxserial::Disconnect(void)
{
    if (serial_handle!=INVALID_HANDLE_VALUE)
        close(serial_handle);
    serial_handle = INVALID_HANDLE_VALUE;
}
/* -------------------------------------------------------------------- */
/* --------------------------    connect      ------------------------- */
/* -------------------------------------------------------------------- */
int  Muxserial::Connect(void)
{
    int erreur = 0;

    serial_handle = open(port,O_RDWR | O_NOCTTY ); /* O_NDELAY | O_NONBLOCK); ttyUSB0 is the FT232 based USB2SERIAL Converter   */
	   						/* O_RDWR Read/Write access to serial port           */
							/* O_NOCTTY - No terminal will control the process   */
							/* O_NDELAY -Non Blocking Mode,Does not care about-  */
							/* -the status of DCD line,Open() returns immediatly */

	if(serial_handle == -1)						/* Error Checking */
	{
    	   printf("Error! in Opening %s \n ", port);
    	   return -1;
	}
	else
	{
    	   printf("%s Opened Successfully\n ", port);
	}

	tcflush(serial_handle, -TCIOFLUSH);
	/*---------- Setting the Attributes of the serial port using termios structure --------- */

#if 1
	struct termios SerialPortSettings;	/* Create the structure                          */

	if (tcgetattr(serial_handle, &SerialPortSettings) < 0)	/* Get the current attributes of the Serial port */
	{
	    perror("Can't get port settings");
	    return -1;
	}
	//cfsetispeed(&SerialPortSettings,B115200); /* Set Read  Speed as 9600                       */
	//cfsetospeed(&SerialPortSettings,B115200); /* Set Write Speed as 9600                       */

	SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
	SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
	SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
	SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */

	SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
	SerialPortSettings.c_cflag |=  B115200 | CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */


	SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
	SerialPortSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode, raw mode                            */

	SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/

	// block for up till 128 characters
	SerialPortSettings.c_cc[VMIN] = 0;

	// 0.1 seconds read timeout
	SerialPortSettings.c_cc[VTIME] = 1;

	if((tcsetattr(serial_handle,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
	{
		printf("ERROR ! in Setting attributes for %s\n", port);
	    perror("Can't set port settings");
	    return -1;
	}
	else
	{
        printf("%s: BaudRate = 115200  StopBits = 1  Parity = none\n", port);
	}

#endif

    return(erreur);
}


/* -------------------------------------------------------------------- */
/* --------------------------    sendChar     ------------------------- */
/* -------------------------------------------------------------------- */
int Muxserial::sendChar(char data)
{
    sendArray((uint8_t *)&data, 1);
	return 1;
}

/* -------------------------------------------------------------------- */
/* --------------------------    sendArray    ------------------------- */
/* -------------------------------------------------------------------- */
int Muxserial::sendArray(uint8_t *buffer, int len)
{
    int bytes_written = 0;

    if (serial_handle!=INVALID_HANDLE_VALUE)
    {
    	bytes_written = write(serial_handle, buffer, len);
    }
    if (bytes_written != len)
    {
    	printf("sendArray: req: %d, actual: %d\n", len,bytes_written );
    }
    return bytes_written;
}

/* -------------------------------------------------------------------- */
/* --------------------------    getChar      ------------------------- */
/* -------------------------------------------------------------------- */
int Muxserial::getChar(unsigned char *ch)
{
    char c;
	int nbr;

	c = 0;
    nbr = getArray(&c, 1);
	*ch = (unsigned char)c;
    return(nbr);
}

/* -------------------------------------------------------------------- */
/* --------------------------    getArray     ------------------------- */
/* -------------------------------------------------------------------- */
int  Muxserial::getArray(char *buffer, int len)
{
    int read_nbr;

    read_nbr = 0;
    if (serial_handle!=INVALID_HANDLE_VALUE)
    {
    	read_nbr = read(serial_handle, buffer, len);
#if 0	// remove this to save time, the perror will return Resource is unvailable	
    	if (-1 == read_nbr)
    	{
    		perror("sertila read\n");
    		if (errno == EAGAIN)
    		{
    			read_nbr = 0;
    		}
    	}
#endif		
    }
	// printf("Number = %d\n", read_nbr);
    return((int) read_nbr);
}
/* -------------------------------------------------------------------- */
/* --------------------------    getNbrOfBytes ------------------------ */
/* -------------------------------------------------------------------- */
int Muxserial::getNbrOfBytes(void)
{
    //struct _COMSTAT status;
    int             n;
    //unsigned long   etat;

    n = 0;

    //if (serial_handle!=INVALID_HANDLE_VALUE)
    //{
    //    ClearCommError(serial_handle, &etat, &status);
    //    n = status.cbInQue;
    //}


    return(n);
}

void Muxserial::clearRcv(void)
{
	unsigned char dum_ch;
	int n;

    if (serial_handle == INVALID_HANDLE_VALUE)
    {
    	return;
    }
	fcntl(serial_handle, F_SETFL, FNDELAY);

	while (1)
	{
		n = getChar(&dum_ch);
		if ((0 == n) || (-1 == n))   // getChar if no char is available since it is unblock. errno will be EAGAIN
			                         // or EWOULDBLOCK, and perror will be resource is temporary unavailabe
		{
			break;
		}
	}
	fcntl(serial_handle, F_SETFL, 0);
}


