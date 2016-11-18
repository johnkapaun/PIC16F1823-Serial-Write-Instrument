/*
 * utility.c
 *
 * This contains Vout and Frequency Setting utilities.  
 * 
*/

#include <pic.h>
#include "globals.h"

//  This defines the power on values for Vout and Frequency
volatile unsigned char Vout_ASCII_Hi = 0x32;	// Vout for User Interface (Upper Char, 2X)
volatile unsigned char Vout_ASCII_Lo = 0x36;	// Vout for User Interface (Lower Char, X6)

volatile unsigned char CLOCK_FREQUENCY = 2;		// Clock frequency
volatile unsigned char CLK_ON_TIME = 50; 		// 50 mSec On and 50 off for 100 mSec (10 Hz)

// This sets the new Vout values.  DACOUT was configured in main.c to use a fixed voltage
// reference.  DACOUT was also enabled so changes will simply adjust the value being
// outputted to the 1k pull-up resisters.  With the current configuration, the vout supplied
// can be calculated as follows:
//
// DACCON1 (~25.7 dec) = Volts Want (3.4V) / Reference Used (4.096V) * Scale (31)
//
// The max value could be set to VDD (5 volts) but requires a change to FVRCON in
// main.c.
void set_vout(void)
{
	// Store current value in case of an error.  This will keep the last 
	// valid setting functioning.
	unsigned char Temp_Vout_ASCII_Hi = Vout_ASCII_Hi;
	unsigned char Temp_Vout_ASCII_Lo = Vout_ASCII_Lo;
	
	// Capture ASCII Values
	Vout_ASCII_Hi = rxfifo[2]; 	// Capture ASCII Upper Char
	Vout_ASCII_Lo = rxfifo[3];	// Capture ASCII Lower Char
	
	// Check for an error
	if( (((Vout_ASCII_Hi - 0x30) * 10) + (Vout_ASCII_Lo - 0x30)) > 31) // Range is 0 to 31 dec
	{
		Vout_ASCII_Hi = Temp_Vout_ASCII_Hi; // Reset ASCII Upper Char
		Vout_ASCII_Lo = Temp_Vout_ASCII_Lo; // Reset ASCII Lower Char
		Error = 1;		// Set Error
		return;
	}	
	
	// The fixed voltage reference has been set in main.c (FVRCON = 0b10001100)
	// and DACOUT has been turned on (DACCON0 = 0b11101000) so all that is done
	// here is updating the Value.  
						
	// This sets the Vout value and is calculated as follows:
	// DACCON1 (~25.7 dec) = Volts Want (3.4V) / Reference Used (4.096V) * Scale (31)
	// (-13 is a trim)
	DACCON1 = (((Vout_ASCII_Hi - 0x30) * 10) + (Vout_ASCII_Lo - 0x30))-13;	// VOLTAGE REFERENCE CONTROL REGISTER 1
																			// — — — DACR<4:0>
	
	// The user interface timing will be used to make sure enough time passes for
	// the change to become stable.
	
	Error = 0;		// Set Pass (No Error)
	
}

//  This sets the clock/data frequency.  Currrently this is all timed off
//  the global ms timer because the rates required are very slow.  There
//  are currently 3 supported settings as follows:
//		1 - 1Hz
//		2 - 10Hz
//		3 - 100Hz
void set_freq(void)
{
	// Store current value in case of an error.  This will keep the last 
	// valid setting functioning.
	unsigned char TEMP_CLOCK_FREQUENCY = CLOCK_FREQUENCY;
	
	CLOCK_FREQUENCY = rxfifo[2] - 0x30;	// Remove ASCII portion
	
	if(CLOCK_FREQUENCY == 1)
		CLK_ON_TIME = 500; 	// 500 mSec On and 500 off for 1000 mSec
							// cycle which is 1 Hz
	else if(CLOCK_FREQUENCY == 2)
		CLK_ON_TIME = 50; 	// 50 mSec On and 50 off for 100 mSec
							// cycle which is 10 Hz
	else if(CLOCK_FREQUENCY == 3)
		CLK_ON_TIME = 5; 	// 5 mSec On and 5 off for 10 mSec
							// cycle which is 100 Hz
	else
	{
		// Reset Clock to last valid setting
		CLOCK_FREQUENCY = TEMP_CLOCK_FREQUENCY;
		Error = 1;		// Set Error
		return;
	}	
		
	Error = 0;		// Set Pass (No Error)

}	