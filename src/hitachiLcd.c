/*******************************************************************************
* @file    hitachLCD.c
* @author  Jay Convertino(electrobs@gmail.com)
* @date    2024.03.11
* @brief   Library for hitachi 44780 LCD controller
* @details Originally based on the Arduino C++ version way back in 2014.
* @version 0.6.0
*
* @TODO
*  - Cleanup interface
*
* @license mit
*
* Copyright 2024 Johnathan Convertino
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
******************************************************************************/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/common.h>

#include "commonDefines.h"
#include "hitachiLcd.h"

void write_4bit(void *p_lcd, uint8_t data, int regSel);
void write_8bit(void *p_lcd, uint8_t data, int regSel);
void enaPulse(struct s_lcd *p_lcd);

//setup LCD screen for 4 wire mode Write Only
void initLCD(struct s_lcd *p_temp, volatile uint8_t *p_dataPort,  uint8_t screenSize, uint8_t width, uint8_t precision, uint8_t base)
{
  uint8_t tmpSREG = 0;
  tmpSREG = SREG;
  cli();

  if(p_temp == NULL) return;

  p_temp->write = write_4bit;

  p_temp->screenSize = screenSize;
  p_temp->width = width;
  p_temp->precision = precision;
  p_temp->base = base;
  p_temp->p_dataPort = p_dataPort;
  p_temp->rs = RS;
  p_temp->ena = ENABLE;
  p_temp->p_ctrlPort = p_dataPort;
  *(p_temp->p_dataPort - 1) |= 0x3F;
  // *(p_temp->p_ctrlPort - 1) |= 0x30;
  //setup as defined in Hitachi Datasheet page 46, delays and all
  //set port bits low
  *(p_temp->p_dataPort) &= ~MASK_8BIT_FF;
  //set RS to instruction mode
  *(p_temp->p_ctrlPort) &= ~(p_temp->rs);
  _delay_ms(50);
  //set port values
  *(p_temp->p_dataPort) |= 0x03;
  //latch values
  enaPulse(p_temp);
  _delay_ms(5);
  //latch values
  enaPulse(p_temp);
  _delay_us(200);
  //latch values
  enaPulse(p_temp);
  //setup for 4 bit mode
  *(p_temp->p_dataPort) |= 0x02;
  *(p_temp->p_dataPort) &= ((MASK_8BIT_FF << 4) | 0x02);
  enaPulse(p_temp);
  //function set again, this time once and for all
  p_temp->functionSet = (LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
  p_temp->write(p_temp, p_temp->functionSet, INS_REG);
  //display control, enable display and setup cursor and blink
  p_temp->displaySetting = (LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
  p_temp->write(p_temp, p_temp->displaySetting, INS_REG);
  //clear display and set cursor to home
  clearLCD(p_temp);
  //setup LCD entry mode
  p_temp->entryModeSet = (LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
  p_temp->write(p_temp, p_temp->entryModeSet, INS_REG);

  SREG = tmpSREG;
}

void initLCD_custom(struct s_lcd *p_temp, volatile uint8_t *p_dataPort, volatile uint8_t *p_ctrlPort, uint8_t rs, uint8_t ena, uint8_t mode, uint8_t screenSize, uint8_t width, uint8_t precision, uint8_t base)
{
  uint8_t tmpSREG = 0;

  tmpSREG = SREG;
  cli();

  if(p_temp == NULL) return;

  p_temp->write = (mode ? write_8bit : write_4bit);

  p_temp->screenSize = screenSize;
  p_temp->width = width;
  p_temp->precision = precision;
  p_temp->base = base;
  p_temp->p_dataPort = p_dataPort;
  p_temp->rs = (1 << rs);
  p_temp->ena = (1 << ena);
  p_temp->p_ctrlPort = p_ctrlPort;
  //set output ports for data port
  *(p_temp->p_dataPort - 1) |= (mode ? ~0 : ~(~0 << 4));
  //setup control port
  *(p_temp->p_ctrlPort -1) |= p_temp->rs | p_temp->ena;
  //setup as defined in Hitachi Datasheet page 45/46, delays and all
  //set port bits low
  *(p_temp->p_dataPort) &= ~MASK_8BIT_FF;
  //set RS to instruction mode
  *(p_temp->p_ctrlPort) &= ~(p_temp->rs);
  _delay_ms(50);
  //set port values
  *(p_temp->p_dataPort) |= (mode ? 0x30 : 0x03);
  //latch values
  enaPulse(p_temp);
  _delay_ms(5);
  //latch values
  enaPulse(p_temp);
  _delay_us(150);
  //latch values
  enaPulse(p_temp);
  //setup
  *(p_temp->p_dataPort) |= (mode ? 0x30 : 0x02);
  *(p_temp->p_dataPort) &= (mode ? 0x30 : ((MASK_8BIT_FF << 4) | 0x02));
  enaPulse(p_temp);

  //function set again, this is once and for all
  p_temp->functionSet = (LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | (mode ? LCD_8BITMODE : LCD_4BITMODE));
  p_temp->write(p_temp, p_temp->functionSet, INS_REG);
  //display control, enable display and setup cursor and blink
  p_temp->displaySetting = (LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
  p_temp->write(p_temp, p_temp->displaySetting, INS_REG);
  //clear display and set cursor to home
  clearLCD(p_temp);
  //setup LCD entry mode
  p_temp->entryModeSet = (LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
  p_temp->write(p_temp, p_temp->entryModeSet, INS_REG);

  SREG = tmpSREG;
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
    p_lcd->write(p_lcd, (uint8_t)*message, DATA_REG);
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
  p_lcd->write(p_lcd, message, DATA_REG);

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

  p_lcd->write(p_lcd, (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT), INS_REG);

  SREG = tmpSREG;
}

//shift display to the right by one character
void scrollDisplayRightLCD(struct s_lcd *p_lcd)
{
  uint8_t tmpSREG = 0;

  if(p_lcd == NULL) return;

  tmpSREG = SREG;
  cli();

  p_lcd->write(p_lcd, (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT), INS_REG);

  SREG = tmpSREG;
}

//clear display, also sets cursor at home position, needs a long delay to work.
void clearLCD(struct s_lcd *p_lcd)
{
  uint8_t tmpSREG = 0;

  if(p_lcd == NULL) return;

  tmpSREG = SREG;
  cli();

  p_lcd->write(p_lcd, LCD_CLEARDISPLAY, INS_REG);
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

  p_lcd->write(p_lcd, LCD_RETURNHOME, INS_REG);
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
  p_lcd->write(p_lcd, p_lcd->displaySetting, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->displaySetting, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->displaySetting, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->displaySetting, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->displaySetting, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->displaySetting, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->entryModeSet, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->entryModeSet, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->entryModeSet, INS_REG);

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
  p_lcd->write(p_lcd, p_lcd->entryModeSet, INS_REG);

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
      p_lcd->write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x40)), INS_REG);
      break;
    //line 3
    case 2:
      p_lcd->write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x14)), INS_REG);
      break;
    //line 4
    case 3:
      p_lcd->write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x00)), INS_REG);
      break;
    //line 1
    case 0:
    default:
      p_lcd->write(p_lcd, (LCD_SETDDRAMADDR | (col + 0x00)), INS_REG);
      break;
  }

  SREG = tmpSREG;
}

//private command used to write data to data lines
void write_4bit(void *p_lcd, uint8_t data, int regSel)
{
  struct s_lcd *pc_lcd = NULL;

  if(p_lcd == NULL) return;

  pc_lcd = (struct s_lcd *)p_lcd;

  //instruction or data mode
  if (regSel)
  {
    *(pc_lcd->p_ctrlPort) |= pc_lcd->rs;
  }
  else
  {
    *(pc_lcd->p_ctrlPort) &= ~(pc_lcd->rs);
  }
  //send out top nibble
  *(pc_lcd->p_dataPort) |= data >> 4;
  *(pc_lcd->p_dataPort) &= ((MASK_8BIT_FF << 4) | (data >> 4));
  //latch data
  enaPulse(pc_lcd);
  //send out bottom nibble
  *(pc_lcd->p_dataPort) |= (~(MASK_8BIT_FF << 4) & data);
  *(pc_lcd->p_dataPort) &= ((MASK_8BIT_FF << 4) | data);
  //latch data
  enaPulse(pc_lcd);
}

//private command used to write data to data lines
void write_8bit(void *p_lcd, uint8_t data, int regSel)
{
  struct s_lcd *pc_lcd = NULL;

  if(p_lcd == NULL) return;

  pc_lcd = (struct s_lcd *)p_lcd;

  //instruction or data mode
  if (regSel)
  {
    *(pc_lcd->p_ctrlPort) |= pc_lcd->rs;
  }
  else
  {
    *(pc_lcd->p_ctrlPort) &= ~(pc_lcd->rs);
  }
  //send out full word
  *(pc_lcd->p_dataPort) = data;
  //latch data
  enaPulse(pc_lcd);
}

//routine to pulse enable pin to latch data
void enaPulse(struct s_lcd *p_lcd)
{
  if(p_lcd == NULL) return;

  //make sure enable is low
  *(p_lcd->p_ctrlPort) &= ~(p_lcd->ena);
  _delay_us(1);
  //enable set to high
  *(p_lcd->p_ctrlPort) |= p_lcd->ena;
  // enable pulse must be >450ns
  _delay_us(1);
  //enable set to low
  *(p_lcd->p_ctrlPort) &= ~(p_lcd->ena);
  // commands need > 37us to settle
  _delay_us(50);
}
