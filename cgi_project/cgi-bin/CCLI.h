#ifndef _CCLI_H_
#define _CCLI_H_

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
 
#include "iReaderapi.h"
 
using namespace std;

#if 0 
static const char  *pcMSG_INVALID_NUM_ARGS = "Invalid number of arguments";
static const char  *pcMSG_INVALID_ARG = "Invalid argument";
static const char  *pcMSG_INVALID_CMD = "Invalid command";
#endif

typedef const vector<const char*> ArgvType;
typedef int (*cliFuncPtr)(ArgvType  &argv);
typedef char CHARARRY[40];

struct cli_function
{
	const char *pCmd;
	cliFuncPtr	pfunction;
	const char  *pHelp;
};

class CCLI
{
    public:
		        CCLI();
				~CCLI();

	
		static  void    process_cli_command(std::string cmd_string);
		
		static int process_seltag(ArgvType  &argv);
		static int process_inserttag(ArgvType &argv);
		static int process_startmonitor(ArgvType  &argv);
		static int process_stopmonitor(ArgvType &argv);
		static int process_setantpower(ArgvType &argv);
		static int process_antmap(ArgvType &argv);		
		static int process_rescanchn(ArgvType &argv);
		static int process_getregion(ArgvType &argv);
		static int process_getsearchtimeout(ArgvType &argv);
		static int process_setsearchtimeout(ArgvType &argv);
		static int process_starttagtodb(ArgvType &argv);
		static int process_stoptagtodb(ArgvType &argv);
		static int process_startstreamtag(ArgvType &argv);
		static int process_stopstreamtag(ArgvType &argv);
		static int process_readtagonce(ArgvType &argv);
		static int process_getmoduletemperature(ArgvType &argv);
		static IReader *handle;

    private:
		static char ttagrbuf[2048];
		static char antmap[1280];
		static char recordrbuf[2048];

};


#endif // _CCLI_H_