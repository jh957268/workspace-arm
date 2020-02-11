#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "iReaderapi.h"

int main(void) 
{
	IReader *handle;
	int ret;
	const char *cgi_env = getenv("QUERY_STRING");

	// parse the cgi_env

	handle = IReaderApiInit();

	if (NULL == handle)
	{
		printf("Create IReader Fails");
		exit(-1);
	}
	ret = IReaderApiConnect(handle, "127.0.0.1");
	if (IREADER_SUCCESS != ret)
	{
		printf("Connect IReader Fails");
		exit(-1);
	}


    setvbuf(stdout, NULL, _IOLBF, 0);
    while ( 1)
    {
	time_t seconds;
	printf("Content-Type: text/event-stream\r\n");
	printf("Cache-Control: no-cache\r\n");
	time(&seconds);
	printf("data: current time is %ld\r\n", seconds);
	printf("data1: current time iiis %ld\r\n\r\n", seconds);
        sleep(2);
     }
}
