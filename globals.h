/*
 * global.h
 * This file contains the definitions of items needing to be used by
 * multiple functions. 
*/
#include <stdlib.h>
#include <stdio.h>
#include <pic16f1823.h>

// Using Internal Clock of 32 Mhz
	#define FOSC 32000000L
	
	extern volatile unsigned int gbl_ms_Timer;	// Used when Pre-Scaler value of '0b100' equals 1:32.  With a FOCS of
												// 32 mHz, the Timer0 Flag will be set every 1.024 mSec.
												// 1.024 = (1/(32M/4))* ( 256: TIMER0 is 8 bit) * (Pre-Scale: 32)
	
	extern volatile bit ms_Timer_flag;	// mSec Timer flag used by the timer 0 interrupt to detirmine
										// which global timer gets update
	
	//  This defines the power on values for Vout and Frequency (see utility.c)
	extern volatile unsigned char Vout_ASCII_Hi;	// Vout for User Interface (Upper Char)
	extern volatile unsigned char Vout_ASCII_Lo;	// Vout for User Interface (Lower Char)

	extern volatile unsigned char CLOCK_FREQUENCY;		// Clock frequency
	extern volatile unsigned char CLK_ON_TIME; 			// Clock On time used to sink Data w/Clock

// Serial Interface defs
	#define SER_BUFFER_SIZE		16	// Transmit Buffer Size
	extern volatile unsigned char rxfifo[4];					// Receive Buffer
	extern volatile bank1 unsigned char txfifo[SER_BUFFER_SIZE];// Transmit Buffer
	extern volatile int index;									// Receiver 'rxfifo' index
	
	void SendData(void); 		// Transmit data function
	void Execute(void);			// Execute Received request
	extern volatile unsigned char Error; 	// Status bit 0 = Pass; 1 = Error, 2 = Info (see ser.c)

// Functions	
	void set_data(void);	// Write Data to Device (see set_data.c)
	void set_vout(void);	// Set Vout 1k pull-up voltage (see utility.c)
	void set_freq(void);	// Set Clock/Data out frquency (see utility.c)
	

