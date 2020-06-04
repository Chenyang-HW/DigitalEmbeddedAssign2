/*  This program demonstrates how to use interrupts and a hardware timer.
	It uses a bit variable as a flag, to allow the interrupt service routine
	and the foreground program to communicate in a safe way. */

#include <ADUC841.H>


#define PERIOD     -250           // 250 clock cycles interrupt period
	/*  At 11.0592 MHz, gives interrupt frequency 55.296 kHz, period 18 us
		Negative value will be interpreted in 2's complement.  When put in
		an 8-bit register, value is equivalent to 256 - 250.  */
		
#define NUM_INTS	11059	// number of interrupts between events
						// Gives event frequency 5 Hz, period 0.2 s

// These are global variables: static and available to all functions
unsigned int	TimerTick;	// global variable to count interrupts
bit				TimeOver;	// global variable - flag to signal event


/*------------------------------------------------
Interrupt service routine for timer 0 interrupt.
Called by the hardware when the interrupt occurs.
------------------------------------------------*/

void timer0 (void) interrupt 1		// interrupt vector at 000BH
{
	TimerTick++;					// increment interrupt counter
	if (TimerTick > NUM_INTS)	// if enough interrupts counted
	{
		TimerTick = 0;			// reset counter
		TimeOver  = 1;			// set flag - event has occurred
	}
}


/*------------------------------------------------
The main C function.  Program execution starts
here after stack initialization.
------------------------------------------------*/

void main (void) 
{
	// Set up the timer 0 interrupt
	TH0   = (unsigned char) PERIOD;		// set timer period - reload value
	TL0   = (unsigned char) PERIOD;		// set period of first cycle also
	TMOD |= 0x02;		// select mode 2
	TR0   = 1;			// start timer 0
	ET0   = 1;			// enable timer 0 interrupt
	EA    = 1;			// global interrupt enable

	// After setting up, main goes into an infinite loop
	while (1) 
	{
		while(!TimeOver);	// wait until interrupt service routine sets flag
		P3 ^= 0x10;			// change state of LED on board
		TimeOver = 0;		// reset the flag
	}

}
