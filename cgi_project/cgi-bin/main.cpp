#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "iReaderapi.h"
char antmap[1280];

char ttagrbuf[2048];
char recordrbuf[2048];
#define DATABASE_MAGIC		0xFFEE
#define DATABASE_USER_MAGIC	0xFFDD

int inserttag_main(IReader *handle);

int main(void) 
{
	IReader *handle;
	int ret;
	int region;
	const char *cgi_env = getenv("QUERY_STRING");
	//const char *cgi_env = "readtag=6-1-2500";
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
	//printf("connecting...\n");
	ret = IReaderApiConnect(handle, (char *)"127.0.0.1");
	if (IREADER_SUCCESS != ret)
	{
		printf("Connect IReader Fails");
		IReaderApiClose(handle);
		exit(-1);
	}
	if (!strcmp(cgi_env, "inserttag"))
	{
		inserttag_main(handle);
		// IReaderApiClose(handle);
		return 0;
		
	}
	else if (!strcmp(cgi_env, "antmap=1"))
	{
		memset(antmap, 0, 270);
		ret = IReaderApiGetAntMap(handle, antmap);

		for (int i = 0; i < 256; i++)
		{
			antmap[i] += 0x30;
#if 0
			if (i & 1)
			{
				antmap[i] += 0x31;
			}
			else
			{
				antmap[i] += 0x30;
			}
#endif			
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
		antmap[strlen(antmap)] = 0;
		printf("%s", antmap);
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strcmp(cgi_env, "readtag=0"))
	{
		ret = IReaderApiStartExecutor(handle, 0, 0);
		printf("ok");
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strcmp(cgi_env, "readtag=2"))
	{
		ret = IReaderApiStartExecutor(handle, 1, DATABASE_MAGIC);
		printf("ok");
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strcmp(cgi_env, "readtag=3"))
	{
		ret = IReaderApiStartExecutor(handle, 0, DATABASE_MAGIC);
		printf("ok");
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strcmp(cgi_env, "readtag=4"))
	{
		ret = IReaderApiStartExecutor(handle, 1, DATABASE_USER_MAGIC);
		printf("ok");
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strcmp(cgi_env, "readtag=5"))
	{
		ret = IReaderApiStartExecutor(handle, 0, DATABASE_USER_MAGIC);
		printf("ok");
		IReaderApiClose(handle);
		return 0;
	}	
	else if (!strcmp(cgi_env, "readtag=1"))
	{
		int ant_id, ttagCount;
		struct taginfo_rssi *pTaginfo_rssi;
		char pcbits[8], epcdata[32], rssidata[20];
		short rssi;

		ret = IReaderApiStartExecutor(handle, 1, 0);
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
	else if (!strncmp(cgi_env, "readtag=6", 9))
	{
		int ant_id, ttagCount, power;
		struct taginfo_rssi *pTaginfo_rssi;
		char pcbits[8], epcdata[32], rssidata[20];
		short rssi;
		char *tmp1, *tmp2;
		char tmpbuff[128];
		int i;
	
		// Ireader read tags once

		strcpy(tmpbuff, cgi_env);
		tmp1 = &tmpbuff[10];
		tmp2 = strchr(tmp1, '-');
		*tmp2++ = 0;
		ant_id = atoi(tmp1);
		power = atoi(tmp2);
		ret = IReaderApiReadTagsMetaDataRSSI(handle, ant_id, power, &ttagCount, (struct taginfo_rssi *)ttagrbuf);

		if (IREADER_SUCCESS != ret)
		{
			IReaderApiClose(handle);
			return 0;
		}

		pTaginfo_rssi = (struct taginfo_rssi *)ttagrbuf;
		time_t now = time(NULL);

		recordrbuf[0] = 0;
		int total_record = 0;

 		for (i = 0; i < ttagCount; i++)
		{
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
			if (total_record != 0)
			{
				strcat(recordrbuf, ";");
			}
			sprintf(tmpbuff, "%s~%s~%s~%d~%ld",epcdata, pcbits, rssidata, ant_id, now);
			strcat(recordrbuf, tmpbuff);
			pTaginfo_rssi++;
			total_record++;
			
		}
		printf("%s", recordrbuf);
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strncmp(cgi_env, "rescanchn", 9))
	{
		int channel = cgi_env[10] - 0x30;

		ret = IReaderApiScanSlave(handle, channel);

		if (IREADER_SUCCESS != ret)
		{
			printf("fail");
		}
		else
		{
			printf("Success");
		}
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strncmp(cgi_env, "setpower1", 9))
	{
		int channel;
		int ant_id, power;
		char *tmp1, *tmp2;
		char tmpbuff[128];
		// Ireader read tags once

		strcpy(tmpbuff, cgi_env);
		tmp1 = &tmpbuff[10];
		tmp2 = strchr(tmp1, '-');
		*tmp2++ = 0;
		ant_id = atoi(tmp1);
		power = atoi(tmp2);
		ret = IReaderApiSetPowerLevel(handle, ant_id, power);

		if (IREADER_SUCCESS != ret)
		{
			printf("fail");
		}
		else
		{
			printf("Success");
		}
		IReaderApiClose(handle);
		return 0;
	}
	else if (!strcmp(cgi_env, "dbtag=1"))
	{
		int ttagCount;
		char db_record[128];

		ret = IReaderApiDBSelectAll(handle, 0, 0, 0);    // select all limit 0 offset 0
		if (IREADER_SUCCESS != ret)
		{
			//printf("command fails\n");
			IReaderApiClose(handle);
			return 0;
		}

		// Ireader read asyn tags
		while (1)
		{
			ret = IReaderApiGetTagDBRecord(handle, db_record, &ttagCount);
			//ret = IReaderApiDBSelectAll(handle);
			//ret = IREADER_SUCCESS;
			//ttagCount = 1;
			//sprintf(db_record, "1~1122334455667788");
			if (IREADER_SUCCESS != ret)
			{
				IReaderApiClose(handle);
				return 0;
			}

			if (ttagCount == 0)
			{
				continue;
			}
		   	printf("Content-Type: text/event-stream\r\n");
		    printf("Cache-Control: no-cache\n\n");
		    printf("data:%s\r\n\r\n", db_record);
		    fflush(stdout);
		}
	}
	else if (!strncmp(cgi_env, "seltag", 6))
	{
		int ttagCount;
		char db_record[128];
		int total_record = 0;
		ttagrbuf[0] = 0;
		int table = 0;

		int offset = atoi(&cgi_env[9]);

		if (cgi_env[7] == '1')
		{
			table = 0;
		}
		else
		{
			table = 1;	
		}
		

		ret = IReaderApiDBSelectAll(handle, 30, offset, table);    // select all limit 0 offset 0 from table
		if (IREADER_SUCCESS != ret)
		{
			//printf("command fails\n");
			IReaderApiClose(handle);
			return 0;
		}

		// Ireader read asyn tags
		while (1)
		{
			ret = IReaderApiGetTagDBRecord(handle, db_record, &ttagCount);
			//ret = IReaderApiDBSelectAll(handle);
			//ret = IREADER_SUCCESS;
			//ttagCount = 1;
			//sprintf(db_record, "1~1122334455667788");
			if (IREADER_SUCCESS != ret)
			{
				IReaderApiClose(handle);
				return 0;
			}

			if (ttagCount == 0)   // ttagCount store  msg len
			{
				continue;
			}
			
			db_record[ttagCount] = 0;

			if (!strcmp(db_record, "done"))
			{
				break;
			}
			if (total_record != 0)
			{
				strcat(ttagrbuf, ";");
			}
			total_record++;
			strcat(ttagrbuf, db_record);
		}
		printf("%s", ttagrbuf);
		IReaderApiClose(handle);
		return 0;
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
