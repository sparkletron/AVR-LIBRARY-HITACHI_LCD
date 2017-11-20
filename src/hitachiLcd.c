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
#include "hitachiLcd.h"

void write(struct s_lcd *p_lcd, uint8_t, int);
void enablePulse(struct s_lcd *p_lcd);

//setup LCD screen for 4 wire mode Write Only
struct s_lcd *initLCD(volatile uint8_t *data_port,  uint8_t screenSize, uint8_t width, uint8_t precision, uint8_t base)
{
	uint8_t tmpSREG = 0;
	struct s_lcd *p_temp = NULL;

	tmpSREG = SREG;
	cli();
	
	p_temp = malloc(sizeof(struct s_lcd));

	if(p_temp == NULL) return NULL;
	
	p_temp->screenSize = screenSize;
	p_temp->width = width;
	p_temp->precision = precision;
	p_temp->base = base;
	p_temp->PORT = data_port;
	*(p_temp->PORT - 1) |= 0x3F;
	//setup as defined in Hitachi Datasheet page 46, delays and all
	//set port bits low
	*(p_temp->PORT) &= ~MASK_8BIT_FF;
	//set RS to instruction mode
	*(p_temp->PORT) &= ~RS;
	_delay_ms(50);
	//set port values
	*(p_temp->PORT) |= 0x03;
	//latch values
	enablePulse(p_temp);
	_delay_ms(5);
	//latch values
	enablePulse(p_temp);
	_delay_us(200);
	//latch values
	enablePulse(p_temp);
	//setup for 4 bit mode
	*(p_temp->PORT) |= 0x02;
	*(p_temp->PORT) &= ((MASK_8BIT_FF << 4) | 0x02);
	enablePulse(p_temp);
	//function set again, this time once and for all
	p_temp->functionSet = (LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
	write(p_temp, p_temp->functionSet, INS_REG);
	//display control, enable display and setup cursor and blink
	p_temp->displaySetting = (LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
	write(p_temp, p_temp->displaySetting, INS_REG);
	//clear display and set cursor to home
	clearLCD(p_temp);
	//setup LCD entry mode
	p_temp->entryModeSet = (LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
	write(p_temp, p_temp->entryModeSet, INS_REG);

	SREG = tmpSREG;
	
	return p_temp;
}
//print string array to display
void printLCD(struct s_lcd *p_lcd, char *message)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	//as long as pointer isn't pointing to null
	while(*message != '\0')
	{
		//write current character from string
		write(p_lcd, (uint8_t)*message, DATA_REG);
		message++;

	}
	SREG = tmpSREG;
}

void printSpecialLCD(struct s_lcd *p_lcd, uint8_t message)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	//write current character
	write(p_lcd, message, DATA_REG);

	SREG = tmpSREG;
}

//convert ints to string
void printIntLCD(struct s_lcd *p_lcd, int number)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	char buffer[p_lcd->screenSize];

	printLCD(p_lcd, ltoa(number, buffer, p_lcd->base));

	SREG = tmpSREG;
}

//convert doubles to string
void printDecLCD(struct s_lcd *p_lcd, double number)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	char buffer[p_lcd->screenSize];

	printLCD(p_lcd, dtostrf(number, p_lcd->width, p_lcd->base, buffer));

	SREG = tmpSREG;
}

//shift display to the left by one character
void scrollDisplayLeftLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

  write(p_lcd, (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT), INS_REG);

	SREG = tmpSREG;
}

//shift display to the right by one character
void scrollDisplayRightLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

  write(p_lcd, (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT), INS_REG);

	SREG = tmpSREG;
}

//clear display, also sets cursor at home position, needs a long delay to work.
void clearLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	write(p_lcd, LCD_CLEARDISPLAY, INS_REG);
	_delay_ms(2);

	SREG = tmpSREG;
}

//set cursor back to home position (0,0)
void homeLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	write(p_lcd, LCD_RETURNHOME, INS_REG);
	_delay_ms(2);

	SREG = tmpSREG;
}

//turn off dispaly
void displayOffLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->displaySetting &= ~LCD_DISPLAYON;
	write(p_lcd, p_lcd->displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn on display
void displayOnLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->displaySetting |= LCD_DISPLAYON;
	write(p_lcd, p_lcd->displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn off cursor
void cursorOffLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->displaySetting &= ~LCD_CURSORON;
	write(p_lcd, p_lcd->displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn on cursor
void cursorOnLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->displaySetting |= LCD_CURSORON;
	write(p_lcd, p_lcd->displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn off blinking cursor
void blinkOffLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->displaySetting &= ~LCD_BLINKON;
	write(p_lcd, p_lcd->displaySetting, INS_REG);

	SREG = tmpSREG;
}

//turn of blinking cursor
void blinkOnLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->displaySetting |= LCD_BLINKON;
	write(p_lcd, p_lcd->displaySetting, INS_REG);

	SREG = tmpSREG;
}

//set text to flow Left to Right
void leftToRightLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->entryModeSet |= LCD_ENTRYLEFT;
	write(p_lcd, p_lcd->entryModeSet, INS_REG);

	SREG = tmpSREG;
}

//set text to flow Right to Left
void rightToLeftLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->entryModeSet &= ~LCD_ENTRYLEFT;
	write(p_lcd, p_lcd->entryModeSet, INS_REG);

	SREG = tmpSREG;
}

// This will 'right justify' text from the cursor
void autoscrollOnLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->entryModeSet |= LCD_ENTRYSHIFTINCREMENT;
	write(p_lcd, p_lcd->entryModeSet, INS_REG);

	SREG = tmpSREG;
}

// This will 'left justify' text from the cursor
void autoscrollOffLCD(struct s_lcd *p_lcd)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	p_lcd->entryModeSet &= ~LCD_ENTRYSHIFTINCREMENT;
	write(p_lcd, p_lcd->entryModeSet, INS_REG);

	SREG = tmpSREG;
}

//allows a cursor to be set, row is defined and col is used as an offset.
void setCursorLCD(struct s_lcd *p_lcd, uint8_t row, uint8_t col)
{
	uint8_t tmpSREG = 0;

	if(p_lcd == NULL) return;
	
	tmpSREG = SREG;
	cli();

	switch(row)
	{
		//line 2
		case 1:
			write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x40)), INS_REG);
			break;
		//line 3
		case 2:
			write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x14)), INS_REG);
			break;
		//line 4
		case 3:
			write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x00)), INS_REG);
			break;
			//line 1
		case 0:
		default:
			write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x00)), INS_REG);
			break;
	}

	SREG = tmpSREG;
}

void freeLCD(struct s_lcd *p_lcd)
{
	if(p_lcd == NULL) return;
	
	free(p_lcd);
}

//private command used to write data to data lines
void write(struct s_lcd *p_lcd, uint8_t data, int regSelection)
{
	if(p_lcd == NULL) return;
	
	//instruction or data mode
	if (regSelection)
	{
		*(p_lcd->PORT) |= RS;
	}
	else
	{
		*(p_lcd->PORT) &= ~RS;
	}
	//send out top nibble
	*(p_lcd->PORT) |= data >> 4;
	*(p_lcd->PORT) &= ((MASK_8BIT_FF << 4) | (data >> 4));
	//latch data
	enablePulse(p_lcd);
	//send out bottom nibble
	*(p_lcd->PORT) |= (~(MASK_8BIT_FF << 4) & data);
	*(p_lcd->PORT) &= ((MASK_8BIT_FF << 4) | data);
	//latch data
	enablePulse(p_lcd);
}

//routine to pulse enable pin to latch data
void enablePulse(struct s_lcd *p_lcd)
{
	if(p_lcd == NULL) return;
	
	//make sure enable is low
	*(p_lcd->PORT) &= ~ENABLE;
	_delay_us(1);
	//enable set to high
	*(p_lcd->PORT) |= ENABLE;
	// enable pulse must be >450ns
	_delay_us(1);
	//enable set to low
	*(p_lcd->PORT) &= ~ENABLE;
	 // commands need > 37us to settle
	_delay_us(100);
}
