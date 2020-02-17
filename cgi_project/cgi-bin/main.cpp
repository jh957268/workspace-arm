#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "iReaderapi.h"
char antmap[300];

int main(void) 
{
	IReader *handle;
	int ret;
	int region;
	const char *cgi_env = getenv("QUERY_STRING");

	// parse the cgi_env

#if 0
	if (cgi_env)
	{
		if (!strcmp(cgi_env, "antmap=1"))
		{
			memset(antmap, 0, 300);
			for (int i = 0; i < 256; i++)
			{
				if (i & 1)
				{
					strcat(antmap, "1");
				}
				else
				{
					strcat(antmap, "0");
				}
			}
			printf("%s", antmap);
			return 0;
		}
	}
#endif
	handle = IReaderApiInit();

	if (NULL == handle)
	{
		printf("Create IReader Fails");
		exit(-1);
	}
	ret = IReaderApiConnect(handle, (char *)"127.0.0.1");
	if (IREADER_SUCCESS != ret)
	{
		printf("Connect IReader Fails");
		IReaderApiClose(handle);
		exit(-1);
	}
	if (!strcmp(cgi_env, "antmap=1"))
	{
		memset(antmap, 0, 270);
		ret = IReaderApiGetAntMap(handle, antmap);
		printf("%s", antmap);
		return 0;
	}
	if (!strcmp(cgi_env, "region=1"))
	{
		ret = IReaderApiGetRegion(handle, &region);
	}
	else
	{
		ret = IReaderApiGetSearchTimeout(handle, &region);
	}
	if (IREADER_SUCCESS != ret)
	{
		printf("0");
	}
	else
	{
		printf("%d", region);
	}
	IReaderApiClose(handle);

	return 0;
#if 0
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
#endif
}
