/*
 * main.c
 *
 * This is the main entry point to the code.  It contains the main
 * function definition as well as the service interrupts routines.
 * 
*/

#include <pic.h>
#include <htc.h>
#include "globals.h";

	volatile unsigned gbl_ms_Timer;	// Used when Pre-Scaler value of '0b100' equals 1:32.  With a FOCS of
									// 32 mHz, the Timer0 Flag will be set every 1.024 mSec.
									// 1.024 = (1/(32M/4))* ( 256: TIMER0 is 8 bit) * (Pre-Scale: 32)

	volatile bit ms_Timer_flag;		// mSec Timer flag used by the timer 0 interrupt to detirmine
									// which global timer gets updated.
									
	volatile int index = 0;			// Receiver 'rxfifo' index

// This is configuration words 1 and 2.  They are defined in pic16lf1823.h and
// are used to configure the chip upon power up.
__CONFIG(FCMEN_OFF & BOREN_OFF & CPD_OFF & CP_OFF & PWRTE_OFF & WDTE_OFF & FOSC_INTOSC & LVP_ON & PLLEN_ON);

// This is the interrupt service routine (isr). The transmiter recieve
// interrupt will always be active but others may be enabled or disabled
// based on the functions using them.  
void interrupt isr (void)
{
/**** This is the RX Interrupt flag ***/
	if(RCIF)
	{	
		// If an error occured, reset to clear it
		if(OERR) 
		{	
			CREN = 0;
			CREN = 1;
		}
		
		// Get the data
		rxfifo[index] = RCREG;
		
		// Transmit the data back to the user.  When using hyper terminal,
		// the user transmitting commands to this instument are not shown.
		// By transmitting back the recieved command, the user can see and 
		// verify what they typed.
		TXREG = rxfifo[index]; 
 
		// Poll the flag looking for the TXREG to become empty indicating
		// the data has been shifted to TSR.  		
		for( int poll = 0; poll < 1000 && TXIF != 1; poll++);
		
		// '?' = Help command
		if( rxfifo[0] == '?')
			Execute();
		else	// Get all other commands
			index++;
		
		// 'Set' Commands for the Voltage and Frequency
		// are 3 chars.  Writing Data is only 2 chars.	
		// If we got two chars, go to execute.  
		if(index == 2 && rxfifo[0] != 'S' )
			Execute();
		// If we got 3 chars, set Frequency go to execute.  
		if(index == 3 && rxfifo[0] == 'S' && rxfifo[1] == 'F')
			Execute();
		// If we got 4 chars, set Vout go to execute.  
		if(index == 4)
			Execute();	
	}

/**** This is the ms Timer Interrupt flag ***/
	if( TMR0IF && ms_Timer_flag)
	{	TMR0 = 0x06;
		gbl_ms_Timer++;
		TMR0IF = 0;
	}
}

// This is the starting point of this code upon power-up.  When the device
// is turned on, the registers will be configured or setup for serial 
// communication and the device inputs and outputs will be configured. After 
// the device has been initialized this function will simply execute doing nothing 
// but wait for an interrupt from the receiver.  This code does nothing unless it 
// is told to by way of a receiver interrupt.
void main(void)
{
// This following will setup everything needed for the serial interface.
// and the inputs and outputs.  If parameters are changed from these starting 
// values by functions executing within this code structure they should be reset 
// by that function before exiting the call.

	// This sets the internal clock frequency.  Much of this was done using
	// configuration word 1&2 (see __CONFIG in main)
	OSCCON = 0xF0;		// 32mHz internal clock
						// The FOSC bits in Configuration Word 1 must be
						// set to use the INTOSC source as the device
						// system clock (FOSC<2:0> = 100).
						// The IRCF bits in the OSCCON register must be
						// set to the 8 MHz HFINTOSC selection
						// (IRCF<3:0> = 1110).
						// The SPLLEN bit in the OSCCON register must be
						// set to enable the 4xPLL, or the PLLEN bit of the
						// Configuration Word 2 must be programmed to a
						// ‘1'

	// This configures special items. Right now it is being used to make
	// sure the Serial Comm pins are set.
	APFCON = 0b10000000;// ALTERNATE PIN FUNCTION CONTROL
						// RXDTSEL SDOSEL SSSEL --- T1GSEL TXCKSEL P1BSEL CCP1SEL
						// Make sure RX is on RA1.  
						// TX is on RC4

	// These configure ports that will be digital outputs and analog in...
	//	RA1 = Rx (Digital input)
	ANSELA = 0b00000000;	// — — — ANSA4 — ANSA2 ANSA1 ANSA0
							// RA1 is Rx, digital input
							// 0 = digital, 1 = analog

	TRISA = 0b00001010; 	// — — TRISA5 TRISA4 TRISA3 TRISA2 TRISA1 TRISA0
							// RA3 is always an input
							// 0 = output, 1 = input

	// SIM HAS RX AND TX BACKWARDS						
	//TRISA = 0b00011101;	// Config RX as output for sim

	// These configure ports that will be digital outputs and analog in...
	//	RC0 = Pin 10 Clock Out 	(Digital)
	//	RC1 = Pin 9 Data Out	(Digital)
	//  RC4 = Tx -> Digital out	(Digital)
	ANSELC = 0b00000000;// — — — — ANSC3 ANSC2 ANSC1 ANSC0
						// 0 = digital, 1 = analog
		
	TRISC = 0b00101100; // — — TRISC5 TRISC4 TRISC3 TRISC2 TRISC1 TRISC0
						// 0 = output, 1 = input
	
	// SIM HAS RX AND TX BACKWARDS
	//TRISC = 0b00111100; // Cofig TX as input for Sim
	
	RC0 = 1;			// Turn Clock Off (drives and NPN Transister)
	RC1 = 1;			// Turn Data Off (drives and NPN Transister)
	
	// This sets the fixed voltage reference for the DACOUT
	FVRCON = 0b10001100;// FVRCON: FIXED VOLTAGE REFERENCE CONTROL REGISTER
						// FVREN FVRRDY Reserved Reserved CDAFVR1 CDAFVR0 ADFVR1 ADFVR0
						// FVRRDY is a status flag but will always read as set
						// ADFVR1 ADFVR0 set the ref to 4x or 4.096V volts
						
	// This sets the Vout value and is calculated as follows:
	// DACCON1 (~25.7 dec) = Volts Want (3.4V) / Reference Used (4.096V) * Scale (31)
	// Vout is defined in utility.c (-13 is a trim)
	DACCON1 = (((Vout_ASCII_Hi - 0x30) * 10) + (Vout_ASCII_Lo - 0x30)) - 13;// VOLTAGE REFERENCE CONTROL REGISTER 1
																			// — — — DACR<4:0>
	
	DACCON0 = 0b11101000;	//  VOLTAGE REFERENCE CONTROL REGISTER 0
							//  DACEN DACLPS DACOE — DACPSS<1:0> — — — 
							//  Turn on DACOUT and use FVR
						
	// The following will set up the communications (TX and RX) rate.  This
	// is setting the prescale factors.  The rate is relative to the FOSC.
	SPBRG = 0x19;		// baud rate = ~19200 (19231)
						// SYNC = 0, BRGH = 0, BRG16 = 0
						// calc baud rate err ~-0.16% for 19200
						// If an error occurs it will show up as
						// a 'frame error'. 	
	
	RCSTA = 0b10110000;	// SPEN RX9 SREN CREN ADDEN FERR OERR RX9D
						// SPEN will enable the EUSART and the TX/CK pin is automatically made an 
						// output. Since the PIN maybe used for some analog function, it must be 
						// disabled by clearing the corresponding bit of the ANSEL register
						
	TXSTA = 0b00100000;	//	TRANSMIT STATUS AND CONTROL REGISTER
						//	CSRC TX9 TXEN SYNC SENDB BRGH TRMT TX9D
	
	BAUDCON = 0b0000000;// ABDOVF RCIDL — SCKP BRG16 — WUE ABDEN

	// This enables the interrupts (also contains some flags)
	INTCON = 0b11010000;// GIE PEIE TMR0IE INTE IOCIE TMR0IF INTF IOCIF
	PIE1 = 0b00100000;	// TMR1GIE ADIE RCIE TXIE SSP1IE CCP1IE TMR2IE TMR1IE

	OPTION_REG = 0b00000100;		// WPUEN INTEDG TMR0CS TMR0SE PSA PS<2:0>
									// PSA cleared to assign a prescaler
									// PS<2:0> of 0x100 sets the prescaler to 1:32
									// And the Timer mode is selected by clearing the
									// TMR0CS bit of the OPTION register (FOSC/4).
	while(1)	
	{
	  /* Do nothing but wait for a receiver interrupt */
	}

}

