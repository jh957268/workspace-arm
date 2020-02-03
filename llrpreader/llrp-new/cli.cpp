// llrp.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
//#include <afxwin.h>
#include <iostream>
#include <string>
 #include <sstream>
#include "CWatchDogFeeder.h"
#include "iReaderapi.h"
#include "llrp_MntServer.h"
#include "debug_print.h"

using namespace std;

#define IREADER_TEST 1
#if IREADER_TEST
static UINT8 ttagrbuf[8192];
#endif

static const char  *pcMSG_INVALID_NUM_ARGS = "Invalid number of arguments";
static const char  *pcMSG_INVALID_ARG = "Invalid argument";
static const char  *pcMSG_INVALID_CMD = "Invalid command";

typedef const vector<const char*> ArgvType;
typedef int (*cliFuncPtr)(ArgvType  &argv);
typedef char CHARARRY[20];

int process_show_agent_status(ArgvType  &argv);
int process_test_reader(ArgvType  &argv);
int process_show_debug_level(ArgvType  &argv);
int process_set_debug_level(ArgvType  &argv);
int process_help(ArgvType  &argv);
int process_show_ant_list(ArgvType  &argv);
int process_rescan_slave(ArgvType &argv);
int process_show_slave_status(ArgvType &argv);

struct cli_function
{
	const char *pCmd;
	cliFuncPtr	pfunction;
	const char  *pHelp;
};

struct cli_function	cli_function_list[] =
{
	{"show_agent_status", process_show_agent_status, "show_agent_status"},
	{"test_reader", process_test_reader, "test_reader <antid> <power> <loop_count>"},
	{"show_debug_level", process_show_debug_level, "show_debug_level"},
	{"set_debug_level", process_set_debug_level, "set_debug_level <newlevel>"},
	{"show_ant_list", process_show_ant_list, "show_ant_list <ip_address>"},
	{"rescan_slave", process_rescan_slave, "rescan_slave <ip_address> <rf_port"},
	{"show_slave_status", process_show_slave_status, "show_slave_status <ip_address>"},
	{"help", process_help, "help"}
};

void
rescan_slave
(
	const char *ip_address, int rfport
)
{
	void * handle;
	char iReader_ip[32];

	memcpy((void *)iReader_ip, ip_address, strlen(ip_address));

	iReader_ip[strlen(ip_address)] = 0;

	DBG_PRINT(DEBUG_INFO, "Initialize iReader-998, please wait..."NL);
	handle = IReaderApiInit((char *)iReader_ip, 6);

	if (NULL == handle)
	{
		DBG_PRINT(DEBUG_WARNING, " iReader Init Fails, testing abort."NL);
		return;
	}
	DBG_PRINT(DEBUG_INFO, "Initialize iReader-998 success, start rescanning on rf port %d..."NL, rfport );
	INT32 status = IReaderApiSyncChannel(handle, rfport);

	if (IREADER_SUCCESS == status)
	{
		DBG_PRINT(DEBUG_INFO, " Rescan rf port %d successes"NL, rfport); 
	}
	else
	{
		DBG_PRINT(DEBUG_INFO, " Rescan rf port %d fails"NL, rfport); 
	}
	return;
}

void test_iReader(int antID, int pwr, int test_loop)
{
	// CWatchDogFeeder *pWtd = new CWatchDogFeeder();

	int i;
	short rssi;
	struct taginfo_rssi *pTaginfo_rssi;
	IReader * handle;
	int ttagCount;

	DBG_PRINT(DEBUG_INFO, "Initialize iReader-998, please wait..."NL);
	handle = IReader::getInstance();

	if (NULL == handle)
	{
		DBG_PRINT(DEBUG_WARNING, " iReader Init Fails, testing abort."NL);
		return;
	}
	DBG_PRINT(DEBUG_INFO, "Initialize iReader-998 success, start reading testing..."NL);
	int total_tag_read = 0;
	if (NULL != handle)
	{
	  for (int k = 0; k < test_loop; k++)
	  {
		ttagCount = 0;
		IReaderApiReadTagsMetaDataRSSI((void *)handle, antID, pwr, &ttagCount, (struct taginfo_rssi *)ttagrbuf);
		if (ttagCount == 0)
		{
			printf("Read tag fail"NL);
			continue;
		}
		total_tag_read += ttagCount;
		pTaginfo_rssi = (struct taginfo_rssi *)ttagrbuf;
		printf ("Loop count = %d, Tags read number = %d"NL, k, ttagCount);
		for (i = 0; i < ttagCount; i++)
		{
			printf("Tag %d :"NL, i+1);
			printf("PC bits	: %02x %02x"NL, u8(pTaginfo_rssi->tagid[0]), u8(pTaginfo_rssi->tagid[1]));
			printf("EPC data :");
			for (int j = 0; j < 12; j++)
				printf("%02x", u8(pTaginfo_rssi->tagid[2 + j]));
			printf(NL);
			rssi = pTaginfo_rssi->tagid[14];
			rssi = (rssi << 8) | ((pTaginfo_rssi->tagid[15]) & 0xff);
			printf("RSSI : %02x %02x"NL, u8(pTaginfo_rssi->tagid[14]), u8(pTaginfo_rssi->tagid[15]));
			printf("RSSI : %d, %f"NL, rssi, (float)(rssi/10.0));
			pTaginfo_rssi++;
		}
	  }
	}

	printf("Total tags read = %d"NL, total_tag_read);
	IReaderApiClose(handle);
}

//std::vector<std::string *>	cmdList;

//std::vector<const char *>	parmList;

void process_cli_command(std::string cmd_string)
{
	// cout << cmd_string << endl;
	CHARARRY cmdArray[10];
	std::vector<const char *>	parmList;

	std::stringstream os(cmd_string);          //a standard stringstream which parses 's'
	std::string temp;                 //a temporary string
  
	// std::cout <<"s is: " <<cmd_string <<std::endl;
  
#if 0
	while (os >> temp)                //the stringstream makes temp a token
		std::cout <<temp <<std::endl;   //and deletes that token from itself 
#endif
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
		argc++;
	}

#if 0
	for ( int ii = 0; ii < cmdList.size(); ii++ )
    {
		std::cout << *cmdList[ii] <<std::endl;
    }
#endif

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
	DBG_PRINT(DEBUG_WARNING, "%s"NL, pcMSG_INVALID_CMD);
}

int
process_show_agent_status
(
	ArgvType  &argv
)
{
	if (argv.size() != 1)
		return -1;
	LLRP_MntServer::getInstance(0)->showAgentStatus();
	return 0;
}

int
process_show_debug_level
(
	ArgvType  &argv
)
{
	if (argv.size() != 1)
	{
		DBG_PRINT(DEBUG_WARNING, "%s"NL, pcMSG_INVALID_NUM_ARGS);
		return -1;
	}
	show_Debug_Level();
	return 0;
}

int
process_set_debug_level
(
	ArgvType  &argv
)
{
	if (argv.size() != 2)
	{
		DBG_PRINT(DEBUG_WARNING, "%s"NL, pcMSG_INVALID_NUM_ARGS);
		return -1;
	}
	int newlevel = atoi(argv[1]);
	set_Debug_Level(newlevel);
	return 0;
}

int
process_test_reader
(
	ArgvType  &argv
)
{
	if (argv.size() != 4)
	{
		DBG_PRINT(DEBUG_WARNING, "%s"NL, pcMSG_INVALID_NUM_ARGS);
		return -1;
	}

	int antid = atoi(argv[1]);
	int pwr = atoi(argv[2]);
	int loop_count = atoi(argv[3]);
	test_iReader(antid, pwr, loop_count);
	return 0;
}

int
process_rescan_slave
(
	ArgvType  &argv
)
{
	if (argv.size() != 3)
	{
		DBG_PRINT(DEBUG_WARNING, "%s"NL, pcMSG_INVALID_NUM_ARGS);
		return -1;
	}

	int rfport = atoi(argv[2]);
	rescan_slave(argv[1], rfport);
	return 0;
}

int
process_help
(
	ArgvType  &argv
)
{
	for (unsigned int i = 0; i < (sizeof(cli_function_list)/sizeof(struct cli_function)); i++)
	{
		struct cli_function *pCLI_function;

		pCLI_function = &cli_function_list[i];
		DBG_PRINT(DEBUG_INFO,"%s"NL, pCLI_function->pHelp);
	}
	return 0;
}

int
process_show_ant_list
(
	ArgvType  &argv
)
{
	IReader * iReaderHandle;

	if (argv.size() != 2)
	{
		DBG_PRINT(DEBUG_WARNING, "%s"NL, pcMSG_INVALID_NUM_ARGS);
		return -1;
	}
	DBG_PRINT(DEBUG_INFO,"Initialize iReader %s, please wait..."NL, argv[1]);
	iReaderHandle = IReader::getInstance();;

	if ( 0 == iReaderHandle)
	{
		DBG_PRINT(DEBUG_INFO,"Initialize iReader Fails!!"NL);
		return (-1);
	}

	DBG_PRINT(DEBUG_INFO,"Initialize iReader Success!!"NL);
	DBG_PRINT(DEBUG_INFO,"Retrieving connected antennas list..Please wait.."NL);
	// Now rescan all the RF Port to get the antenna list
	INT32 status;

	for (int i = 1; i <= 8; i++)
	{
		status = IReaderApiSyncChannel(iReaderHandle, i);
		if (IREADER_SUCCESS != status)
		{
			DBG_PRINT(DEBUG_INFO,"Rescan iReader Channel fails, close iReader Handler."NL);
			// IReaderApiClose(iReaderHandle);    never close iReader
			return (status);
		}
	}

	int antCount, antList[256];
	status = IReaderApiGetAntList(iReaderHandle, &antCount, antList);

	if (IREADER_SUCCESS != status)
	{
		// IReaderApiClose(iReaderHandle);       never close IReader
		return status;
	}
	DBG_PRINT(DEBUG_INFO,"Connected antennas List:"NL);
	DBG_PRINT(DEBUG_INFO,"{ ");
	for (int i = 0; i < antCount; i++)
	{
		DBG_Printf("%d ", antList[i]);
	}
	DBG_Printf(" }"NL);
	// IReaderApiClose(iReaderHandle);             never close IReader
	return 0;
}

int
process_show_slave_status
(
	ArgvType  &argv
)
{
	void * iReaderHandle;
	UINT8 stats[40];

	if (argv.size() != 2)
	{
		DBG_PRINT(DEBUG_WARNING, "%s"NL, pcMSG_INVALID_NUM_ARGS);
		return -1;
	}
	DBG_PRINT(DEBUG_INFO,"Initialize iReader %s, please wait..."NL, argv[1]);
	iReaderHandle = IReaderApiInit((char *)argv[1], REGION_USA);

	if ( 0 == iReaderHandle)
	{
		DBG_PRINT(DEBUG_INFO,"Initialize iReader Fails!!"NL);
		return (-1);
	}
	// IReaderApiGetSlaveStat(iReaderHandle, stats);     temporary delete to compile

	for (int i = 0; i < 32; i++)
	{
		printf("Slave %d %s"NL, i+1, ((stats[i] == 1) ? "Attached" : "Not Attached"));
	}

	return 0;
}
