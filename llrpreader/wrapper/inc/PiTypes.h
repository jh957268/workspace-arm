//=============================================================================
///
/// @file  PiTypes.h
/// @brief  This file contains global data types used in RRH software
///
//=============================================================================

#ifndef PITYPES_H
#define PITYPES_H

//=============================================================================

// Define reset reasons
static const int PF_R0_WATCHDOG_TIMEOUT = 0;
static const int PF_R11_CPRI_RESET = -11;

//-----------------------------------------------------------------------------

/// @enum PiStatus An enum indicating the return status of a function call
enum  PiStatus
{
    OK              = 0,    /**< successful function execution */
#ifdef TESTER_APP
    ERROR_STATUS    = 1,    /**< Generic Error code */
#else
    ERROR           = 1,    /**< Generic Error code */
#endif
    OVERWRITTEN     = 2,    /**< A registry has been overwritten by somevalue */
    UNCONFIGURED    = 3,    /**< The system or object is not configured
                              properly */
    ALARM_ACTIVE    = 4,    /**< An outstanding alarm condition prevents the
                              operation */
    NOT_READY       = 5,    /**< The hardware is not able to support the
                              operation at this time */
    OUT_OF_BOUNDS   = 6,    /**< The parmeter given exceeds certain limits */
    TIMEOUT         = 7,    /**< The operation times out */
    NOT_SUPPORTED   = 8,    /**< The function is not supported by the hardware
                              or software version */
    CORRUPTED       = 9,    /**< Data content corrupted */
    FULL            = 10,    /**< Registry full */
    FILE_SYSTEM_FULL = 11,
    EXIT            = 30,   /** Command is exit **/
    DONE            = 31,
	UNKNOWN         = 32,
    ABORT           = 33
};

//-----------------------------------------------------------------------------

enum PiUnitType
{
	RRH = 0,
	TRDU = 1,
	INVALID_UNIT_TYPE
};

//-----------------------------------------------------------------------------
typedef long long PiTime;          ///< Specifying time in milliseconds
static const PiTime PI_FOREVER = -1; ///< infinite amount of time

//-----------------------------------------------------------------------------
typedef unsigned int PiFrequency; ///< UARFCN number as specified in
                                    ///  3GPP 25.104 (deprecated, use PiFreq instead)
static const PiFrequency PI_FREQUENCY_ERROR = 1;  ///< error condition for frequency

typedef unsigned int PiFreq;		///< Frequency in kHz
static const PiFreq PI_FREQ_NULL = 0; ///< Not a valid frequency value
static const PiFreq PI_FREQ_ERROR = 1;///< error condition for frequency

//-----------------------------------------------------------------------------
typedef int PiAxcIndex;				///< Index of CPRI containers
static const PiAxcIndex PI_AXC_UNCONFIG = -1; ///< Not a valid index

//-----------------------------------------------------------------------------
typedef short PiPower;              ///< Power level in dBm. Gain (Positive)
                                    ///  or loss (negative) in dB. unit is 0.1dBm
static const PiPower PI_POWER_ERROR = -2999; ///< error condition for power

//-----------------------------------------------------------------------------
typedef short PiCurrent;             ///< Currnt reading in mA
static const PiCurrent PI_CURRENT_ERROR   = -1; ///< error condition for current
static const PiCurrent PI_CURRENT_UNKNOWN = -2; ///< current reading unknown

//-----------------------------------------------------------------------------
typedef short PiTemperature;                          ///< Temperature in celsius
static const PiTemperature PI_TEMPERATURE_ERROR = -274; ///< error condition for temperature

//-----------------------------------------------

enum eSensorIDs
{
	eSID_PA_Temperature = 0,
	eSID_28_current = 1,
	eSID_28_voltage = 2,
	eSID_3pt3_voltage = 3,
	eSID_1pt2_voltage = 4,
	eSID_5pt5_voltage = 5,
	eSID_PSM_Temperature = 6,
	eSID_ASIC_Temperature = 7,	// For RRH
	eSID_AISG_Voltage = 7,		// For TRDU
	eSID_Last = 7

};


//-----------------------------------------------------------------------------

enum eSpecialChars
{
	eChar_Hor_Tab = 0x09,
	eChar_LineFeed = 0x0A,
	eChar_CarrReturn = 0x0D,
	eChar_Escape = 0x1B,
	eChar_Space = 0x20

};


//-----------------------------------------------------------------------------
// A fixed point integer scaled by 2^14
typedef	unsigned long	Q14;

//-----------------------------------------------------------------------------

static const double  dTWENTY_LOG_2	= 6.020599913279623904;
static const double  dTEN_LOG_2		= 3.010299956639811952;
static const double  dLOG10_2		= 3.010299956639811952e-1;

//=============================================================================

#endif // RRH_DATA_TYPES_H__

//=============================================================================
