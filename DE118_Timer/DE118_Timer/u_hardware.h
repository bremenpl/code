/*
 * u_hardware.h
 *
 * Created: 2016-06-10 19:49:41
 *  Author: Lukasz
 */ 


#ifndef U_HARDWARE_H_
#define U_HARDWARE_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include <stdint.h>
#include <stdbool.h>

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

#define BUTTONS_NR		3

// Struct defining a gpio. All the hardware specs has to be filled manually.
typedef struct  
{
	volatile uint8_t*	port;		
	volatile uint8_t*	ddr;
	volatile uint8_t*	pin;
	uint8_t				nr;
	
} gpio_t;

// struct defining a button
typedef struct  
{
	gpio_t				gpio;
	uint8_t				pcint;
	bool				isHigh;
	uint8_t				hystCounter;
	bool				isPressed;
} button_t;

extern button_t buttons[3];
extern gpio_t	buzzer;

// Functions prototypes
void OneSecTim2_Init();
void SystemTim0_Init();

void Buttons_Init();

void Buzzer_Init();
void BuzzOn();
void BuzzOff();
void BuzzToggle();

#endif /* U_HARDWARE_H_ */