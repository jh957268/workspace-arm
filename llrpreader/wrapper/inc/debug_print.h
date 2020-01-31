#ifndef __DEBUG_PRINT__
#define __DEBUG_PRINT__

#include <iostream>
#include  <stdio.h>
#include  <signal.h>
#include  <string.h>
#include  <errno.h>

#define NL "\n"

#define DEBUG_ALL			1
#define	DEBUG_INFO			2
#define	DEBUG_WARNING		3
#define	DEBUG_CRITICAL		4

extern const char *debug_string [];
extern int iDebug_Level;

#define DBG_Printf(... )	do {		\
								fprintf(stderr, __VA_ARGS__);	\
							} while (0);

#if 0
#define DBG_PRINT(level, ... )	do {		\
								if ( (level <= DEBUG_CRITICAL) && (level >= iDebug_Level) )	 \
								{											\
									fprintf(stdout, "%s %s", __TIME__, debug_string[level]);	\
									fprintf(stdout, __VA_ARGS__);	\
								}									\
							} while (0);
									// time_str.c_str()[time_str.size() - 1] = 0;

#endif
#define DBG_PRINT(level, ... )	do {		\
								if ( (level <= DEBUG_CRITICAL) && (level >= iDebug_Level) )	 \
								{												\
									if ( 1 /*level >= DEBUG_WARNING */)				\
									{											\
										time_t rawtime;							\
										struct tm * timeinfo;					\
										time (&rawtime);						\
										timeinfo = localtime (&rawtime);		\
										printf ("Current local time and date: %s", asctime(timeinfo)); \
									}		\
									else	\
									{		\
										fprintf(stdout, "%s", debug_string[level]);	\
									}		\
									fprintf(stdout, __VA_ARGS__);	\
								}									\
							} while (0);

#define debug_print(level, fmt, ...) \
            do { if ((level <= DEBUG_CRITICAL) && (level >= iDebug_Level) ) {fprintf(stdout, debug_string[level]); fprintf(stderr, fmt, __VA_ARGS__);} } while (0)

#if 0
#if 1
#define DBG_Printf(fmt, args, ...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args, ...)    /* Don't do anything in release builds */
#endif
#endif

#if 0
#define DBG_Printf		printf
#endif

bool set_Debug_Level(int newLevel);
void show_Debug_Level(void);

#endif
