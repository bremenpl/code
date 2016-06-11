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



#endif /* U_TABLES_H_ */