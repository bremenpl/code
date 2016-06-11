/*
 * u_hardware.c
 *
 * Created: 2016-06-10 19:50:34
 *  Author: Lukasz
 */ 

#include "u_hardware.h"
#include <assert.h>

// local globals
button_t buttons[3];
gpio_t	 buzzer;
time_t	 time;
shiftR_t shReg;
mstate_t mState;
union32_t colon;
holdAction_t holdAction;

// Functions prototypes
void OneSecTimerStart();
void OneSecTimerStop();
void OneSecTimerReset();

void BtnPressTimerStart();
void BtnPressTimerStop();

void SetButtonAsInput(button_t* btn);

void BuzzOn();
void BuzzOff();
void BuzzToggle();

void Shr_TransmissionOn();
void Shr_TransmissionOff();


// Functions bodies
/************************************************************************
	1 second interrupts are needed in ctc mode.
	Y_freq = 32768 Hz,
	to get 1 Hz from equation F_tim = Y_freq / (2 * presc * (1 + OCR).
	F_tim = 32768 / (2 * 1024 * (1 + 31)
************************************************************************/
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
	
	// reset timer
	OneSecTimerReset();
	
	// clear int flag
	TIFR2 |= _BV(OCF2A); 
	
	// enable interrupts on CTC2
	TIMSK2 = _BV(OCIE2A);
}

/************************************************************************
@brief	Turns the Timer 2 ON.
************************************************************************/
void OneSecTimerStart()
{
	// prescaller /1024
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
}

/************************************************************************
@brief	Turns the Timer 2 OFF.
************************************************************************/
void OneSecTimerStop()
{
	TCCR2B = 0;
	TCNT2 = 0;
}

/************************************************************************

************************************************************************/
void OneSecTimerReset()
{
	// First stop the timer
	OneSecTimerStop();
	
	// Now reset the data
	time.mm = 0;
	time.ss = 0;
	
	// set digits to visible
	shReg.digitVisible.u32 = 0x01010101;
}

/************************************************************************
 Inits system timer for lcd refreshing and handling buttons events.
 Set to CTC mode, with 100 Hz interval.
************************************************************************/
void SystemTim0_Init()
{
	// Reset counter
	TCNT0 = 0;
	
	// Set period (100 Hz)
	OCR0A = 38;
	
	// set CTC mode
	TCCR0A = _BV(WGM01);
	
	// prescaller /1024
	TCCR0B = _BV(CS02) | _BV(CS00);
	
	// clear int flag
	TIFR0 |= _BV(OCF0A);
	
	// enable interrupts on CTC2
	TIMSK0 = _BV(OCIE0A);	
	
	// set default machine state
	mState = e_mstate_timerInit;
	
	// set digits to visible
	shReg.digitVisible.u32 = 0x01010101;
	
	// reset holdAction
	holdAction = e_holdAction_None;
}

/************************************************************************
Initlaizes the timer 1 peripheral. It used w for counting the time when 
the user presses the button.
************************************************************************/
void BtnPressTimer1_Init()
{
	// Turn the timer off
	BtnPressTimerStop();
	
	// set time base to 2 seconds
	OCR1A = 7811;
	
	// reset interrupts and enable them
	TIFR1 = _BV(OCF1A);
	TIMSK1 = _BV(OCIE1A);
}

/************************************************************************
Starts the button timer
************************************************************************/
void BtnPressTimerStart()
{
	TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10);
	TCNT1 = 0;
}

/************************************************************************
Stops and resets the button timer
************************************************************************/
void BtnPressTimerStop()
{
	TCCR1B = 0;
}

/************************************************************************
@brief	Initializes the SPI peripheral to work with 4 595's shift registers
************************************************************************/
void ShiftRegisterInit()
{
	// Initlialize SPI pins, first define structures
	shReg.ncs.ddr = &DDRB;
	shReg.ncs.port = &PORTB;
	shReg.ncs.pin = &PINB;
	shReg.ncs.nr = 2;
	
	shReg.mosi.ddr = &DDRB;
	shReg.mosi.port = &PORTB;
	shReg.mosi.pin = &PINB;
	shReg.mosi.nr = 3;
	
	shReg.miso.ddr = &DDRB;
	shReg.miso.port = &PORTB;
	shReg.miso.pin = &PINB;
	shReg.miso.nr = 4;
	
	shReg.sck.ddr = &DDRB;
	shReg.sck.port = &PORTB;
	shReg.sck.pin = &PINB;
	shReg.sck.nr = 5;
	
	// CS as output, high when not active
	*shReg.ncs.ddr |= _BV(shReg.ncs.nr);
	*shReg.ncs.port |= _BV(shReg.ncs.nr);
	
	// MOSI as output
	*shReg.mosi.ddr |= _BV(shReg.mosi.nr);
	
	// SCK as output
	*shReg.sck.ddr |= _BV(shReg.sck.nr);
	
	// MISO will be overtaken by the spi peripheral so no need to config
	
	// Configure spi
	//     Spi enable  master     CLK/128
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
	
	// clear SPIF flag
	SPSR;
	SPSR;
	
	// reset shift register (display nothing)
	shReg.timeReg.u32 = 0;
	Shr_SendTimeRegister();
}

/************************************************************************
pulls CS low updating the storage registers in 595's
************************************************************************/
void Shr_TransmissionOn()
{
	*shReg.ncs.port &= ~_BV(shReg.ncs.nr);
}

/************************************************************************
Pulls CS high in order to update shift registers of the 595's
************************************************************************/
void Shr_TransmissionOff()
{
	*shReg.ncs.port |= _BV(shReg.ncs.nr);
}

/************************************************************************
Sends the content of timeReg to the 595's
************************************************************************/
void Shr_SendTimeRegister()
{	
	static bool refresh = false;
	uint8_t i;	
	
	// pull cs low
	Shr_TransmissionOn();
	
	for (i = 0; i < NR_OF_ROWS; i++)
	{
		if (!refresh)
			SPDR = shReg.timeReg.u8[NR_OF_ROWS - 1 - i];
		else
			SPDR = 0;
		
		// wait for end of the transmission
		while (!(SPSR & (1 << SPIF)));
	}
	
	// pull cs high to update storage registers
	Shr_TransmissionOff();
	
	// toggle refresh flag
	refresh ^= 1;
}

/************************************************************************
@brief initializes external interrupts on buttons 1, 2 and 3
************************************************************************/
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

/************************************************************************
Configures a button struct as input
************************************************************************/
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

/************************************************************************
Intitializes the buzzer
************************************************************************/
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

/************************************************************************
Buttons press interrupt routine
************************************************************************/
ISR(PCINT1_vect)
{
	uint8_t i;
	uint8_t k;
	
	// check which buttons state changed
	for (i = 0; i < BUTTONS_NR; i++)
	{
		// is the state low?
		if ( bit_is_clear(*buttons[i].gpio.pin, buttons[i].gpio.nr) && buttons[i].isHigh)
		{
			// check if other buttons are pressed. if yes, set the flag and return
			for (k = 0; k < BUTTONS_NR; k++)
			{
				if (!buttons[k].isHigh)
				{
					buttons[i].isHigh = false;
					return;
				}
			}
			
			buttons[i].isHigh = false;
			
			// Driver reacts only on falling edges, so trigger pressed is set here
			buttons[i].isPressed = true;
		}
		else if ( bit_is_set(*buttons[i].gpio.pin, buttons[i].gpio.nr) && !buttons[i].isHigh)
		{
			buttons[i].isHigh = true;
			
			// reset press timer and hold action
			BtnPressTimerStop();
			holdAction = e_holdAction_None;
		}
	}
	
	//PINC |= _BV(0);
}

/************************************************************************
@brief	Refresh LCD (send data through SPI), parse button events
************************************************************************/
ISR(TIMER0_COMPA_vect)
{
	uint8_t i;
	static uint8_t colonPresc = 0;
	
	Shr_SendTimeRegister();
	
	// handle buttons presses
	for (i = 0; i < BUTTONS_NR; i++)
	{
		if (buttons[i].isPressed)
		{
			buttons[i].isPressed = false;
			
			// check which button was pressed
			switch (i)
			{
				case 2: // btn 3
				{
					switch (mState)
					{
						case e_mstate_timerSet_mm:
						{
							time.mm++;
							
							if (time.mm > TIME_MAX_MM)
								time.mm = 0;
		
							// start the timer.
							// If button is hold, digits should decrement automatically
							BtnPressTimerStart();
							break;
						}
	
						case e_mstate_timerSet_ss:
						{
							time.ss++;
							
							if (time.ss > TIME_MAX_SS)
								time.ss = 0;
		
							// start the timer.
							// If button is hold, digits should decrement automatically
							BtnPressTimerStart();
							break;
						}
	
						case e_mstate_timerFinished:
						{
							OneSecTimerReset();
							BuzzOff();
							mState = e_mstate_timerInit;
							break;
						}
	
						default: { }
					}
					break;
				}
				
				case 0: // btn 1
				{
					switch (mState)
					{
						case e_mstate_timerSet_mm:
						{
							time.mm--;
							
							if (time.mm < 0)
								time.mm = TIME_MAX_MM;
								
							// start the timer. 
							// If button is hold, digits should decrement automatically
							BtnPressTimerStart();
							break;
						}
						
						case e_mstate_timerSet_ss:
						{
							time.ss--;
						
							if (time.ss < 0)
								time.ss = TIME_MAX_SS;
								
							// start the timer.
							// If button is hold, digits should decrement automatically
							BtnPressTimerStart();
							break;
						}
						
						case e_mstate_timerFinished:
						{
							OneSecTimerReset();
							BuzzOff();
							mState = e_mstate_timerInit;
							break;
						}
						
						default: { }
					}
					break;
				}
				
				case 1: // btn 2
				{
					switch (mState)
					{
						// if the button is pressed in init state, go to time setting
						case e_mstate_timerInit:
						{
							mState = e_mstate_timerSet_mm;
							break;
						}
						
						// change to ss but leave mm digits on
						case e_mstate_timerSet_mm:
						{
							shReg.digitVisible.u8[2] = 1;
							shReg.digitVisible.u8[3] = 1;
							mState = e_mstate_timerSet_ss;
							BtnPressTimerStart();
							break;
						}
						
						// change to mm and leave ss digits on
						case e_mstate_timerSet_ss:
						{
							shReg.digitVisible.u8[0] = 1;
							shReg.digitVisible.u8[1] = 1;
							mState = e_mstate_timerSet_mm;
							BtnPressTimerStart();
							break;
						}
						
						// when button is pressed while timer is running, pause it.
						// if hold long enough will reset timer
						case e_mstate_timerRunning:
						{
							OneSecTimerStop();
							BtnPressTimerStart();
							mState = e_mstate_timerPaused;
							break;
						}
						
						// And start again if paused
						// if hold long enough will reset timer
						case e_mstate_timerPaused:
						{
							OneSecTimerStart();
							BtnPressTimerStart();
							mState = e_mstate_timerRunning;
							break;
						}
						
						// if button is pressed when counting is finished, go to init state
						case e_mstate_timerFinished:
						{
							OneSecTimerReset();
							BuzzOff();
							mState = e_mstate_timerInit;
							break;
						}
						
						default: { }
					}
					break;
				}
				
				default: { } // cant happen
			}
		}
	}
	
	// fill the register, reset first
	shReg.timeReg.u32 = 0;
	
	// digit 1: seconds lsd
	if (shReg.digitVisible.u8[0])
		shReg.timeReg.u32 |= digits[0][time.ss % 10];
			
	// digit 2: seconds msd
	if (shReg.digitVisible.u8[1])
		shReg.timeReg.u32 |= digits[1][time.ss / 10];
			
	// digit 3: minutes lsd
	if (shReg.digitVisible.u8[2])
		shReg.timeReg.u32 |= digits[2][time.mm % 10];
		
	// digit 4: minutes msd
	if (shReg.digitVisible.u8[3])	
		shReg.timeReg.u32 |= digits[3][time.mm / 10];
		
	// add the current colon
	shReg.timeReg.u32 |= colon.u32;
	
	// delay the colon blink
	if (colonPresc <= COLON_PRESC)
		colonPresc++;
	else
	{
		colonPresc = 0;
		colon.u32 ^= COLON_OR_MASK;
		
		switch (mState)
		{
			// ring buzzer with this frequency if state is finished and blink the digits
			case e_mstate_timerFinished:
			{
				BuzzToggle();
				shReg.digitVisible.u32 ^= 0x01010101;
				break;
			}
			
			// blink only minutes digits 4 and 3
			case e_mstate_timerSet_mm:
			{
				shReg.digitVisible.u8[2] ^= 1;
				shReg.digitVisible.u8[3] ^= 1;
				
				if (e_holdAction_Increment == holdAction)
				{
					time.mm += HOLD_ACTION_VAL;
					
					if (time.mm > TIME_MAX_MM)
						time.mm = 0;
				}
				else if (e_holdAction_Decrement == holdAction)
				{
					time.mm -= HOLD_ACTION_VAL;
					
					if (time.mm < 0)
						time.mm = TIME_MAX_MM;
				}
				break;
			}
			
			// blink only seconds digits 2 and 1
			case e_mstate_timerSet_ss:
			{
				shReg.digitVisible.u8[0] ^= 1;
				shReg.digitVisible.u8[1] ^= 1;
				
				if (e_holdAction_Increment == holdAction)
				{
					time.ss += HOLD_ACTION_VAL;
					
					if (time.ss > TIME_MAX_SS)
						time.ss = 0;
				}
				else if (e_holdAction_Decrement == holdAction)
				{
					time.ss -= HOLD_ACTION_VAL;
					
					if (time.ss < 0)
						time.ss = TIME_MAX_SS;
				}
				break;
			}
			
			default: { }
		}
	}
}

/************************************************************************
@brief	Update time in this interrupt routine
************************************************************************/
ISR(TIMER2_COMPA_vect)
{
	//BuzzToggle();
	// as soon as this timer is on, start decrementing time.
	time.ss--;
	
	if (time.ss < 0)
	{
		time.ss = TIME_MAX_SS;
		time.mm--;
		
		if (time.mm < 0)
		{
			// counting has been finished, counter reached 0
			OneSecTimerReset();
			
			// change machine state
			mState = e_mstate_timerFinished;
		}
	}
}

/************************************************************************
When button is pressed longer than 1 second, this interrupt routine hits in
************************************************************************/
ISR(TIMER1_COMPA_vect)
{
	// turn off the rimer
	BtnPressTimerStop();
	
	// choose action
	switch (mState)
	{
		case e_mstate_timerSet_mm:
		case e_mstate_timerSet_ss:
		{
			// start the timer only if any time is set and the middle btn was pressed. 
			// All digits visible
			if ( ((time.mm) || (time.ss)) && (!buttons[1].isHigh) )
			{
				shReg.digitVisible.u32 = 0x01010101;
				mState = e_mstate_timerRunning;
				OneSecTimerStart();
			}
			
			// check if any of the up/down buttons caused event and set holdAction
			else if (!buttons[0].isHigh)
				holdAction = e_holdAction_Decrement;
			else if (!buttons[2].isHigh)
				holdAction = e_holdAction_Increment;
			break;
		}
		
		case e_mstate_timerRunning:
		case e_mstate_timerPaused:
		{
			// reinit the timer if middle button is pressed
			if (!buttons[1].isHigh)
			{
				OneSecTimerReset();
				BuzzOff();
				mState = e_mstate_timerInit;
				break;
			}
		}
		
		default: { }
	}
}



