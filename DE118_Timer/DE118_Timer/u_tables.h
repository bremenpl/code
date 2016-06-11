/*
 * u_tables.h
 *
 * Created: 2016-06-10 19:47:38
 *  Author: Lukasz
 */ 


#ifndef U_TABLES_H_
#define U_TABLES_H_

#include <stdint.h>

// Signs or masks
// For pinout check the schematics in DE118_Timer project

// Colon
#define COLON_OR_MASK	((uint32_t)0x00000100)

#define NR_OF_ROWS		4
#define NR_OF_DIGITS	10

// Digits
static const uint32_t digits[NR_OF_ROWS][NR_OF_DIGITS] =
{
	//	    0		    1			2			3			4			5			6			7			8			9
	/*1*/ { 0x001CE000, 0x00048000, 0x001AC000, 0x000EC000, 0x0006A000, 0x000E6000, 0x001E6000, 0x0004C000, 0x001EE000, 0x000EE000 },
	/*2*/ { 0x00E00E00, 0x00200800, 0x00C01C00, 0x00601C00, 0x00201A00, 0x00601600, 0x00E01600, 0x00200C00, 0x00E01E00, 0x00601E00 },
	/*3*/ { 0x0E000070, 0x02000040, 0x0C0000E0, 0x060000E0, 0x020000D0, 0x060000B0, 0x0E0000B0, 0x02000060, 0x0E0000F0, 0x060000F0 },
	/*4*/ { 0x70000007, 0x10000004, 0x6000000E, 0x3000000E, 0x1000000D, 0x3000000B, 0x7000000B, 0x10000006, 0x7000000F, 0x3000000F }
};

typedef union
{
	uint32_t		u32;
	uint8_t			u8[4];
} union32_t;

typedef struct  
{
	int8_t			mm;			/*!< Minutes */
	int8_t			ss;			/*!< Seconds */
} time_t;

typedef enum
{
	e_mstate_timerInit		= 0,
	e_mstate_timerRunning	= 1,
	e_mstate_timerPaused	= 2,
	e_mstate_timerFinished	= 3,
	e_mstate_timerSet_mm	= 4,
	e_mstate_timerSet_ss	= 5,
} mstate_t;

typedef enum
{
	e_holdAction_None		= 0,
	e_holdAction_Increment	= 1,
	e_holdAction_Decrement	= 2
} holdAction_t;

#endif /* U_TABLES_H_ */