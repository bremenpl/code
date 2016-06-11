/*
 * DE118_Timer.c
 *
 * Created: 2016-06-08 21:30:09
 *  Author: Lukasz
 *
 * Project for Atmega88pa chip
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "u_tables.h"
#include "u_hardware.h"

//volatile uint8_t* testpin = &PINC;

int main(void)
{
	// disable global interrupts
	cli();
	
	// Initialize 1 sec timer
	OneSecTim2_Init();
	
	// System timer init, for display refreshing and events handling
	SystemTim0_Init();
	
	// Initialize buttons
	Buttons_Init();
	
	// Initialize buzzer
	Buzzer_Init();

	// enable globa interrupts
	sei();

    while(1)
    {
		asm("NOP");
		_delay_ms(1000);
		//PINC |= _BV(0);
    }
}













