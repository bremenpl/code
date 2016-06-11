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

#include "u_tables.h"

// Pins:

// SPI

#define BUTTONS_NR		3

#define TIME_MAX_MM		99
#define TIME_MAX_SS		59

#define COLON_PRESC		70

#define HOLD_ACTION_VAL	5

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

// struct defining shift register
typedef struct  
{
	gpio_t				mosi;
	gpio_t				miso;
	gpio_t				ncs;
	gpio_t				sck;
	
	union32_t			timeReg;
	union32_t			digitVisible;
} shiftR_t;

extern button_t buttons[3];
extern gpio_t	buzzer;

// Functions prototypes
void OneSecTim2_Init();
void SystemTim0_Init();
void BtnPressTimer1_Init();
void ShiftRegisterInit();
void Shr_SendTimeRegister();
void Buttons_Init();
void Buzzer_Init();


#endif /* U_HARDWARE_H_ */