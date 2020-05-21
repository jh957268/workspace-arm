#include "CCLI.h"

IReader* CCLI::handle = 0;
char CCLI::ttagrbuf[2048] = {0};
char CCLI::recordrbuf[2048] = {0};
char CCLI::antmap[1280] = {0};

#define DATABASE_MAGIC		0xFFEE
#define DATABASE_USER_MAGIC	0xFFDD

struct cli_function	cli_function_list[] =
{
	{"seltag", CCLI::process_seltag, "select * from DB"},
	{"inserttag", CCLI::process_inserttag, "insert * from DB"},
	{"startmonitor", CCLI::process_startmonitor, "Start Monitoring"},
	{"stoptmonitor", CCLI::process_stopmonitor, "Stop Monitoring"},
	{"starttagtodb", CCLI::process_starttagtodb, "Start read tag to db"},
	{"stoptagtodb", CCLI::process_stoptagtodb, "Stop read tag to db"},		
	{"setantpower", CCLI::process_setantpower, "setantpower <ant> <power>"},
	{"antmap", CCLI::process_antmap, "antmap"},
	{"rescanchn", CCLI::process_rescanchn, "rescanchn <1..8>"},
	{"getregion", CCLI::process_getregion, "getregion"},
	{"startstreamtag", CCLI::process_startstreamtag, "Start straming tags"},
	{"stopstreamtag", CCLI::process_stopstreamtag, "Stop straming tags"},	
	{"readtagonce", CCLI::process_readtagonce, "read tags once"},	
	{"getsearchtimeout", CCLI::process_getsearchtimeout, "getsearchtimeout"},
	{"setsearchtimeout", CCLI::process_setsearchtimeout, "setsearchtimeout"},
	{"getmoduletemp", CCLI::process_getmoduletemperature, "getmoduletemp"}	
};

CCLI::CCLI()
{}

CCLI::~CCLI()
{}

int
CCLI::process_readtagonce(ArgvType  &argv)
{
	int ant_id, ttagCount, power;
	struct taginfo_rssi *pTaginfo_rssi;
	char pcbits[8], epcdata[32], rssidata[20];
	short rssi;
	char tmpbuff[128];
	int i;
	
	// Ireader read tags once

	ant_id = atoi(argv[1]);
	power = atoi(argv[2]);
	int ret = IReaderApiReadTagsMetaDataRSSI(handle, ant_id, power, &ttagCount, (struct taginfo_rssi *)ttagrbuf);

	if (IREADER_SUCCESS != ret)
	{
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
	return 0;
}

int
CCLI::process_stopstreamtag(ArgvType  &argv)
{
	int ret = IReaderApiStartExecutor(handle, 0, 0);
	if (ret != 0)
	{
		printf("Command Fail");
	}
	else
	{
		printf("ok");
	}
	return 0;
}

int
CCLI::process_startstreamtag(ArgvType  &argv)
{
	int ant_id, ttagCount;
	struct taginfo_rssi *pTaginfo_rssi;
	char pcbits[8], epcdata[32], rssidata[20];
	short rssi;

	int ret = IReaderApiStartExecutor(handle, 1, 0);
	if (IREADER_SUCCESS != ret)
	{
		return 0;
	}

	// Ireader read asyn tags
	while (1)
	{
		int ret = IReaderApiGetTagsMetaDataRSSI(handle, &ant_id, &ttagCount, (struct taginfo_rssi *)ttagrbuf);
		if (IREADER_SUCCESS != ret)
		{
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
			fflush(stdout);
			pTaginfo_rssi++;
		}
	}
	return 0;
}

int
CCLI::process_starttagtodb(ArgvType  &argv)
{
	int ret = IReaderApiStartExecutor(handle, 1, DATABASE_MAGIC);
	if (ret != 0)
	{
		printf("Command Fail");
	}
	else
	{
		printf("ok");
	}
	return 0;
}

int
CCLI::process_stoptagtodb(ArgvType  &argv)
{
	int ret = IReaderApiStartExecutor(handle, 0, DATABASE_MAGIC);
	if (ret != 0)
	{
		printf("Command Fail");
	}
	else
	{
		printf("ok");
	}
	return 0;
}

int
CCLI::process_getmoduletemperature(ArgvType  &argv)
{
	int temp;
	
	int ret = IReaderApiEquipTempGet(handle, &temp);
	if (IREADER_SUCCESS != ret)
	{
		printf("0");
	}
	else
	{
		printf("%d", temp);
	}
	return 0;	
}

int
CCLI::process_getregion(ArgvType  &argv)
{
	int region;
	int ret = IReaderApiGetRegion(handle, &region);
	
	if (IREADER_SUCCESS != ret)
	{
		printf("0");
	}
	else
	{
		printf("%d", region);
	}
	return 0;
}

int
CCLI::process_getsearchtimeout(ArgvType  &argv)
{
	int timeout;
	
	int ret = IReaderApiGetSearchTimeout(handle, &timeout);
	if (IREADER_SUCCESS != ret)
	{
		printf("0");
	}
	else
	{
		printf("%d", timeout);
	}
	return 0;	
}

int
CCLI::process_setsearchtimeout(ArgvType  &argv)
{

	int timeout = atoi(argv[1]);	
	int ret = IReaderApiTagSearchTimeout(handle, timeout);
	if (IREADER_SUCCESS != ret)
	{
		printf("0");
	}
	else
	{
		printf("%d", timeout);
	}
	return 0;	
}

int
CCLI::process_rescanchn(ArgvType  &argv)
{
	int channel = atoi(argv[1]);

	int ret = IReaderApiScanSlave(handle, channel);

	if (IREADER_SUCCESS != ret)
	{
		printf("fail");
	}
	else
	{
		printf("Success");
	}
	return 0;
}

int
CCLI::process_antmap(ArgvType  &argv)
{
	memset(antmap, 0, 270);
	IReaderApiGetAntMap(handle, antmap);

	for (int i = 0; i < 256; i++)
	{
		antmap[i] += 0x30;
	}
	antmap[strlen(antmap)] = 0;
	printf("%s", antmap);
	return 0;	
}

int
CCLI::process_setantpower(ArgvType  &argv)
{
	int ant_id = atoi(argv[1]);
	int power = atoi(argv[2]);
	int ret = IReaderApiSetPowerLevel(handle, ant_id, power);

	if (IREADER_SUCCESS != ret)
	{
		printf("fail");
	}
	else
	{
		printf("Success");
	}
	return 0;	
}

int
CCLI::process_startmonitor(ArgvType  &argv)
{
	int ret = IReaderApiStartExecutor(handle, 1, DATABASE_USER_MAGIC);
	if (ret != 0)
	{
		printf("Command Fail");
	}
	else
	{
		printf("ok");
	}
	return 0;
}

int
CCLI::process_stopmonitor(ArgvType  &argv)
{
	int ret = IReaderApiStartExecutor(handle, 0, DATABASE_USER_MAGIC);
	if (ret != 0)
	{
		printf("Command Fail");
	}
	else
	{
		printf("ok");
	}
	return 0;
}

int 
CCLI::process_inserttag(ArgvType  &argv)
{
	char set_str[128];
	int ret;
	
	sprintf(set_str, "tagval=%s&antloc=%s&action=%s&submit=Insert", argv[1], argv[2], argv[3]);
	
	ret = IReaderApiDBInsertTag(handle, set_str);    // select all limit 0 offset 0 from table
	
	if (IREADER_SUCCESS != ret)
	{
		printf("command fails");
		return 0;
	}
	printf("Command sucess");
	return 0;	
}

int 
CCLI::process_seltag(ArgvType  &argv)
{
	int ttagCount;
	char db_record[128];
	int total_record = 0;
	ttagrbuf[0] = 0;
	int ret;
	
	int table = atoi(argv[1]);      // table is 0  or 1 (user asset monitoring db)
	int offset = atoi(argv[2]);;

	ret = IReaderApiDBSelectAll(handle, 30, offset, table);    // select all limit 0 offset 0 from table
	if (IREADER_SUCCESS != ret)
	{
		//printf("command fails\n");
		return 0;
	}

	// Ireader read asyn tags
	while (1)
	{
		ttagCount = 0;
		ret = IReaderApiGetTagDBRecord(handle, db_record, &ttagCount);

		if (IREADER_SUCCESS != ret)
		{
			IReaderApiClose(handle);
			return 0;
		}

		if (ttagCount == 0)   // ttagCount store  msg len, is atually msg_len
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
	return (0);
}

void 
CCLI::process_cli_command(std::string cmd_string)
{
	CHARARRY cmdArray[10];
	std::vector<const char *>	parmList;

	std::stringstream os(cmd_string);	//a standard stringstream which parses 's'
	std::string temp;                 	//a temporary string
	
	int argc = 0;
	while (os >> temp)	
	{	
		//std::string *news = new string;		//the stringstream makes temp a token
		//*news = temp;
		memcpy((void *)cmdArray[argc], temp.c_str(), temp.size());
		cmdArray[argc][temp.size()] = 0;
		//cmdList.push_back(news);
		parmList.push_back(cmdArray[argc]);
		//std::cout <<*news <<std::endl;		//and deletes that token from itself 

		//printf("CMD = %s\n", cmdArray[argc]);
		argc++;
	}

	for (unsigned int i = 0; i < (sizeof(cli_function_list)/sizeof(struct cli_function)); i++)
	{
		struct cli_function *pCLI_function;

		pCLI_function = &cli_function_list[i];

		if (strcmp(pCLI_function->pCmd, cmdArray[0]) == 0)
		{
			pCLI_function->pfunction(parmList);
			return;
		}
	}	
}

