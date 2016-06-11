/*
 * u_hardware.c
 *
 * Created: 2016-06-10 19:50:34
 *  Author: Lukasz
 */ 

#include "u_hardware.h"
#include <assert.h>

button_t buttons[3];
gpio_t	 buzzer;

// Functions prototypes
void SetButtonAsInput(button_t* btn);

// Functions bodies
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
 Inits system timer for lcd refreshing and handling buttons events.
 Set to CTC mode, with 50 Hz interval.
 */

void SystemTim0_Init()
{
	// Reset counter
	TCNT0 = 0;
	
	// Set period (50.08 Hz)
	OCR0A = 77;
	
	// set CTC mode
	TCCR0A = _BV(WGM01);
	
	// prescaller /1024
	TCCR0B = _BV(CS02) | _BV(CS00);
	
	// clear int flag
	TIFR0 |= _BV(OCF0A);
	
	// enable interrupts on CTC2
	TIMSK0 = _BV(OCIE0A);	
}

/*
	@brief initializes external interrupts on buttons 1, 2 and 3
*/
void Buttons_Init()
{
	uint8_t i;
	// FIll up the buttons sctructures
	
	// BTN1:
	buttons[0].gpio.port = &PORTC;
	buttons[0].gpio.ddr = &DDRC;
	buttons[0].gpio.pin = &PINC;
	buttons[0].gpio.nr = 5;
	buttons[0].pcint = PCINT13;
	
	// BTN2:
	buttons[1].gpio.port = &PORTC;
	buttons[1].gpio.ddr = &DDRC;
	buttons[1].gpio.pin = &PINC;
	buttons[1].gpio.nr = 3;
	buttons[1].pcint = PCINT11;
	
	// BTN3:
	buttons[2].gpio.port = &PORTC;
	buttons[2].gpio.ddr = &DDRC;
	buttons[2].gpio.pin = &PINC;
	buttons[2].gpio.nr = 4;
	buttons[2].pcint = PCINT12;
	
	// Enable interrupts on PCINT pins 8-14
	PCICR = _BV(PCIE1);
	
	// Common settings for buttons
	for (i = 0; i < BUTTONS_NR; i++)
	{
		// Set button pins as inputs with internal pull-ups
		SetButtonAsInput(&buttons[i]);
		
		// Activate interrupts for each button
		PCMSK1 |= _BV(buttons[i].pcint);
	}
}

void SetButtonAsInput(button_t* btn)
{
	assert(btn);
	
	// set as input
	*btn->gpio.ddr &= ~_BV(btn->gpio.nr);
	
	// set internal pullup
	*btn->gpio.port |= _BV(btn->gpio.nr);
	
	// so the state is high
	btn->isHigh = true;
	
	// and button is not pressed
	btn->isPressed = false;
	
	// hysteresis is reset
	btn->hystCounter = 0;
}

// Intitializes buzzer
void Buzzer_Init()
{
	buzzer.port = &PORTB;
	buzzer.ddr = &DDRB;
	buzzer.pin = &PINB;
	buzzer.nr = 0;
	
	// set as output
	*buzzer.ddr |= _BV(buzzer.nr);
	
	// turn off
	BuzzOff();
}

void BuzzOn()
{
	assert(buzzer.port);
	*buzzer.port |= _BV(buzzer.nr);
}
void BuzzOff()
{
	assert(buzzer.port);
	*buzzer.port &= ~_BV(buzzer.nr);
}

void BuzzToggle()
{
	assert(buzzer.pin);
	*buzzer.pin |= _BV(buzzer.nr);
}

// Buttons press interrupt routine
ISR(PCINT1_vect)
{
	uint8_t i;
	
	// check which buttons state changed
	for (i = 0; i < BUTTONS_NR; i++)
	{
		// is the state low?
		if ( bit_is_clear(*buttons[i].gpio.pin, buttons[i].gpio.nr) && buttons[i].isHigh)
		{
			buttons[i].isHigh = false;
			
			// Driver reacts only on falling edges, so trigger pressed is set here
			buttons[i].isPressed = true;
		}
		else if ( bit_is_set(*buttons[i].gpio.pin, buttons[i].gpio.nr) && !buttons[i].isHigh)
		{
			buttons[i].isHigh = true;
			
			// no action for button release
		}
	}
	
	//PINC |= _BV(0);
}

/*
	@brief	Update time in this interrupt routine
*/
ISR(TIMER2_COMPA_vect)
{
	BuzzToggle();
}

/*
	@brief	Refresh LCD (send data through SPI), parse button events
*/
ISR(TIMER0_COMPA_vect)
{
	uint8_t i;
	
	// TODO: Add spi routines
	
	// handle buttons presses
	for (i = 0; i < BUTTONS_NR; i++)
	{
		if (buttons[i].isPressed)
		{
			buttons[i].isPressed = false;
			
			// check which button was pressed
			switch (i)
			{
				case 0: // btn 1
				{
					asm("NOP");
					break;
				}
				
				case 1: // btn 2
				{
					asm("NOP");
					break;
				}
				
				case 2: // btn 3
				{
					asm("NOP");
					break;
				}
				
				default: { } // cant happen
			}
		}
	}
}



