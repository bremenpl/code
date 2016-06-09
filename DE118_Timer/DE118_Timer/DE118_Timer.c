/*
 * DE118_Timer.c
 *
 * Created: 2016-06-08 21:30:09
 *  Author: Lukasz
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <stdint.h>

// Pins:
#define BTN1_PORT		PORTC
#define BTN1_DDR		DDRC
#define BTN1_PIN		PINC
#define BTN1_P			5
#define BTN1_PCINT		PCINT13

#define BTN2_PORT		PORTC
#define BTN2_DDR		DDRC
#define BTN2_PIN		PINC
#define BTN2_P			3
#define BTN2_PCINT		PCINT11

#define BTN3_PORT		PORTC
#define BTN3_DDR		DDRC
#define BTN3_PIN		PINC
#define BTN3_P			4
#define BTN3_PCINT		PCINT12

#define NR_OF_ROWS		4
#define NR_OF_DIGITS	10

// Signs or masks
// For pinout check the schematics in DE118_Timer project

// Colon
#define COLON_OR_MASK	((uint32_t)0x00000100)

// Digits
static const uint32_t digits[NR_OF_ROWS][NR_OF_DIGITS] = 
{
//	  0		    1		 2		  3		   4		5		 6		  7		   8	    9
/*1*/ {0x1CE000,0x048000,0xC01A00,0xE0C000,0x06A000,0xE06000,0x1E6000,0x04C000,0x1EE000,0x0EE000},
/*2*/ {0x280000,0xCC1000},
/*3*/ {},
/*4*/ {}
};

typedef union
{
	uint32_t		u32;
	uint8_t			u8[4];
} union32_t;

/*
	1 second interrupts are needed in ctc mode.
	Y_freq = 32768 Hz,
	to get 1 Hz from equation F_tim = Y_freq / (2 * presc * (1 + OCR).
	F_tim = 32768 / (2 * 1024 * (1 + 31)
*/
void OneSecTim2_Init()
{
	// Output Compare Register2 Update Busy and cativate tosc crystal
	ASSR = _BV(OCR2AUB) | _BV(AS2);
	
	// Reset counter
	TCNT2 = 0;
	
	// Set period
	OCR2A = 31;
	
	// set CTC mode
	TCCR2A = _BV(WGM21);
	
	// prescaller /1024
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
	
	// clear int flag
	TIFR2 |= _BV(OCF2A); 
	
	// enable interrupts on CTC2
	TIMSK2 = _BV(OCIE2A);
}

/*
	@brief initializes external interrupts on buttons 1, 2 and 3
*/
void Buttons_Init()
{
	// Set button pins as inputs with internal pull-ups
	BTN1_DDR &= ~_BV(BTN1_P);
	BTN1_PORT |= _BV(BTN1_P);
	
	BTN2_DDR &= ~_BV(BTN2_P);
	BTN2_PORT |= _BV(BTN2_P);
	
	BTN3_DDR &= ~_BV(BTN3_P);
	BTN3_PORT |= _BV(BTN3_P);	

	// Enable interrupts on PCINT pins 8-14
	PCICR = _BV(PCIE1);
	PCMSK1 = _BV(BTN1_PCINT) | _BV(BTN2_PCINT) | _BV(BTN3_PCINT);

	
}

/*
	@brief	Update time in this interrupt routine
*/
ISR(TIMER2_COMPA_vect)
{
	PINC |= _BV(0);
}

ISR(PCINT1_vect)
{
	asm("NOP");
}

int main(void)
{
	DDRC |= _BV(0);
	PORTC |= _BV(0);
	
	// Initialize 1 sec timer
	OneSecTim2_Init();
	
	// Initialize buttons
	Buttons_Init();

	// enable globa interrupts
	sei();

    while(1)
    {
		asm("NOP");
		_delay_ms(1000);
		//PINC |= _BV(0);
    }
}