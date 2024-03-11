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

#ifndef _LCD_H_
#define _LCD_H_

#include <inttypes.h>

// LCD defines were imported from the Arduino LiquidCrystal.h, no need to reinvent the wheel.
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

//RS and enable places in bit pattern
#define RS 0x20
#define ENABLE 0x10

//setup stuffs
#define INS_REG	 0
#define DATA_REG 1

/***************************************************************************//**
 * @typedef write_callback
 * @brief   generic typedef for writer callback
 ******************************************************************************/
typedef void (*write_callback)(void *p_lcd, uint8_t, int);

/**
 * @struct s_lcd
 * @brief Struct for containing hitachi LCD instances
 */
struct s_lcd
{
  /**
   * @var s_lcd::p_dataPort
   * data port to use for writing output data.
   */
  volatile uint8_t *p_dataPort;
  /**
   * @var s_lcd::p_ctrlPort
   * control port to use for controlling RS/ENABLE.
   */
  volatile uint8_t *p_ctrlPort;
  /**
   * @var s_lcd::rs
   * Bit used for rs on control port
   */
  uint8_t rs;
  /**
   * @var s_lcd::ena
   * Bit used for enable on control port
   */
  uint8_t ena;
  /**
   * @var s_lcd::displaySetting
   * Store display settings
   */
  uint8_t displaySetting;
  /**
   * @var s_lcd::entryModeSet
   * Store entry mode settings
   */
  uint8_t entryModeSet;
  /**
   * @var s_lcd::functionSet
   * Store function settings
   */
  uint8_t functionSet;
  /**
   * @var s_lcd::screenSize
   * Store screen size settings
   */
  uint8_t screenSize;
  /**
   * @var s_lcd::precision
   * Store precision settings
   */
  uint8_t precision;
  /**
   * @var s_lcd::width
   * Store width settings
   */
  uint8_t width;
  /**
   * @var s_lcd::base
   * Store base settings
   */
  uint8_t base;
  /**
   * @var s_lcd::write
   * function pointer for write method (8 vs 4 bit).
   */
  write_callback write;
};

/***************************************************************************//**
 * @brief   Initialize hitachi LCD port with data set for 0 to 3, 4 = enable, 5 = RS
 *
 * @param   p_temp LCD struct pointer
 * @param   p_dataPort pointer to data register (PORT).
 * @param   screenSize size of the screen (in number of characters).
 * @param   width number of rows of the screen.
 * @param   precision decimal presented.
 * @param   base number base (10, 16)
 ******************************************************************************/
void initLCD(struct s_lcd *p_temp, volatile uint8_t *p_dataPort, uint8_t screenSize, uint8_t width, uint8_t precision, uint8_t base);

/***************************************************************************//**
 * @brief   Initialize hitachi LCD ports for correct I/O and set pin numbers
 *
 * @param   p_temp LCD struct pointer
 * @param   p_dataPort pointer to data register (PORT).
 * @param   p_ctrlPort pointer to control register (PORT).
 * @param   rs pin to use for register select on ctrlPort
 * @param   ena pin to use for enable select on ctrlPort
 * @param   mode 0 for 4 bit mode, anything else is 8 bit.
 * @param   screenSize size of the screen (in number of characters).
 * @param   width number of rows of the screen.
 * @param   precision decimal presented.
 * @param   base number base (10, 16)
 ******************************************************************************/
void initLCD_custom(struct s_lcd *p_temp, volatile uint8_t *p_dataPort, volatile uint8_t *p_ctrlPort, uint8_t rs, uint8_t ena, uint8_t mode, uint8_t screenSize, uint8_t width, uint8_t precision, uint8_t base);

/***************************************************************************//**
 * @brief   print string to LCD
 *
 * @param   p_lcd LCD struct pointer
 * @param   message Null terminated string to print
 ******************************************************************************/
void printLCD(struct s_lcd *p_lcd, char *message);

/***************************************************************************//**
 * @brief   print int to LCD
 *
 * @param   p_lcd LCD struct pointer
 * @param   number integer to print
 ******************************************************************************/
void printIntLCD(struct s_lcd *p_lcd, int number);

/***************************************************************************//**
 * @brief   print decimal(double) to LCD
 *
 * @param   p_lcd LCD struct pointer
 * @param   number double to print
 ******************************************************************************/
void printDecLCD(struct s_lcd *p_lcd, double number);

/***************************************************************************//**
 * @brief   print raw 8 bit data to LCD
 *
 * @param   p_lcd LCD struct pointer
 * @param   messge 8bit value to print ot screen
 ******************************************************************************/
void printSpecialLCD(struct s_lcd *p_lcd, uint8_t message);

/***************************************************************************//**
 * @brief   set cursor to a position on screen (columns by rows)
 *
 * @param   p_lcd LCD struct pointer
 * @param   row number to index starting at 0
 * @param   col number to index starting at 0
 ******************************************************************************/
void setCursorLCD(struct s_lcd *p_lcd, uint8_t row, uint8_t col);

/***************************************************************************//**
 * @brief   clear screen and set cursor for home
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void clearLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   scroll screen to the left by one space
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void scrollDisplayLeftLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   scroll screen to the right by one space
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void scrollDisplayRightLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   set cursor to the home position
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void homeLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn the display off
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void displayOffLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn the display on
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void displayOnLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn the cursor off
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void cursorOffLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn the cursor on
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void cursorOnLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn blining off
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void blinkOffLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn blining ob
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void blinkOnLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   set text flow left to right
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void leftToRightLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   set text flow right to left
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void rightToLeftLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn off autoscroll
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void autoscrollOffLCD(struct s_lcd *p_lcd);

/***************************************************************************//**
 * @brief   turn on autoscroll
 *
 * @param   p_lcd LCD struct pointer
 ******************************************************************************/
void autoscrollOnLCD(struct s_lcd *p_lcd);


#endif /* LCD_H_ */
