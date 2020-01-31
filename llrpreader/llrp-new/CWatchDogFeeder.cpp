//=============================================================================
//
/// @file  CWatchDogFeeder.cpp
/// @brief  Definitions for Watch Dog Feeder class
//
//=============================================================================

#include  <fcntl.h>

#include  "CWatchDogFeeder.h"
//#include  "OwSystemTime.h"

//=============================================================================

#define  WDIOC_SETTIMEOUT  0xC0045706
#define  WDIOC_GETTIMEOUT  0x80045707
#define  WDIOC_KEEPALIVE   0x80045705
#define  WATCHDOG_DEVICE_STRING  "/dev/watchdog"
static const char  MSG_MISSING[] = "Missing Receiver parameter ";

//=============================================================================

CWatchDogFeeder::CWatchDogFeeder
(
) : OwTask( OwTask::MEDIUM_HIGH, 1024, "CWatchDogFeeder" ),
	mbDebug( false )
{
	const char  *pcName = "CWatchDogFeeder::CWatchDogFeeder";
	const char  *devName = WATCHDOG_DEVICE_STRING;


	//-----------------------------------------------------
	// Begin feeding the dog.

	run();

} // CWatchDogFeeder::CWatchDogFeeder()


//=============================================================================

CWatchDogFeeder::~CWatchDogFeeder()
{

} // CWatchDogFeeder::~CWatchDogFeeder()


//=============================================================================

void
CWatchDogFeeder::main( OwTask *task )
{
	while ( true )
	{
		// Feed the dog

		OwTask::sleep( 10000 );

	} // while
} // CWatchDogFeeder::main()


//=============================================================================
