//=============================================================================
///
/// @file  AkMsgProcessor.h
///
/// Copyright (c) 2009 MTI Laboratory, Inc. USA
///
/// @brief  This file contains the declarations for AkMsgProcessor class
///
//=============================================================================

#ifndef AGENT_AKMSGPROCESSOR
#define AGENT_AKMSGPROCESSOR

//=============================================================================

#include  <queue>
#include  <map>
#include  <set>
#include  <string>

#include  "OwErrand.h"
#include  "AkAttrProcessor.h"
#include  "AkController.h"
#include  "AkParser.h"
#include  "Command.h"
#include  "LoopCounter.h"
#include  "AkConstants.h"

using namespace std;

//=============================================================================

//expend the Message buffer size to 5000 in ARD 4.2
#define  MSG_BUFFER_LIMIT		( 5000 ) 


#define  AK_MSG_BUFFER_SIZE	MSG_BUFFER_LIMIT
#define  TERMNAME_LENGTH 	( 80 )


//=============================================================================

class AkMsgBuffer
{
public:

	enum  eBufferState
	{
		eNoMsg,
		ePartialMsg,
		eCompleteMsg,
		eOverSizeMsg
	};

					AkMsgBuffer();
	unsigned int	getMsgCount( void ) const;
			char*	getMessage( void );

	eBufferState	processBuffer(
						const char*  pcBuffer,
						unsigned int  length );

	//-----------------------------------------------------
private:
	eBufferState	mState;
	unsigned int	muiInBuffPos;
	unsigned int	muiMsgBuffPos;
			char*	mpcMessage;
	vector<char*>	mVectorMessages;
};

//=============================================================================
/// AkMsgProcessor receives messages from AkControllers.  The messages
/// may be concatenated.  The object breaks it into individual
/// messages.  In each message, the object checks and manage its
/// header information.  If they are good, AkMsgProcessor further
/// breaks down the message into attributes, and forward the
/// attributes to its respective AkAttrProcessor object.
///
/// The object also receives respond messages and event messages from
/// AkAttrProcessors and forward to appropriate AkController objects.

class AkMsgProcessor:
	public  OwErrand,
	public CommandSubscriber<AkMsgProcessor>
{
public:

	typedef  map<AkController*, AkMsgBuffer*>	mapControllerBuffer;
	/// Return status for member function

	enum Status
	{
		SUCCESS,	// Successful
		FAILURE,	// Failed
		REDUNDANT,	// Attempt to register something already registered
		OVERRIDE	// Previous setting has been overriden
	};

	enum  eThrottleState
	{
		THROTTLE_NONE,	//
		THROTTLING,		//
		THROTTLED,		//
		THROTTLE_CLEAR	//
	};

	/// There is only one instance of the object.  This function gets it.

	static AkMsgProcessor*	getInstance()
	{
		if ( 0 == spInstance )
		{
			spInstance = new AkMsgProcessor();
		}
		return  spInstance;
	};

	//-----------------------------------------------------
	/// Process some messages.  TCP may concatenate the messages together.
	/// This function breaks them up and use processOneMessage to process
	/// them one by one.

	void	process(
				AkController*  controller,	//[in] Controller sending the messages
				char*  message,				//[in] Points to the messages
				unsigned int  len );		//[in] # of bytes received

	//-----------------------------------------------------
	/// This function registers an AkController
	/// @retval SUCCESS Registration is successful
	/// @retval REDUNDANT The controller has already been registered

	Status	setController(
				AkController*  controller );	//[in] Controller to be registered

	//-----------------------------------------------------
	/// Unregister an AkController
	///
	/// @retval SUCCESS The controller has been removed from the registry
	/// @retval FAILURE The controller does not exist

	Status	unsetController(
				AkController*  controller );	//[in] Controller to be removed

	//-----------------------------------------------------
	/// For AkAttrProcessor to register attribute
	///
	/// @retval SUCCESS The attribute is registered successfully
	/// @retval OVERRIDE A previous entry is overriden by this one

	Status	handleAttribute(
				AkAttrProcessor*  attrProc,	//[in] Object who wants to process the attr
				const char*  attribute );	//[in] The attribute

	//-----------------------------------------------------
	/// For AkAttributeProcessor to raise an event to the controller.
	/// This function will delete the memory pointed by "result".

	void	event(
				const char*  attribute,	//[in] Attribute name of the event
				char*  result );		//[in] Content of the event

	//-----------------------------------------------------
	/// This interface is to get the result of an ACTION message by
	/// AkAttrProcessor

	void	actComplete(
				AkController*  controller,	//[in] The controller initiates the ACTION
				unsigned int  transaction,	//[in] The transaction ID of ACTION msg
				const char*  attribute,		//[in] The attribute of the action
				char*  result );			//[in] The result string

	//-----------------------------------------------------
	/// This function is called by OwErrandRunner periodically

	void	task();

	//------------------------------------------------------
	//
	void    dropConnections( );

private:
	/// How often task() is run
	static const int			ERRAND_INTERVAL = 1000; // 1 sec
	static const unsigned int	COUNTER_REPORTING_PERIOD = 10;

	//-----------------------------------------------------
	/// Private constructor

			AkMsgProcessor();

	//-----------------------------------------------------
	// Construct and send syntax error message

	void	sendSyntaxError(
				AkController* pController, // [in] Controller sending message
				char *message);            // [in] Received message containing syntax errors
	  
	//-----------------------------------------------------
	/// Process just one message.  The message shall be a null
	/// terminated string

	void	processOneMessage(
				AkController*  controller,	//[in] Controller sending the messages
				char*  message );			//[in] message being processed

	//-----------------------------------------------------
	// Functions that process CLI commands begin with process_
	int		process_akgets32number( ArgvType  &argv );
	int		process_akgetu32number( ArgvType  &argv );
	int		process_akgetu64number( ArgvType  &argv );
	int		process_ard( ArgvType  &argv );
	int		process_console( ArgvType  &argv );
	int		process_disable_ard_msg( ArgvType  &argv );
	int		process_enable_ard_msg( ArgvType  &argv );
	int		process_set_evt_threshold( ArgvType  &argv );

static AkMsgProcessor*	spInstance;		// The only instance of the object
set<AkController*>		controllers;	// Registry for controller objects

	/// For AkAttrProcessor to register their ownerships of attributes
	map<string, AkAttrProcessor*> registry;

				int		eventCounter ;	// Used to throttle event messages
				int		eventThreshold;	// Throttle threshold for event messages
				bool	mbDisplayEnabled;
				bool	mbGetTemperatureHasBeenRecorded;
				bool	mbSetTimestampHasBeenRecorded;
				bool	mbGetUnitStateHasBeenRecorded;
				bool	mbGetCarrierCfgHasBeenRecorded;
				bool	mbGetIntAlmStatHasBeenRecorded;
				bool	mbGetActiveSW_HasBeenRecorded;
				bool	mbGetInvent3GPP_HasBeenRecorded;
				bool    mbGetTxPower_HasBeenRecorded;
				bool    mbGetTx2Power_HasBeenRecorded;
				bool	mbFirstSetMainCountersHasBeenRecorded;
				bool	mbFirstGetCarrierCountersHasBeenRecorded;
mapControllerBuffer		mMapMsgBuffers;
		LoopCounter*	mpoSetMainCountersCounter;
		LoopCounter*	mpoGetCarrCountersCounter;
		eThrottleState  meThrottle;
				char	mcTermName[ TERMNAME_LENGTH ];
				int		mgpts;
};

//=============================================================================

#endif

//=============================================================================
