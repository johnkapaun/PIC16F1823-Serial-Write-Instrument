/*
 * ser.c
 *
 * This contains the serial interface code.  
 * 
*/

#include <pic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"

	volatile unsigned char rxfifo[4];						// Receive Buffer
	volatile bank1 unsigned char txfifo[SER_BUFFER_SIZE];	// Transmit Buffer
	volatile unsigned char Error = 2; 						// Status bit 0 = Pass; 1 = Error, 2 = Info 

// This will write (TX) the txfifo buffer. It will handle
// the transmission. The enable is done in Main during the
// initialization.
void SendData(void)
{

// The TXIF interrupt can be enabled by setting the TXIE
// interrupt enable bit of the PIE1 register. However, the
// TXIF flag bit will be set whenever the TXREG is empty,
// regardless of the state of TXIE enable bit.

	GIE = 0; // Disable general Interrupts while sending data

	// Start by send a Line feed
	TXREG = 0x0A; 
 
	// Poll the flag looking for the TXREG to become empty indicating
	// the data has been shifted to TSR.  		
	for( int poll = 0; poll < 1000 && TXIF != 1; poll++);
	
	// Send a Carriage Return
	TXREG = 0x0D; 
	
	// Poll the flag looking for the TXREG to become empty indicating
	// the data has been shifted to TSR.  		
	for( int poll = 0; poll < 1000 && TXIF != 1; poll++);

	// Check for pass/fail
	if(Error == 0)
	{
		txfifo[0] = 2;
		txfifo[1] = 'O';
		txfifo[2] = 'K';
	}
	else if(Error == 1)
	{
		txfifo[0] = 3;
		txfifo[1] = 'E';
		txfifo[2] = 'r';
		txfifo[3] = 'r';
	}
	else
		Error = 2; // Reset	(2 = Info)	
 	
	// Send buffer based on the number of characters specified by txfifo[0]
	for( unsigned char a = 1; a < SER_BUFFER_SIZE && a <= txfifo[0]; a++)
	{
		TXREG = txfifo[a];  // The transmission of the Start bit, 
							// data bits and Stop bit sequence commences 
							// immediately following the transfer of the 
							// data to the TSR from the TXREG.
		
		// TXIF may not be valid for one instruction cycle. This will 
		// poll the flag looking for the TXREG to become empty indicating
		// the data has been shifted to TSR.  Polling 1000 times eliminates
		// a lockup condition that may occur if something 'bad' happens.		
		for( int poll = 0; poll < 1000 && TXIF != 1; poll++);
	}
	
	GIE = 1; // Re-enable general Interrupts
	
//	TXEN = 0;	// Disable the transmitter (not using interrupt)
	
}	// End SendData

// This function does the actual work.  Because functions are acted on based
// on serial communications, to speed up the proccessing of the request it is
// important to keep the received message as short a possible.  Here are the 
// available settings:
//
//		'?'  - Help, returns command info and current device settings
//		'XX' - Send Data where "XX" is the byte data written in Hex
//		'SVXX' 	- Set Vout where XX dec = 31(Vout/4.096) Limit 00 to 31
//		'SFX'	- Set Clock Frequency where X = 1 to 3 as follows:
//					1 = 1Hz, 2 = 10Hz, 3 = 100Hz
//
// These commands could be expanded if needed.
void Execute(void)
{
	GIE = 1;	// Enable Interupts
	index = 0;	// Clear Receiver 'rxfifo' index

	// Check first byte for one of the above listed commands.
	if(  rxfifo[0] == '?')	// This is a 'Help' command
	{
		// This isn't needed unless the interface is messed up
		Error = 2; // Reset	(2 = Info)

		// SW 1.0 JK
		txfifo[0] = 10;
		txfifo[1] = 0x0A;	// Additional Linefeed
		txfifo[2] = 0x0D;	// Additional Carriage Return
		txfifo[3] = 'S';
		txfifo[4] = 'W';
		txfifo[5] = '1';
		txfifo[6] = '.';
		txfifo[7] = '0';
		txfifo[8] = ' ';
		txfifo[9] = 'J';
		txfifo[10] = 'K';
		SendData();
		// Commands:
		txfifo[0] = 11;
		txfifo[1] = 0x0A;	// Additional Linefeed
		txfifo[2] = 0x0D;	// Additional Carriage Return
		txfifo[3] = 'C';
		txfifo[4] = 'o';
		txfifo[5] = 'm';
		txfifo[6] = 'm';
		txfifo[7] = 'a';
		txfifo[8] = 'n';
		txfifo[9] = 'd';
		txfifo[10] = 's';
		txfifo[11] = ':';
		SendData();
		// USE CAPS LOCK
		txfifo[0] = 13;
		txfifo[1] = 'U';
		txfifo[2] = 'S';
		txfifo[3] = 'E';
		txfifo[4] = ' ';
		txfifo[5] = 'C';
		txfifo[6] = 'A';
		txfifo[7] = 'P';
		txfifo[8] = 'S';
		txfifo[9] = ' ';
		txfifo[10] = 'L';
		txfifo[11] = 'O';
		txfifo[12] = 'C';
		txfifo[13] = 'K';
		SendData();
		//  ML:Set Data
		txfifo[0] = 13;
		txfifo[1] = 0x0A;	// Additional Linefeed
		txfifo[2] = 0x0D;	// Additional Carriage Return
		txfifo[3] = 'M';
		txfifo[4] = 'L';
		txfifo[5] = '=';
		txfifo[6] = 'S';
		txfifo[7] = 'e';
		txfifo[8] = 't';
		txfifo[9] = ' ';
		txfifo[10] = 'D';
		txfifo[11] = 'a';
		txfifo[12] = 't';
		txfifo[13] = 'a';	
		SendData();
		// M=MS Nibble-Hex
		txfifo[0] = 15;
		txfifo[1] = 'M';
		txfifo[2] = '=';
		txfifo[3] = 'M';
		txfifo[4] = 'S';
		txfifo[5] = ' ';
		txfifo[6] = 'N';
		txfifo[7] = 'i';
		txfifo[8] = 'b';
		txfifo[9] = 'b';
		txfifo[10] = 'l';
		txfifo[11] = 'e';
		txfifo[12] = '-';
		txfifo[13] = 'H';
		txfifo[14] = 'E';
		txfifo[15] = 'X';
		SendData();
		// L=LS Nibble-Hex
		txfifo[0] = 15;
		txfifo[1] = 'L';
		txfifo[2] = '=';
		txfifo[3] = 'L';
		txfifo[4] = 'S';
		txfifo[5] = ' ';
		txfifo[6] = 'N';
		txfifo[7] = 'i';
		txfifo[8] = 'b';
		txfifo[9] = 'b';
		txfifo[10] = 'l';
		txfifo[11] = 'e';
		txfifo[12] = '-';
		txfifo[13] = 'H';
		txfifo[14] = 'E';
		txfifo[15] = 'X';
		SendData();
		// SVXX=Set Vout
		txfifo[0] = 15;
		txfifo[1] = 0x0A;	// Additional Linefeed
		txfifo[2] = 0x0D;	// Additional Carriage Return
		txfifo[3] = 'S';
		txfifo[4] = 'V';
		txfifo[5] = 'X';
		txfifo[6] = 'X';
		txfifo[7] = '=';
		txfifo[8] = 'S';
		txfifo[9] = 'e';
		txfifo[10] = 't';
		txfifo[11] = ' ';
		txfifo[12] = 'V';
		txfifo[13] = 'o';
		txfifo[14] = 'u';
		txfifo[15] = 't';
		SendData();
		// XX:Dec(00-31)
		txfifo[0] = 13;
		txfifo[1] = 'X';
		txfifo[2] = 'X';
		txfifo[3] = ':';
		txfifo[4] = 'D';
		txfifo[5] = 'e';
		txfifo[6] = 'c';
		txfifo[7] = '(';
		txfifo[8] = '0';
		txfifo[9] = '0';
		txfifo[10] = '-';
		txfifo[11] = '3';
		txfifo[12] = '1';
		txfifo[13] = ')';
		SendData();
		// 31(Vout/4.096)
		txfifo[0] = 14;
		txfifo[1] = '3';
		txfifo[2] = '1';
		txfifo[3] = '(';
		txfifo[4] = 'V';
		txfifo[5] = 'o';
		txfifo[6] = 'u';
		txfifo[7] = 't';
		txfifo[8] = '/';
		txfifo[9] = '4';
		txfifo[10] = '.';
		txfifo[11] = '0';
		txfifo[12] = '9';
		txfifo[13] = '6';
		txfifo[14] = ')';
		SendData();
		// Current Vout=XX
		txfifo[0] = 15;
		txfifo[1] = 'C';
		txfifo[2] = 'u';
		txfifo[3] = 'r';
		txfifo[4] = 'r';
		txfifo[5] = 'e';
		txfifo[6] = 'n';
		txfifo[7] = 't';
		txfifo[8] = ' ';
		txfifo[9] = 'V';
		txfifo[10] = 'o';
		txfifo[11] = 'u';
		txfifo[12] = 't';
		txfifo[13] = '=';
		txfifo[14] = Vout_ASCII_Hi;
		txfifo[15] = Vout_ASCII_Lo;
		SendData();
		// SFX=Set Freq
		txfifo[0] = 14;
		txfifo[1] = 0x0A;	// Additional Linefeed
		txfifo[2] = 0x0D;	// Additional Carriage Return
		txfifo[3] = 'S';
		txfifo[4] = 'F';
		txfifo[5] = 'X';
		txfifo[6] = '=';
		txfifo[7] = 'S';
		txfifo[8] = 'e';
		txfifo[9] = 't';
		txfifo[10] = ' ';
		txfifo[11] = 'F';
		txfifo[12] = 'r';
		txfifo[13] = 'e';
		txfifo[14] = 'q';
		SendData();
		// X = 1-3 where:
		txfifo[0] = 14;
		txfifo[1] = 'X';	
		txfifo[2] = ' ';	
		txfifo[3] = '=';
		txfifo[4] = ' ';
		txfifo[5] = '1';
		txfifo[6] = '-';
		txfifo[7] = '3';
		txfifo[8] = ' ';
		txfifo[9] = 'w';
		txfifo[10] = 'h';
		txfifo[11] = 'e';
		txfifo[12] = 'r';
		txfifo[13] = 'e';
		txfifo[14] = ':';
		SendData();
		// 1 = 1 Hz
		txfifo[0] = 8;
		txfifo[1] = '1';	
		txfifo[2] = ' ';	
		txfifo[3] = '=';
		txfifo[4] = ' ';
		txfifo[5] = '1';
		txfifo[6] = ' ';
		txfifo[7] = 'H';
		txfifo[8] = 'z';
		SendData();
		// 2 = 10 Hz
		txfifo[0] = 9;
		txfifo[1] = '2';	
		txfifo[2] = ' ';	
		txfifo[3] = '=';
		txfifo[4] = ' ';
		txfifo[5] = '1';
		txfifo[6] = '0';
		txfifo[7] = ' ';
		txfifo[8] = 'H';
		txfifo[9] = 'z';
		SendData();
		// 3 = 100 Hz
		txfifo[0] = 10;
		txfifo[1] = '3';	
		txfifo[2] = ' ';	
		txfifo[3] = '=';
		txfifo[4] = ' ';
		txfifo[5] = '1';
		txfifo[6] = '0';
		txfifo[7] = '0';
		txfifo[8] = ' ';
		txfifo[9] = 'H';
		txfifo[10] = 'z';
		SendData();
		// Current Freq=X
		txfifo[0] = 14;
		txfifo[1] = 'C';	
		txfifo[2] = 'u';	
		txfifo[3] = 'r';
		txfifo[4] = 'r';
		txfifo[5] = 'e';
		txfifo[6] = 'n';
		txfifo[7] = 't';
		txfifo[8] = ' ';
		txfifo[9] = 'F';
		txfifo[10] = 'r';
		txfifo[11] = 'e';
		txfifo[12] = 'q';
		txfifo[13] = '=';
		txfifo[14] = CLOCK_FREQUENCY+0x30;	// Convert to ASCII
	}
	else if(  rxfifo[0] == 'S')	// This is a 'Set' Utility command
	{
		if(rxfifo[1] == 'V')		// Set Vout
			set_vout();				// Set Vout 1k pull-up voltage (see utility.c)
		else if(rxfifo[1] == 'F')	// Set Freq
			set_freq();				// Set Clock/Data out frquency (see utility.c)
		else	// Set Command Error
			Error = 1;		// Set Error			
	}		
	else 	// This is a 'Set Data' Command
		set_data();	// Send Data to the Device

	//  Everything returns information to the user.
	SendData();	// Send Data to user
	
	// Send another Line feed
	TXREG = 0x0A; 
 
	// Poll the flag looking for the TXREG to become empty indicating
	// the data has been shifted to TSR.  		
	for( int poll = 0; poll < 1000 && TXIF != 1; poll++);
	
	// Send a Carriage Return
	TXREG = 0x0D; 
	
	// Poll the flag looking for the TXREG to become empty indicating
	// the data has been shifted to TSR.  		
	for( int poll = 0; poll < 1000 && TXIF != 1; poll++);

}	// End Execute
