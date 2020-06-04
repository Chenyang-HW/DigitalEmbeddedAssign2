/* EEEN40280 Digital and Embedded Systems

	Generates a square wave on P3.6 with adjustable frequency.
	Switches on P2.2 to P2.0 set the frequency, by setting the 
	reload value for timer 2 according to values in a look-up table.
	Outputs frequency number on LEDs on port 0, using one-hot code.
	In parallel, generates a slow square wave on P3.4 to flash an
	LED - this uses a software delay, independent of interrupts.
	Flashing can be turned off or on by pressing the INT0 button.

	Main program configures timer and then loops, checking switches,
	setting reload value, updating LEDs on port 0,
	then checking INT0 and flashing the LED if required.
	Timer 2 ISR changes state of ouptut on P3.6 and clears flag.

	Frequency calculation, based on clock frequency 11.0592 MHz:
	For 200 Hz output, want interrupts at 400 Hz, or every 
	27648 clock cycles.  So set timer 2 reload value to 
	(65536 - 27648) = 37888. 
	*/

#include <ADUC841.H>	// definitions of SFRs and bits

// define some useful variable types
typedef unsigned char uint8;				// 8-bit unsigned integer
typedef unsigned short int uint16;	// 16-bit unsigned integer

#define OUT_PIN		WR		// output on Port 3 pin 6 (WR in header file)
#define LED_PIN		T0		// LED on P3.4 (called T0 in header file)
#define LED_BANK	P0		// LEDs are connected to Port 0
#define SWITCHES	P2		// switches are connected to Port 2
#define FREQ_MASK	0x07	// mask to select lowest 3 bits (freq set)
#define LOW_BYTE	0x00FF	// mask to select low byte of 16-bit word


/*------------------------------------------------
Interrupt service routine for timer 2 interrupt.
Called by the hardware when the interrupt occurs.
------------------------------------------------*/
void timer2 (void) interrupt 5	// interrupt vector at 002BH
{
	OUT_PIN = ~OUT_PIN;			// invert the output pin
	TF2 = 0;					// clear interrupt flag
}	// end timer2 interrupt service routine


/*------------------------------------------------
Software delay function - argument is number of loops.
------------------------------------------------*/
void delay (uint16 delayVal)
{
	uint16 i;                 // counting variable 
	for (i = 0; i < delayVal; i++)    // repeat  
	{
		// do nothing
	}
}	// end delay


/*------------------------------------------------
The main C function.  Program execution starts
here after stack initialization.
------------------------------------------------*/
void main (void) 
{
	uint8 temp;		// temporary variable - 8 bits
	uint8 freqNum;	// frequency number
	uint16 reloadVal;	// reload value for timer 2
	bit flash = 1;		// bit variable to enable flashing
	bit lastButton = 0;	// bit variable for last button state
	
	// reload values for 200, 300, 400, 600, 800, 1600, 2400, 3600 Hz
	code uint16 lookUp[8] = { 37888, 47104, 51712, 56320, 58624, 
								62080, 63232, 64000 };
/*	Note: Standard C would use "const uint16 lookUp[8] ---"
	The "const" qualifier specifies that the array is constant, 
	 and the compiler should not allow the program to change it.  
	 This compiler would store the array in data memory (like a 
	 normal array), and create code to copy the initial values 
	 from program memory to data memory at startup.
	 The "code" qualifier is an extension to C, similar to const, 
	 but telling the compiler to store the array in program memory. */

	// Set switch port for use as input
	SWITCHES = 0xFF;		// output 1 to allow pins to be used as inputs
	/* Note: The port registers contain 0xFF after reset, but the 
	startup code changes Port 2 to 0x00 in "compact" mode. */

	// Set up timer 2 in timer mode, auto reload, no external control
	T2CON = 0x04;	// all zero except run control
	ET2 = 1;		// enable timer 2 interrupt
	EA = 1;			// enable interrupts in general

	// After setting up, main goes into an infinite loop
	while (1)		// loop forever, repeating tasks
	{
		// Get required frequency number (range 0 to 7)
		temp = SWITCHES;		// read switch values
		freqNum = temp & FREQ_MASK;		// frequency number from 3 LSBs

		// Use frequency number to set reload value (hence frequency)
		reloadVal = lookUp[freqNum];	// get reload value from table
		RCAP2L = reloadVal & LOW_BYTE;	// put low byte in reload register
		RCAP2H = reloadVal >> 8;		// put high byte in reload register
		/* Alternative: define a 16-bit sfr (not done in header file)
		and load the 16-bit value directly:
		sfr16	RCAP2	= 0xCA;		// use address of RCAP2L
		- - 
		RCAP2 = reloadVal;		// put 16-bit reload value in reload register */

		// Use frequency number to set pattern on port 0
		temp = 1;			// single 1 bit (note re-use of variable)
		temp <<= freqNum;	// shift left according to frequency number
		LED_BANK = ~temp;	// output complement to suit LED wiring

		/* Check for INT0 button press, and update flashing state.
		Button will bounce, but as it is only checked at long intervals,
		due to the delay below, this bounce will have no effect.  */
		if (lastButton & ~INT0)		// button has just been pressed
		{
			flash = ~flash;		// change flashing state
		}
		lastButton = INT0;		// update last button state
		
		// Flash if enabled
		if (flash) LED_PIN = ~LED_PIN;	// Change state of LED
		else LED_PIN = 1;	// otherwise turn LED off

		// Delay
		delay (60000);		// approx. 87 ms software delay

	} // end while(1)

}	// end main
