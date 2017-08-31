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
	char bufSREG = SREG;
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
	SREG = bufSREG;
}
//print string array to display
void print(char *message)
{
	char bufSREG = SREG;
	cli();
	//as long as pointer isn't pointing to null
	while(*message != '\0')
	{
		//write current character from string
		write((uint8_t)*message, DATA_REG);
		message++;

	}
	SREG = bufSREG;
}

void printSpecial(uint8_t message)
{
	char bufSREG = SREG;
	cli();

	//write current character
	write(message, DATA_REG);

	SREG = bufSREG;
}

//convert ints to string
void printInt(int number)
{
	char bufSREG = SREG;
	cli();

	char buffer[lcd.screenSize];

	print(ltoa(number, buffer, lcd.base));

	SREG = bufSREG;
}

//convert doubles to string
void printDec(double number)
{
	char bufSREG = SREG;
	cli();

	char buffer[lcd.screenSize];

	print(dtostrf(number, lcd.width, lcd.base, buffer));

	SREG = bufSREG;
}

//shift display to the left by one character
void scrollDisplayLeft()
{
  write((LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT), INS_REG);
}

//shift display to the right by one character
void scrollDisplayRight()
{
  write((LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT), INS_REG);
}

//clear display, also sets cursor at home position, needs a long delay to work.
void clear()
{
	write(LCD_CLEARDISPLAY, INS_REG);
	DELAY_MS(2);
}

//set cursor back to home position (0,0)
void home()
{
	write(LCD_RETURNHOME, INS_REG);
	DELAY_MS(2);
}

//turn off dispaly
void displayOff()
{
	lcd.displaySetting &= ~LCD_DISPLAYON;
	write(lcd.displaySetting, INS_REG);
}

//turn on display
void displayOn()
{
	lcd.displaySetting |= LCD_DISPLAYON;
	write(lcd.displaySetting, INS_REG);
}

//turn off cursor
void cursorOff()
{
	lcd.displaySetting &= ~LCD_CURSORON;
	write(lcd.displaySetting, INS_REG);
}

//turn on cursor
void cursorOn()
{
	lcd.displaySetting |= LCD_CURSORON;
	write(lcd.displaySetting, INS_REG);
}

//turn off blinking cursor
void blinkOff()
{
	lcd.displaySetting &= ~LCD_BLINKON;
	write(lcd.displaySetting, INS_REG);
}

//turn of blinking cursor
void blinkOn()
{
	lcd.displaySetting |= LCD_BLINKON;
	write(lcd.displaySetting, INS_REG);
}

//set text to flow Left to Right
void leftToRight()
{
	lcd.entryModeSet |= LCD_ENTRYLEFT;
	write(lcd.entryModeSet, INS_REG);
}

//set text to flow Right to Left
void rightToLeft()
{
	lcd.entryModeSet &= ~LCD_ENTRYLEFT;
	write(lcd.entryModeSet, INS_REG);
}

// This will 'right justify' text from the cursor
void autoscrollOn()
{
	lcd.entryModeSet |= LCD_ENTRYSHIFTINCREMENT;
	write(lcd.entryModeSet, INS_REG);
}

// This will 'left justify' text from the cursor
void autoscrollOff()
{
	lcd.entryModeSet &= ~LCD_ENTRYSHIFTINCREMENT;
	write(lcd.entryModeSet, INS_REG);
}

//allows a cursor to be set, row is defined and col is used as an offset.
void setCursor(uint8_t row, uint8_t col)
{
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
}

//private command used to write data to data lines
void write(uint8_t data, int regSelection)
{
	char bufSREG = SREG;
	cli();
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
	SREG = bufSREG;
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
