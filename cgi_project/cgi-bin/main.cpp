#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "iReaderapi.h"
char antmap[300];

char ttagrbuf[512];

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

		for (int i = 0; i < 256; i++)
		{
			antmap[i] += 0x30;
		}
#if 0
		for (int i = 0; i < 256; i++)
		{
			if ((antmap[i] != '0') && (antmap[i] != '1'))
			{
				antmap[i] = '0';
			}
#if 0
			if (i & 1)
			{
				strcat(antmap, "1");
			}
			else
			{
				strcat(antmap, "0");
			}
#endif
		}
#endif
		antmap[256] = 0;
		printf("%s", antmap);
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strcmp(cgi_env, "readtag=1"))
	{
		int ant_id, ttagCount;
		struct taginfo_rssi *pTaginfo_rssi;
		char pcbits[8], epcdata[32], rssidata[20];
		short rssi;

		ret = IReaderApiStartExecutor(handle, 1);
		if (IREADER_SUCCESS != ret)
		{
			IReaderApiClose(handle);
			return 0;
		}

		// Ireader read asyn tags
		while (1)
		{
			ret = IReaderApiGetTagsMetaDataRSSI(handle, &ant_id, &ttagCount, (struct taginfo_rssi *)ttagrbuf);
			if (IREADER_SUCCESS != ret)
			{
				IReaderApiClose(handle);
				return 0;
			}
			pTaginfo_rssi = (struct taginfo_rssi *)ttagrbuf;
			time_t now = time(NULL);
			for (int i = 0; i < ttagCount; i++)
			{
			   	printf("Content-Type: text/event-stream\r\n");
			    printf("Cache-Control: no-cache\n\n");
			    sprintf(pcbits,"%02x %02x", u8(pTaginfo_rssi->tagid[0]), u8(pTaginfo_rssi->tagid[1]));

				sprintf(epcdata, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
									u8(pTaginfo_rssi->tagid[2]),
									u8(pTaginfo_rssi->tagid[3]),
									u8(pTaginfo_rssi->tagid[4]),
									u8(pTaginfo_rssi->tagid[5]),
									u8(pTaginfo_rssi->tagid[6]),
									u8(pTaginfo_rssi->tagid[7]),
									u8(pTaginfo_rssi->tagid[8]),
									u8(pTaginfo_rssi->tagid[9]),
									u8(pTaginfo_rssi->tagid[10]),
									u8(pTaginfo_rssi->tagid[11]),
									u8(pTaginfo_rssi->tagid[12]),
									u8(pTaginfo_rssi->tagid[13])
									);

				rssi = pTaginfo_rssi->tagid[14];
				rssi = (rssi << 8) | ((pTaginfo_rssi->tagid[15]) & 0xff);
				sprintf(rssidata, "%f dBm",(float)(rssi/10.0));
			    printf("data:%s~%s~%s~%d~%ld\r\n\r\n",epcdata, pcbits, rssidata, ant_id, now);
				pTaginfo_rssi++;
			}

		}
	}
	else if (!strcmp(cgi_env, "region=1"))
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
