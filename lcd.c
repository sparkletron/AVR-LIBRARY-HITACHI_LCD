/*
 * lcd.c
 *
 *  Created on: Oct 26, 2014
 *      Author: John Convertino
 *
 *      Version: v0.6
 *      11/5/14  v0.6	initial port from C++ library
 */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/common.h>

#include "commonDefines.h"
#include "lcd.h"


typedef struct
{
	volatile uint8_t *PORT;
	uint8_t displaySetting;
	uint8_t entryModeSet;
	uint8_t functionSet;
	uint8_t screenSize;
	uint8_t precision;
	uint8_t width;
	uint8_t base;
} s_lcd;

static s_lcd lcd;
void write(uint8_t, int);
void enablePulse();

//setup LCD screen for 4 wire mode Write Only
void lcdInitialize(volatile uint8_t *data_port,  uint8_t screenSize, uint8_t width, uint8_t precision, uint8_t base)
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.screenSize = screenSize;
	lcd.width = width;
	lcd.precision = precision;
	lcd.base = base;
	lcd.PORT = data_port;
	*(lcd.PORT - 1) |= 0x3F;
	//setup as defined in Hitachi Datasheet page 46, delays and all
	//set port bits low
	*(lcd.PORT) &= ~MASK_8BIT_FF;
	//set RS to instruction mode
	*(lcd.PORT) &= ~RS;
	DELAY_MS(50);
	//set port values
	*(lcd.PORT) |= 0x03;
	//latch values
	enablePulse();
	DELAY_MS(5);
	//latch values
	enablePulse();
	DELAY_US(200);
	//latch values
	enablePulse();
	//setup for 4 bit mode
	*(lcd.PORT) |= 0x02;
	*(lcd.PORT) &= ((MASK_8BIT_FF << 4) | 0x02);
	enablePulse();
	//function set again, this time once and for all
	lcd.functionSet = (LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
	write(lcd.functionSet, INS_REG);
	//display control, enable display and setup cursor and blink
	lcd.displaySetting = (LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	write(lcd.displaySetting, INS_REG);
	//clear display and set cursor to home
	clear();
	//setup LCD entry mode
	lcd.entryModeSet = (LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
	write(lcd.entryModeSet, INS_REG);

	SREG = tmpSREG;
}
//print string array to display
void print(char *message)
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	//as long as pointer isn't pointing to null
	while(*message != '\0')
	{
		//write current character from string
		write((uint8_t)*message, DATA_REG);
		message++;

	}
	SREG = tmpSREG;
}

void printSpecial(uint8_t message)
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	//write current character
	write(message, DATA_REG);

	SREG = tmpSREG;
}

//convert ints to string
void printInt(int number)
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	char buffer[lcd.screenSize];

	print(ltoa(number, buffer, lcd.base));

	SREG = tmpSREG;
}

//convert doubles to string
void printDec(double number)
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	char buffer[lcd.screenSize];

	print(dtostrf(number, lcd.width, lcd.base, buffer));

	SREG = tmpSREG;
}

//shift display to the left by one character
void scrollDisplayLeft()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

  write((LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT), INS_REG);

	SREG = tmpSREG;
}

//shift display to the right by one character
void scrollDisplayRight()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

  write((LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT), INS_REG);

	SREG = tmpSREG;
}

//clear display, also sets cursor at home position, needs a long delay to work.
void clear()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	write(LCD_CLEARDISPLAY, INS_REG);
	DELAY_MS(2);

	SREG = tmpSREG;
}

//set cursor back to home position (0,0)
void home()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	write(LCD_RETURNHOME, INS_REG);
	DELAY_MS(2);

	SREG = tmpSREG;
}

//turn off dispaly
void displayOff()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.displaySetting &= ~LCD_DISPLAYON;
	write(lcd.displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn on display
void displayOn()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.displaySetting |= LCD_DISPLAYON;
	write(lcd.displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn off cursor
void cursorOff()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.displaySetting &= ~LCD_CURSORON;
	write(lcd.displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn on cursor
void cursorOn()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.displaySetting |= LCD_CURSORON;
	write(lcd.displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn off blinking cursor
void blinkOff()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.displaySetting &= ~LCD_BLINKON;
	write(lcd.displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn of blinking cursor
void blinkOn()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.displaySetting |= LCD_BLINKON;
	write(lcd.displaySetting, INS_REG);

	SREG = tmpSREG;
}

//set text to flow Left to Right
void leftToRight()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.entryModeSet |= LCD_ENTRYLEFT;
	write(lcd.entryModeSet, INS_REG);

	SREG = tmpSREG;
}

//set text to flow Right to Left
void rightToLeft()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.entryModeSet &= ~LCD_ENTRYLEFT;
	write(lcd.entryModeSet, INS_REG);

	SREG = tmpSREG;
}

// This will 'right justify' text from the cursor
void autoscrollOn()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.entryModeSet |= LCD_ENTRYSHIFTINCREMENT;
	write(lcd.entryModeSet, INS_REG);

	SREG = tmpSREG;
}

// This will 'left justify' text from the cursor
void autoscrollOff()
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	lcd.entryModeSet &= ~LCD_ENTRYSHIFTINCREMENT;
	write(lcd.entryModeSet, INS_REG);

	SREG = tmpSREG;
}

//allows a cursor to be set, row is defined and col is used as an offset.
void setCursor(uint8_t row, uint8_t col)
{
	uint8_t tmpSREG = 0;

	tmpSREG = SREG;
	cli();

	switch(row)
	{
		//line 2
		case 1:
			write((LCD_SETDDRAMADDR | (col + 0x40)), INS_REG);
			break;
		//line 3
		case 2:
			write((LCD_SETDDRAMADDR | (col + 0x14)), INS_REG);
			break;
		//line 4
		case 3:
			write((LCD_SETDDRAMADDR | (col + 0x00)), INS_REG);
			break;
			//line 1
		case 0:
		default:
			write((LCD_SETDDRAMADDR | (col + 0x00)), INS_REG);
			break;
	}

	SREG = tmpSREG;
}

//private command used to write data to data lines
void write(uint8_t data, int regSelection)
{
	//instruction or data mode
	if (regSelection)
		//data mode
		*(lcd.PORT) |= RS;
	else
		//instruction mode
		*(lcd.PORT) &= ~RS;
	//send out top nibble
	*(lcd.PORT) |= data >> 4;
	*(lcd.PORT) &= ((MASK_8BIT_FF << 4) | (data >> 4));
	//latch data
	enablePulse();
	//send out bottom nibble
	*(lcd.PORT) |= (~(MASK_8BIT_FF << 4) & data);
	*(lcd.PORT) &= ((MASK_8BIT_FF << 4) | data);
	//latch data
	enablePulse();
}

//routine to pulse enable pin to latch data
void enablePulse()
{
	//make sure enable is low
	*(lcd.PORT) &= ~ENABLE;
	DELAY_US(1);
	//enable set to high
	*(lcd.PORT) |= ENABLE;
	// enable pulse must be >450ns
	DELAY_US(1);
	//enable set to low
	*(lcd.PORT) &= ~ENABLE;
	 // commands need > 37us to settle
	DELAY_US(100);
}
