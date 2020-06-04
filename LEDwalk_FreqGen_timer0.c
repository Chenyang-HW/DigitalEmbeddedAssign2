/*  This program does two things at the same time:
	In the foreground, it generates a walking pattern on LEDs on Port 0.
	In parallel, a hardware timer drives an interrupt service routine 
	to generate a square-wave at an adjustable frequency.
	The frequency is set by switches on Port 2, pins 0 to 3.
	Another switch, on Port 2 pin 7, enables and disables the interrupts. 

	Frequency calculation:
	Timer 0 is configured to give interrupts every 108 clock cycles.
	At clock frequency 11.0592 MHz, this gives interrupt frequency 102.4 kHz.
	On each interrupt, an increment derived from the switches is added to 
	a counter, modulo 256.  At each counter overflow, the output pin changes state.
	At minimum increment of 1, state changes every 256 interrupts, giving 200 Hz.
	Maximum increment is 16, giving frequency 16 times higher, 3200 Hz. 
	Frequency can be set in 200 Hz steps between these values.
	*/

#include <ADUC841.H>		// include header file with definitions of SFRs etc.

// define some useful variable types
typedef unsigned char uint8;		// 8-bit unsigned integer
typedef unsigned short int uint16;	// 16-bit unsigned integer

#define PERIOD		-108	// 108 clock cycles interrupt period
#define HIGH_BYTE	0xFF00	// mask to select upper byte of 16 bits
#define LOW_BYTE	0xFF	// mask to select lowest byte
#define TOP_BIT		0x80	// mask to select bit 7
#define LOW_NIBBLE	0x0F	// mask to select lowest 4 bits
#define LED_BANK	P0		// LEDs are connected to Port 0
#define SWITCHES	P2		// switches are connected to Port 2
#define OUT_PIN		WR		// output on Port 3 bit 6 (named as WR in header file)

// Global variables: static and available to all functions
uint8 Increment;	// counter increment - passed from main to ISR


/*------------------------------------------------
Interrupt service routine for timer 0 interrupt.
Called by the hardware when the interrupt occurs.
------------------------------------------------*/
void timer0 (void) interrupt 1	// interrupt vector at 000BH
{
	static uint16 Counter = 0;	// static => retains value between interrupts
	Counter += Increment;		// advance the counter by specified increment
	if (Counter & HIGH_BYTE) 	// if 8-bit value has overflowed
	{
		OUT_PIN = ~OUT_PIN;		// invert the output pin
		Counter &= LOW_BYTE;	// counting is modulo 256
	}
}	// end timer0 interrupt service routine


/*------------------------------------------------
Software delay function - argument is number of loops.
------------------------------------------------*/
void delay (uint16 delayVal)
{
	uint16 i;		// counting variable 
	for (i = 0; i < delayVal; i++)	// repeat
	{
		// do nothing
	}
}	// end delay



/*------------------------------------------------
LED walking function.
------------------------------------------------*/
void LEDwalk (void)
{
	static uint8 pattern = 1;	// LED pattern - 8-bits
	static bit direction = 1;	// direction of walking
	
	if (direction) 
	{
		pattern <<=1;			// shift left
		if (pattern == 0x80)	// have reached the end...
			direction = 0;		  // so change direction
	}
	else 
	{
		pattern >>=1;			// shift right
		if (pattern == 0x01)	// have reached the end...
			direction = 1;			// so change direction
	}
	
	LED_BANK = ~pattern;	// output to LEDs, inverted to suit wiring
}	// end LEDwalk


/*------------------------------------------------
The main C function.  Program execution starts
here after stack initialization.
------------------------------------------------*/
void main (void) 
{
	uint8 Swit;		// switch value - 8 bits
	
	SWITCHES = 0xff;	// set switch pins as inputs

	// Set up the timer 0 interrupt
  TH0   = (unsigned char) PERIOD;	// set timer period
  TL0   = (unsigned char) PERIOD;
  TMOD |= 0x02;			// select mode 2
  TR0   = 1;			// start timer 0
  EA    = 1;			// global interrupt enable

	// After setting up, main goes into an infinite loop
	while (1)		// Loop forever, repeating tasks
	{
		Swit = SWITCHES;	// read switch values

		ET0 = ((Swit & TOP_BIT) == TOP_BIT);	//enable timer interrupt if switch is 1

		Increment = (Swit & LOW_NIBBLE) + 1;	// set increment value (used by ISR)

		LEDwalk();		// move to next LED

		delay(60000);	// waste some time

	} // end while(1)

}	// end main
