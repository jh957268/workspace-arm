
#include  "debug_print.h"

using namespace  std;

const char *debug_string [] =  {"NONE: ", "DBG: ", "INFO: ", "WARNING: ", "CRITICAL: "};

int iDebug_Level = DEBUG_ALL;

bool set_Debug_Level(int newLevel)
{
	if ((newLevel < DEBUG_ALL) || (newLevel > DEBUG_CRITICAL))
		return false;
	iDebug_Level = newLevel;
	return true;
}

void show_Debug_Level(void)
{
	fprintf(stdout, "Current Debug Level = %s"NL, debug_string[iDebug_Level]);
	
}