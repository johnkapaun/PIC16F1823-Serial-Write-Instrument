/*
 * set_data.c
 *
 * This function writes the data out.  
 * 
*/

#include <pic.h>
#include "globals.h"

	// These are the ASCII Lookup Tables - CHR$  Aditional Ram = --RAM=default,+100-1ff
	unsigned char ASCII[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '\0'};
	unsigned char Data[8];	// Data Pattern Array (init to 0s by compiler)

void set_data(void)
{
	unsigned char h = 0;	// High Nible data (for loop index)
	unsigned char l = 0;	// Low Nible data (for loop index)
	
	// Look up the High Nibble
	for( h = 0; h < 16 && ASCII[h] != rxfifo[0]; h++ );
	// Look up the Low Nibble
	for( l = 0; l < 16 && ASCII[l] != rxfifo[1]; l++ );
	
	// If both are valid, set data
	if(h < 16 && l < 16)
	{
		// Set High Nibble Data
		if( (h & 0x08) == 0)
			Data[0] = 1;
		else
			Data[0] = 0;
		if( (h & 0x04) == 0)
			Data[1] = 1;
		else
			Data[1] = 0;
		if( (h & 0x02) == 0)
			Data[2] = 1;
		else
			Data[2] = 0;
		if( (h & 0x01) == 0)
			Data[3] = 1;
		else
			Data[3] = 0;

		// Set Low Nibble Data
		if( (l & 0x08) == 0)
			Data[4] = 1;
		else
			Data[4] = 0;
		if( (l & 0x04) == 0)
			Data[5] = 1;
		else
			Data[5] = 0;
		if( (l & 0x02) == 0)
			Data[6] = 1;
		else
			Data[6] = 0;
		if( (l & 0x01) == 0)
			Data[7] = 1;
		else
			Data[7] = 0;
	}		
	else 
	{ 
		Error = 1;		// Set Error
		return;
	}	
		
	// Use Timer 0 to control both the Clock and Data.  This is setup
	// to be a mSec Timer but this Serial Write being done in this
	// application is very slow.
	TMR0 = 0x06;  		// This is the gbl_ms_Timer (Timer 0) offset
	TMR0IF = 0;			// Clear overflow HW Flag
	TMR0IE = 1;			// Enable the HW register interrupt
	ms_Timer_flag = 1;	// Timer0/gbl_ms_Timer FW iterrupt will now run
	gbl_ms_Timer = 0; 	// Clear out any Previous Counts in FW
	
	// This is a little sloppy but will work
	for(h = 0; h < 8; h++)
	{
		while(gbl_ms_Timer < (CLK_ON_TIME - 2));
		RC1 = Data[h];	// Data On
		
		while(gbl_ms_Timer < (CLK_ON_TIME));
		RC0 = 0;		// Clock On
		
		while(gbl_ms_Timer < (CLK_ON_TIME * 2));	
		RC0 = 1;		// Clock Off
		
		while(gbl_ms_Timer < ((CLK_ON_TIME * 2) + 2));
		RC1 = 1;		// Data Off
		
		gbl_ms_Timer = 2; 	// Reset Timer For Loop
	}
	
	TMR0IE = 0;			// Disable the HW register interrupt
	ms_Timer_flag = 0;	// Disable Timer0/gbl_ms_Timer FW iterrupt 

	// Set Complete Data Message
	Error = 0;		// Set Pass (No Error)

}	