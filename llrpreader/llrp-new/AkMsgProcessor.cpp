//=============================================================================
//
/// @file  AkMsgProcessor.cpp
///
/// Copyright (c) 2009 MTI Laboratory, Inc. USA
///
/// @brief  This file contains the member function for AkMsgProcessor class
//
//=============================================================================
#include  <stdio.h>
#include  <stdlib.h>
#include  <signal.h>
#include  <unistd.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  "CClipTask.h"

#include  <string.h>
#include  <string>
#include  <sstream>

#include  "SPrintf.h"
#include  "FlightRecorder.h"
#include  "AkMsgProcessor.h"
#include  "AkConstants.h"
#include  "AkAlarm.h"
#include  "AkTtlna.h"
#include  "OwSystemTime.h"

#include  "AkRfConfig.h"
#include  "AkMntServer.h"

using namespace  std;
//#define HARDCODED_TEST

//=============================================================================
// Define static data members
AkMsgProcessor*  AkMsgProcessor::spInstance = 0;

static const char  *MSG_INVALID_NUM_ARGS = "Invalid number of arguments";
static const char  *MSG_INVALID_ARG = "Invalid argument";

void  deleteCharArray( const char  *pcName, char  *pCharArray );
void  deleteCharNonArray( const char  *pcName, char  *pCharArray );

//=============================================================================

char*  newCharArray( const char  *pcName, size_t  size );

//=============================================================================

AkMsgBuffer::AkMsgBuffer
(
)
{
	const char  *pcName = "AkMsgBuffer::AkMsgBuffer";
	mState = eNoMsg;
	muiInBuffPos = 0;

	muiMsgBuffPos = 0;
	mpcMessage = newCharArray( pcName, AK_MSG_BUFFER_SIZE );

} // AkMsgBuffer::AkMsgBuffer()


//=============================================================================

unsigned int
AkMsgBuffer::getMsgCount
(
	void

) const
{
	return  ( mVectorMessages.size() );

} // AkMsgBuffer::getMsgCount()


//=============================================================================

char*
AkMsgBuffer::getMessage
(
	void
)
{
//	const char  *pcName = "AkMsgBuffer::getMessage";
	unsigned long  ulSize =  mVectorMessages.size();

	if ( ulSize > 0 )
	{
		char*  pcMessage = mVectorMessages.front();
//		printf( "%s - size %u erase 0x%08X"NL, pcName, ulSize, mpcMessage );
		mVectorMessages.erase( mVectorMessages.begin() );

		return  ( pcMessage );
	}
	else
	{
		return  ( 0 );
	}

} // AkMsgBuffer::getMessage()


//=============================================================================
// processBuffer

AkMsgBuffer::eBufferState
AkMsgBuffer::processBuffer
(
	const char*  pcBuffer,
	unsigned int  uiBufferLength
)
{
	const char  *pcName = "AkMsgBuffer::processBuffer";
	unsigned int  uiCharsAvailable = uiBufferLength;
	muiInBuffPos = 0;
	char  theChar;
	if( AK_MSG_BUFFER_SIZE < strlen(pcBuffer) )
	{
		SPrintf("Length of request message = (%d) is too long !!!"NL, strlen(pcBuffer));	
		return eOverSizeMsg;	
	}
	while ( uiCharsAvailable > 0 )
	{
//		SPrintf( "L %u"NL, uiCharsAvailable );

		theChar = pcBuffer[ muiInBuffPos ];

		// Ignore any carriage returns
		if ( '\r' != theChar )
		{
			if ( eNoMsg == mState )
			{
				// Look for start of message '['
				if ( '[' == theChar )
				{
					mState = ePartialMsg;
//					SPrintf( "eNoMsg -> ePartialMsg, L %u"NL, uiCharsAvailable );
					mpcMessage[ muiMsgBuffPos ] = theChar;
					muiMsgBuffPos++;
				}
			}
			else if ( ePartialMsg == mState )
			{
				// Look for end of message
				if ( ']' == theChar )
				{
					mState = eCompleteMsg;
//					SPrintf( "ePartialMsg -> eCompleteMsg, L %u"NL, uiCharsAvailable );
				}
				mpcMessage[ muiMsgBuffPos ] = theChar;
				muiMsgBuffPos++;
			}
		}

		if ( eCompleteMsg == mState )
		{
//			SPrintf( "eCompleteMsg -> eNoMsg L %u"NL, uiCharsAvailable );
			mpcMessage[ muiMsgBuffPos ] = 0;
//			printf( "%s - push 0x%08X"NL, pcName, (unsigned int)mpcMessage );
			mVectorMessages.push_back( mpcMessage );
			mpcMessage = newCharArray( pcName, AK_MSG_BUFFER_SIZE );
			muiMsgBuffPos = 0;
			mState = eNoMsg;
		}
		muiInBuffPos++;
		uiCharsAvailable--;

	} // while

	return ( mState );

} // AkMsgBuffer::processBuffer()


//=============================================================================
// constructor

AkMsgProcessor::AkMsgProcessor():
	OwErrand( ERRAND_INTERVAL, "AkMsgProcessor", 3 ),
	eventCounter( 0 ),
    eventThreshold ( AK_EVENT_THROTTLE ),
    mbDisplayEnabled( true ),
	mbGetTemperatureHasBeenRecorded( false ),
	mbSetTimestampHasBeenRecorded( false ),
	mbGetUnitStateHasBeenRecorded( false ),
	mbGetCarrierCfgHasBeenRecorded( false ),
	mbGetIntAlmStatHasBeenRecorded( false ),
	mbGetActiveSW_HasBeenRecorded( false ),
	mbGetInvent3GPP_HasBeenRecorded( false ),
	mbGetTxPower_HasBeenRecorded( false ),
	mbGetTx2Power_HasBeenRecorded( false ),
	mbFirstSetMainCountersHasBeenRecorded( false ),
	mbFirstGetCarrierCountersHasBeenRecorded( false ),
    meThrottle( AkMsgProcessor::THROTTLE_NONE )
{
	memset(mcTermName,0,TERMNAME_LENGTH);
	start();	// starts errand runner

	SPrintf( "AkMsgProcessor created"NL );

	mpoGetCarrCountersCounter = new LoopCounter( COUNTER_REPORTING_PERIOD );
	mpoSetMainCountersCounter = new LoopCounter( COUNTER_REPORTING_PERIOD );

	// Register CLI commands for debugging
	CommandSubscriber<AkMsgProcessor>::subscribe(
			"akgets32number",
			&AkMsgProcessor::process_akgets32number,
			"Test akGetSigned32BitNumber function"NL
			"Syntax: akgets32number <data to convert>"NL );

	CommandSubscriber<AkMsgProcessor>::subscribe(
			"akgetu32number",
			&AkMsgProcessor::process_akgetu32number,
			"Test akGetUnsigned32BitNumber function"NL
			"Syntax: akgetu32number <data to convert>"NL );

	CommandSubscriber<AkMsgProcessor>::subscribe(
			"akgetu64number",
			&AkMsgProcessor::process_akgetu64number,
			"Test akGetUnsigned64BitNumber function"NL
			"Syntax: akgetu64number <data to convert>"NL );

	CommandSubscriber<AkMsgProcessor>::subscribe(
			"ard",
			&AkMsgProcessor::process_ard,
			"Toggle display of ARD messages"NL
			"Syntax: ard"NL );

	CommandSubscriber<AkMsgProcessor>::subscribe(
			"console",
			&AkMsgProcessor::process_console,
			"Redirect RCS output message to other terminal"NL
			"Syntax: console [/dev/pts/x], which x is the id"NL );

	CommandSubscriber<AkMsgProcessor>::subscribe(
			"set_evt_threshold",
			&AkMsgProcessor::process_set_evt_threshold,
			"Set throttle threshold for event messages"NL
			"Syntax: set_evt_threshold <int>"NL );

	CommandSubscriber<AkMsgProcessor>::subscribe(
			"disable_ard_msg",
			&AkMsgProcessor::process_disable_ard_msg,
			"Disable display of ARD messages"NL );

	CommandSubscriber<AkMsgProcessor>::subscribe(
			"enable_ard_msg",
			&AkMsgProcessor::process_enable_ard_msg,
			"Enable display of ARD messages"NL );

} // AkMsgProcessor::AkMsgProcessor()


//=============================================================================
// process
//
// The message received may be a concatenation of multiple messages
// or may be a portion of a message.

void
AkMsgProcessor::process
(
	AkController*  pController,
	char*  pcBuffer,
	unsigned int  uiBufferLength
)
{
	const char  *pcName = "AkMsgProcessor::process";
	mapControllerBuffer::const_iterator  it = mMapMsgBuffers.find( pController );

	if ( mMapMsgBuffers.end() == it )
	{
		// This controller has not been referenced before
//		SPrintf( NL"New controller"NL );
		AkMsgBuffer*  pMsgBuffer = new AkMsgBuffer();
		mMapMsgBuffers.insert( make_pair( pController, pMsgBuffer ) );
	}
	AkMsgBuffer*  pMB = mMapMsgBuffers[ pController ];

	if ( AkMsgBuffer::eOverSizeMsg == pMB->processBuffer( pcBuffer, uiBufferLength ) )
	{
	    pController->sendMessage( "["NL "MESSAGE: TYPE=ERROR"NL "TRANSACTION:ID=-1"NL
                       "ERRORIND: ERROR=\"MESSAGE TOO LARGE\" INFO=\"MESSAGE TOO LARGE\"""\""NL "]"NL );
	}

	while ( pMB->getMsgCount() > 0 )
	{
		char*  pcMsg = pMB->getMessage();
		processOneMessage( pController, pcMsg );
		deleteCharArray( pcName, pcMsg );
	}

} // AkMsgProcessor::process()

//=============================================================================
// Create and send SYNTAX_ERROR message
// Syntax error messages must contain (maximum 80) characters 
// of received errored message in INFO data field

void
AkMsgProcessor::sendSyntaxError
(
    AkController* pController, // Controller sending message
    char *message              // Received message containing syntax errors
)
{
    int i;
    char c;
    int len;
    int rspStartLen;
    char rspMsg[180] = "["NL "MESSAGE: TYPE=ERROR"NL "TRANSACTION:ID=-1"NL 
                       "ERRORIND: ERROR=\"SYNTAX ERROR\" INFO=\"";

    // Get length of start of error message
    rspStartLen = strlen(rspMsg);

    // Retrieve length of received errored message - restrict it to max 80 chars
    len = strlen(message);
    if (len > 80)
    {
        len = 80;
    }

    // Copy errored message into INFO data field
    for (i = 0; i < len; i++)
    {
        // Exchange newlines with blanks
        c = (message[i] == '\n') ? ' ' : message[i];
        rspMsg[rspStartLen+i] = c;
    }

    // Concatenate end of error message including NULL termination
    strcat(rspMsg, "\""NL "]"NL);

    // Send response
    SPrintf ( rspMsg );
    pController->sendMessage( rspMsg );
}

//=============================================================================
// processOneMessage
#define  RESULT_BUFFER_SIZE		( MSG_BUFFER_LIMIT + 100 ) 

void
AkMsgProcessor::processOneMessage
(
	AkController*  pController,
	char*  message
)
{
	const char  *pcName = "AkMsgProcessor::processOneMessage";
	FlightRecorder*  poFR = FlightRecorder::getInstance();
	long long  llStartTime_ms = OwSystemTime::get_ms();
	long long  llFinishTime_ms;
	long long  llElapsedTime_ms;

	enum MessageType { GET, SET, ACTION };

	char  result[ RESULT_BUFFER_SIZE ];
	char  *result_ptr;
	char  tempBuffer[ 100 ];

	const char  *cp = message;
	const char  *lineBegin;
	const char  *lineEnd;
	char  savedLineEnd;

	string  word;
	string  strType;
	char  cDelimiter;

	MessageType  msgType;
	int  iTransaction = 0;
	memset( result, 0, RESULT_BUFFER_SIZE );

	bool isSessionReset = false;
	bool isSoftReset = false;
	map< AkController*, string >:: iterator pSessionData;

//	SPrintf( "AkMsgProcessor:: Processing message (%d, 0x%X):"NL "%s"NL,
//			strlen( message ), pController, message );

	if ( mbDisplayEnabled )
	{
		SPrintf( NL"%s"NL, message );
	}

	// Get the "["
	akGetLine( cp, lineBegin, lineEnd );

	if ( lineBegin == 0 )
	{
		return;
	}

	if ( *lineBegin != '[' )
	{
    	sendSyntaxError(pController, message);
		return;
	}

	// Go to next line
	cp = lineEnd + 1;

	//----------------------------------------
	// Start processing MESSAGE attribute
	akGetLine( cp, lineBegin, lineEnd );
	savedLineEnd = *lineEnd;
	*(char*)lineEnd = 0;
	cp = lineBegin;

	// Get "MESSAGE:"
	cDelimiter = akGetWord( cp, word );

	if ( cDelimiter != ':' || word != "MESSAGE" )
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	// Get "TYPE="
	cDelimiter = akGetWord( cp, word );

	if ( cDelimiter != '=' || word != "TYPE" )
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	// Find the value of type
	cDelimiter = akGetWord( cp, strType );

	if ( cDelimiter != ' ' )
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	if ( *cp != 0 )             // for defect 10690
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	if ( strType == "GET" )
	{
		msgType = GET;
	}
	else if ( strType == "SET" )
	{
		msgType = SET;
	}
	else if ( strType == "ACTION" )
	{
		msgType = ACTION;
	}
	else
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	// Restore end of line
	cp = lineEnd + 1;
	*(char*)lineEnd = savedLineEnd;

	//----------------------------------------
	// Start processing TRANSCTION attribute
	akGetLine( cp, lineBegin, lineEnd );
	savedLineEnd = *lineEnd;
	*(char*)lineEnd = 0;
	cp = lineBegin;

	// Get "TRANSACTION:"
	cDelimiter = akGetWord( cp, word );

	if ( cDelimiter != ':' || word != "TRANSACTION" )
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	// Get "ID="
	cDelimiter = akGetWord( cp, word );

	if ( cDelimiter != '=' || word != "ID" )
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	// Find the value of transaction ID
	cDelimiter = akGetSigned32BitNumber( cp, iTransaction );

    if ( *cp != 0 )             // for defect 10690 fix
    {
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
        return;
    }

	if ( cDelimiter == 'E' )
	{
	    *(char*)lineEnd = savedLineEnd;
	    sendSyntaxError(pController, message);
		return;
	}

	// Print warning if transaction ID is not in sequence
//	static int  oldTransaction = -1;
//
//	if ( ++oldTransaction != iTransaction )
//	{
//		if ( mbDisplayEnabled )
//		{
//			SPrintf( "Transaction ID was %d, now %d"NL,
//				oldTransaction - 1, iTransaction );
//		}
//
//		oldTransaction = iTransaction;
//	}

	//----------------------------------------
	// Prepare message header
	//

	const char*  pcResponseType = 0;

	switch ( msgType )
	{
		case GET:
		pcResponseType = "GETRESPONSE";
		break;

		case SET:
		pcResponseType = "SETRESPONSE";
		break;

		case ACTION:
		pcResponseType = "ACTIONACK";
		break;

		default:
		break;
	}

	strcpy( result, "[" NL "MESSAGE: TYPE=" );
	strcat( result, pcResponseType );
	strcat( result, NL "TRANSACTION: ID=" );
	sprintf( tempBuffer, "%d"NL, iTransaction );
	strcat( result, tempBuffer );
	result_ptr = result + strlen(result);

	// Restore end of line
	cp = lineEnd + 1;
	*(char*)lineEnd = savedLineEnd;

	//----------------------------------------
	// Start processing regular attributes
	while ( *cp != 0 )
	{
		akGetLine( cp, lineBegin, lineEnd );
		savedLineEnd = *lineEnd;
		*(char*)lineEnd = 0;
		cp = lineBegin;
		// Get attribute name
		cDelimiter = akGetWord( cp, word );

		// See if we are at the end of the message
		if ( cDelimiter == ' ' && word == "]" )
		{
			break;
		}

		// If the word does not end with a ':', it's an error
		if ( cDelimiter != ':' )  // Fix Defect 10782
		{
		    *(char*)lineEnd = savedLineEnd;
		    sendSyntaxError(pController, message);
			return;
		}

		const char*  pcAttribute = word.c_str();

		// If attribute not found.  Skip to next attribute
		if ( registry.find( pcAttribute ) == registry.end() )
		{

			AkAttrProcessor*  attrProc = new AkAttrProcessor;
		        char  *pcDefaultResponse = newCharArray( pcName, STRING_LIMIT );

		        // Default response

		         sprintf( pcDefaultResponse,
			    "ERRORIND: ATTR=%s ERROR=\"ATTRIBUTE UNKNOWN\" INFO=\"ATTRIBUTE UNKNOWN\"",
			    pcAttribute );

			delete attrProc;

			if ( pcDefaultResponse != 0 )
			{
				if( MSG_BUFFER_LIMIT > ( strlen( pcDefaultResponse ) + strlen( result )))
				{
					strcat( result, pcDefaultResponse );
					strcat( result, NL );
				}
				else
				{
					sprintf( result_ptr,"ERRORIND: ATTR=%s ERROR=\"MESSAGE TOO LARGE\" INFO=\"MESSAGE TOO LARGE\""NL,pcAttribute);
				}
				deleteCharArray( pcName, pcDefaultResponse );
			}

			// Restore end of line
			cp = lineEnd + 1;
			*(char*)lineEnd = savedLineEnd;

			continue;
		}
		else
		{
			// Attribute known
			// Process attribute
			char*  pcResponse = 0;

			// Record only the first occurrence of a heartbeat message to the Flight Recorder
			bool  bIsHeartbeatMsg = false;

			if ( ( 0 == strcmp( pcAttribute, "TEMPERATURE" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetTemperatureHasBeenRecorded )
				{
					bIsHeartbeatMsg = true;
				}
				else
				{
					mbGetTemperatureHasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "TIMESTAMP" ) ) &&
					( SET == msgType ) )
			{
				if ( mbSetTimestampHasBeenRecorded )
				{
					bIsHeartbeatMsg = true;
				}
				else
				{
					mbSetTimestampHasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "UNITSTATE" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetUnitStateHasBeenRecorded )
				{
					bIsHeartbeatMsg = true;
				}
				else
				{
					mbGetUnitStateHasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "CARRIERCFG" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetCarrierCfgHasBeenRecorded )
				{
					bIsHeartbeatMsg = true;
				}
				else
				{
					mbGetCarrierCfgHasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "INTALMSTAT" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetIntAlmStatHasBeenRecorded )
				{
					bIsHeartbeatMsg = true;
				}
				else
				{
					mbGetIntAlmStatHasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "ACTIVESW" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetActiveSW_HasBeenRecorded )
				{
					bIsHeartbeatMsg = true;
				}
				else
				{
					mbGetActiveSW_HasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "INVENT3GPP" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetInvent3GPP_HasBeenRecorded )
				{
					bIsHeartbeatMsg = true;
				}
				else
				{
					mbGetInvent3GPP_HasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "MAINCOUNTERS" ) ) &&
					( SET == msgType ) )
			{
				if ( mbFirstSetMainCountersHasBeenRecorded )
				{
					// Not really a heartbeat message, but too frequent to
					// permit every occurrence clutter up the FR.
					if ( ! mpoSetMainCountersCounter->isReady() )
					{
						bIsHeartbeatMsg = true;
					}
				}
				else
				{
					mbFirstSetMainCountersHasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "CARRCOUNTERS" ) ) &&
					( GET == msgType ) )
			{
				if ( mbFirstGetCarrierCountersHasBeenRecorded )
				{
					// Not really a heartbeat message, but too frequent to
					// permit every occurrence clutter up the FR.
					if ( ! mpoGetCarrCountersCounter->isReady() )
					{
						bIsHeartbeatMsg = true;
					}
				}
				else
				{
					mbFirstGetCarrierCountersHasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "TXPOWER" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetTxPower_HasBeenRecorded )
				{
					bIsHeartbeatMsg = true;

				}
				else
				{
					mbGetTxPower_HasBeenRecorded = true;
				}
			}
			else if ( ( 0 == strcmp( pcAttribute, "TX2POWER" ) ) &&
					( GET == msgType ) )
			{
				if ( mbGetTx2Power_HasBeenRecorded )
				{
					bIsHeartbeatMsg = true;

				}
				else
				{
					mbGetTx2Power_HasBeenRecorded = true;
				}
			}

			if ( ! bIsHeartbeatMsg )
			{
				// Log in flight recorder, except for heartbeat messages
				poFR->log( "%s %s: %s"NL, strType.c_str(), pcAttribute, cp );
			}

			switch ( msgType )
			{
				case  GET:
	//			SPrintf( "%s get %s %s"NL, pcName, attribute, cp );
				pcResponse = registry[ pcAttribute ]->get( pcAttribute, cp );
	//			SPrintf( "%s size %d"NL, rp, strlen( rp ) );
				break;

				case  SET:
				pcResponse = registry[ pcAttribute ]->set( pcAttribute, cp );
				// TxPower and Tx2Power will be logged into flight record
				if ( 0 == strcmp ( pcAttribute, "CARRIERCFG" ) )
				{
					mbGetTxPower_HasBeenRecorded = false;
					mbGetTx2Power_HasBeenRecorded = false;
				}
				break;

				case  ACTION:

				pcResponse = registry[ pcAttribute ]->act( pController, iTransaction,
						pcAttribute, cp );

				if ( 0 == strcmp( pcAttribute, "RESETREQ" ) && 0 == pcResponse [ 0 ] )
				{
					if ( strstr ( cp, "SESSION" ) )
					{
						isSessionReset = true;
					}
					else if ( strstr( cp, "SOFT" ) )
					{
	;					isSoftReset = true;
					}
				}

				break;

				default:
				if ( mbDisplayEnabled )
				{
					SPrintf( "AkMsgProcessor: Invalid type %ld"NL, msgType );
				}
				break;
			}
			if ( pcResponse[ 0 ] != 0 )
			{
				if( MSG_BUFFER_LIMIT > ( strlen( pcResponse ) + strlen( result )))
				{
					strcat( result, pcResponse );
					strcat( result, NL );
				}
				else
				{
					sprintf( result_ptr,"ERRORIND: ATTR=%s ERROR=\"MESSAGE TOO LARGE\" INFO=\"MESSAGE TOO LARGE\""NL,pcAttribute);
				}
			}
	//		else
	//		{
	//			if ( mbDisplayEnabled )
	//			{
	//				SPrintf( "Skipping result"NL );
	//			}
	//		}

			if ( ! bIsHeartbeatMsg )
			{
				// Log in flight recorder, except for heartbeat messages
				if ( pcResponse[ 0 ] != 0 )
				{
					poFR->log( "%s %s"NL, pcResponseType, pcResponse );
				}
				else
				{
					poFR->log( "%s (%s)"NL, pcResponseType, pcAttribute );
				}
			}

			// Memory pointed by rp is allocated in get/set/act functions above.
			// Must be deleted so no memory leak
	 		deleteCharArray( pcName, pcResponse );

			// Restore end of line
			cp = lineEnd + 1;
			*(char*)lineEnd = savedLineEnd;

		} // end of process attribute known
	}

	if ( word != "]" )
	{
	    sendSyntaxError(pController, message);
		return;
	}

	// Add end of response message and send it
	strcat( result, "]"NL );

#   ifndef HARDCODED_TEST
	// Send the result to the controller

//	SPrintf( "AkMsgProcessor:: sending message (%d):" NL "%s",
//			strlen( result ), result );

	if ( mbDisplayEnabled )
	{
		SPrintf( NL"%s"NL, result );
	}
	pController->sendMessage( result );

	// After response message is sent out, close a session

	if ( isSoftReset || isSessionReset )
	{
		dropConnections( );
	}

	llFinishTime_ms = OwSystemTime::get_ms();

	llElapsedTime_ms = llFinishTime_ms - llStartTime_ms;

	if ( llElapsedTime_ms > 18000 )
	{
		SPrintf( "*****"NL"%s - Elapsed Time %llu"NL"*****"NL, pcName, llElapsedTime_ms );
	}

#   else
	// When debugging, just print, don't send to controller
	printf( "%s"NL, result );
#   endif

} // AkMsgProcessor::processOneMessage()

//=============================================================================
// dropConnections

void
AkMsgProcessor::dropConnections
(
)
{
        //clear all sessions
        set<AkController*>::iterator  ptrController;
        ptrController = controllers.begin( );

        while ( ptrController != controllers.end() )
        {
                ( (AkController*) (*ptrController) )->disconnectSocket( );
                //SPrintf ("AkMsgProcessor:Close one session\n");
                ptrController++;
        }
        // last protection
        if ( !controllers.empty() )
        {
                SPrintf ("AkMsgProcessor: Delete all controllers \n");
                controllers.clear();
        }
}  // dropConnections

//=============================================================================
// setController

AkMsgProcessor::Status
AkMsgProcessor::setController
(
	AkController*  controller
)
{
	if ( controllers.find( controller ) == controllers.end() )
	{
		// Not found
		controllers.insert( controller );

		return  SUCCESS;
	}
	return  REDUNDANT;

} // AkMsgProcessor::setController()


//=============================================================================
// unsetController

AkMsgProcessor::Status
AkMsgProcessor::unsetController
(
	AkController*  controller
)
{
	const char  *pcName = "AkMsgProcessor::unsetController";
	AkMsgProcessor::Status  status;
	set<AkController*>::iterator  it = controllers.find( controller );

	if ( it != controllers.end() )
	{
		controllers.erase( controller );
		status = SUCCESS;
	}
	else
	{
		SPrintf( "%s - Cannot find the entry in the set."NL, pcName );
		status = FAILURE;
	}

	return  status;

} // AkMsgProcessor::unsetController()


//=============================================================================
// handleAttribute

AkMsgProcessor::Status
AkMsgProcessor::handleAttribute
(
	AkAttrProcessor*  attrProc,
	const char*  attribute
)
{
	bool exist = (registry.find( attribute ) != registry.end() );

	if ( exist && registry[ attribute ] != attrProc )
	{
		SPrintf( "Overriding attribute %s"NL, attribute );
	}
	registry[ attribute ] = attrProc;

	return  exist ? OVERRIDE : SUCCESS;

} // AkMsgProcessor::handleAttribute()


//=============================================================================
// event

void
AkMsgProcessor::event
(
	const char*  pcAttribute,
	char*  pcEvent
)
{
	const char  *pcName = "AkMsgProcessor::event";
	// For measurement events, only print a trace message every TRACE_MEASURE
	// events.  Otherwise, the screen will be swamped.

	bool  bTraceEvent = false;
	static unsigned int  traceTXPOWER = 0;
	static unsigned int  traceRTWP = 0;
	static unsigned int  traceFILRXPWR = 0;

	ostringstream  ss;

	ss << "["NL;
	ss << "MESSAGE: TYPE=EVENT"NL;
	ss << "TRANSACTION: ID=-2"NL;

	if ( pcEvent[ 0 ] != 0 )
	{
	   ss << pcEvent << NL;
	}

	if ( AkTtlna::getInstance()->AMR_IsEnabled() )
	{
		// Add EXTREGSTAT info
		char*  pcRegStatus =  AkAlarm::getInstance()->getEXTREGSTAT( 0, 0 );
		ss << pcRegStatus << NL;
		deleteCharArray( pcName, pcRegStatus );
	}
	ss << "]"NL;

	// These are measurement events, which do not count toward
	// alarm events when determining when to raise an event throttle alarm.
	// add ALSTAT, this is to notify ALD state change, should not be counted to
	// raise an event throttle alarm
	if ( strcmp( pcAttribute, "TXPOWER" ) == 0 )
	{
		bTraceEvent = ( ( traceTXPOWER++ % 300 ) == 0 );
	}
	else if ( strcmp( pcAttribute, "RTWP" ) == 0 )
	{
		bTraceEvent = ( ( traceRTWP++ % 300 ) == 0 );
	}
	else if ( strcmp( pcAttribute, "FILRXPWR" ) == 0 )
	{
		bTraceEvent = ( ( traceFILRXPWR++ % 300 ) == 0 );
	}
	else
	{
		// Regular event under throttling control
		bTraceEvent = true;
        if (AkMsgProcessor::THROTTLE_NONE == meThrottle)
        {
		    eventCounter += AK_EVENT_FILLER;
        }
	}

	if ( eventCounter < eventThreshold )
	{
		// Not too many event occurred, so send it to the controllers
		set<AkController*>::iterator  it;

		string  str = ss.str();
		const char*  cp = str.c_str();

		if ( bTraceEvent )
		{
			// We do not print every measurement event, so we don't flood
			// the screen.
			SPrintf( "AkMsgProcessor:: send event (%d):" NL "%s",
					strlen( cp ), cp );

			// Log in the flight recorder too
			FlightRecorder::getInstance()->log( "EVENT %s"NL, pcEvent );
		}

		// Send message to the controller
		for ( it = controllers.begin(); it != controllers.end(); it++ )
		{
			(*it)->sendMessage( cp );
		}
        meThrottle = AkMsgProcessor::THROTTLE_NONE;
	}
	else
	{
		// Too many events occurred in a short period of time.  Raise alarm.
        if ( AkMsgProcessor::THROTTLE_NONE == meThrottle )
        { 
		    AkAlarm::getInstance()->setMsgThrottlingAlarm( AkAlarm::RAISE );
            meThrottle = AkMsgProcessor::THROTTLING;
        }
        else if ( AkMsgProcessor::THROTTLING == meThrottle )
        {
            // send at least one Throttling alarm

		    set<AkController*>::iterator  it;

		    string  str = ss.str();
		    const char*  cp = str.c_str();

		    if ( bTraceEvent )
		    {
			    // We do not print every measurement event, so we don't flood
			    // the screen.
			    SPrintf( "AkMsgProcessor:: send event (%d):" NL "%s",
					    strlen( cp ), cp );

			    // Log in the flight recorder too
			    FlightRecorder::getInstance()->log( "EVENT %s"NL, pcEvent );
		    }

		    // Send message to the controller
		    for ( it = controllers.begin(); it != controllers.end(); it++ )
		    {
			    (*it)->sendMessage( cp );
		    }

            meThrottle = AkMsgProcessor::THROTTLED;
        }
	}

	deleteCharArray( pcName, pcEvent );

} // AkMsgProcessor::event()


//=============================================================================
// actComplete

void
AkMsgProcessor::actComplete
(
	AkController  *pController,
	unsigned int  transaction,
	const char*  pcAttribute,
	char*  pcResult
)
{
	const char  *pcName = "AkMsgProcessor::actComplete";
	ostringstream  ss;

	ss << "["NL;
	ss << "MESSAGE: TYPE=ACTIONCOMP"NL;
	ss << "TRANSACTION: ID=" << transaction << NL;

	if ( pcResult[ 0 ] != 0 )
	{
		ss << pcResult << NL;
	}

	ss << "]"NL;

	string  str = ss.str();
	const char*  cp = str.c_str();

	SPrintf( "AkMsgProcessor:: send action complete (%d):\n%s",
			strlen( cp ), cp );

	pController->sendMessage( cp );

	// Log to flight recorder
	FlightRecorder::getInstance()->log( "ACTIONCOMP %s"NL, pcResult );

	deleteCharArray( pcName, pcResult );

} // AkMsgProcessor::actComplete()


//=============================================================================
// task

void
AkMsgProcessor::task
(
)
{
	if ( eventCounter == eventThreshold )
	{
		// Clear throttling alarm
		AkAlarm::getInstance()->setMsgThrottlingAlarm( AkAlarm::CLEAR );
        // meThrottle = AkMsgProcessor::THROTTLE_CLEAR;
	}

	if ( eventCounter )
	{
		eventCounter--;
	}

} // AkMsgProcessor::task()

//=============================================================================

int
AkMsgProcessor::process_akgets32number
(
	ArgvType&  argv
)
{
	int  iValue;
	const char*  pcLine = argv[ 1 ];
	char  cDelimiter = akGetSigned32BitNumber( pcLine, iValue );

	if ( 'E' == cDelimiter )
	{
		clipPrintf( "Conversion error"NL );

		return  ( ERROR );
	}
	clipPrintf( "%s => %ld"NL, argv[ 1 ], iValue );

    return  ( OK );

} // AkMsgProcessor::process_akgets32number()


//=============================================================================

int
AkMsgProcessor::process_akgetu32number
(
	ArgvType&  argv
)
{
	unsigned int  ulValue;
	const char*  pcLine = argv[ 1 ];
	char  cDelimiter = akGetUnsigned32BitNumber( pcLine, ulValue );

	if ( 'E' == cDelimiter )
	{
		clipPrintf( "Conversion error"NL );

		return  ( ERROR );
	}
	clipPrintf( "%s => %lu"NL, argv[ 1 ], ulValue );

    return  ( OK );

} // AkMsgProcessor::process_akgetu32number()


//=============================================================================

int
AkMsgProcessor::process_akgetu64number
(
	ArgvType&  argv
)
{
	ULLONG  ullValue;
	const char*  pcLine = argv[ 1 ];
	char  cDelimiter = akGetUnsigned64BitNumber( pcLine, ullValue );

	if ( 'E' == cDelimiter )
	{
		clipPrintf( "Conversion error"NL );

		return  ( ERROR );
	}
	clipPrintf( "%s => %llu"NL, argv[ 1 ], ullValue );

    return  ( OK );

} // AkMsgProcessor::process_akgetu64number()


//=============================================================================
int
AkMsgProcessor::process_console
(
	ArgvType&  argv
)
{

	unsigned int  argc = argv.size();

	if ( argc == 1 )
	{
		if(0 != strlen(mcTermName))
			clipPrintf( "Current terminal ID : %s"NL, mcTermName );

		return( OK );
	}
	else if ( argc == 2 )
	{
		if ( 0 == sscanf( argv[ 1 ], "%s", mcTermName ) )
		{
			clipPrintf( "Invalid terminal name argument"NL );

			return ( ERROR );
		}
		SPrintf("Redirect RCS to teminal %s"NL,mcTermName);
		mgpts =  open(mcTermName,O_RDWR);
		if(-1 != mgpts)
		{
		    CClipTask::getInstance()->deactivate(); //disable clip mode to see all clipPrintf message 
		    dup2(mgpts,STDOUT_FILENO);
		    dup2(mgpts,STDERR_FILENO);
		}
		else
		{
		    SPrintf("open %s fail !"NL,mcTermName);
		}

		return  ( OK );
	}
	else return ( ERROR );
} // AkMsgProcessor::process_console()


//=============================================================================

int
AkMsgProcessor::process_ard
(
	ArgvType&  argv
)
{
	mbDisplayEnabled = ! mbDisplayEnabled;

    return  ( OK );

} // AkMsgProcessor::process_ard()


//=============================================================================

int
AkMsgProcessor::process_disable_ard_msg
(
	ArgvType&  argv
)
{
	mbDisplayEnabled = false;

    return  ( OK );

} // AkMsgProcessor::process_disable_ard_msg()


//=============================================================================

int
AkMsgProcessor::process_enable_ard_msg
(
	ArgvType&  argv
)
{
	mbDisplayEnabled = true;

    return  ( OK );

} // AkMsgProcessor::process_enable_ard_msg()


//=============================================================================

int
AkMsgProcessor::process_set_evt_threshold
(
	ArgvType&  argv
)
{
	if ( argv.size() < 2 )
	{
		clipPrintf( "%s"NL, MSG_INVALID_NUM_ARGS );

		return  ( ERROR );
	}

	unsigned int  thres;

	if ( 0 == sscanf( argv[ 1 ], "%d", &thres ) )
	{
		clipPrintf( "%s - threshold"NL, MSG_INVALID_ARG );

		return  ( ERROR );
	}
    eventThreshold = thres;

    return OK;

} // AkMsgProcessor::process_set_evt_threshold()


//=============================================================================
