
/*
 * lcd.h
 *
 *  Created on: Oct 26, 2014
 *      Author: John Convertino
 * Interface file, this is a more basic and self contained
 * implimentation of the arduino liquid crysral library.
 * This is a 4 bit, write only interface implimentation.
 *
 *
 *      Version: v0.6
 *      See implementation file for details.
 */

#ifndef LCD_H_
#define LCD_H_

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

//easy way to change delay for porting
#define DELAY_MS(x) _delay_ms(x)
#define DELAY_US(x) _delay_us(x)

//initializes port and screen for 4 bit mode
void initLCD(volatile uint8_t *data_port, uint8_t screenSize, uint8_t width, uint8_t precision, uint8_t base);
//print
void printLCD(char *);
void printIntLCD(int);
void printDecLCD(double);
void printSpecialLCD(uint8_t);
//set cursor to a position on screen (columns by rows)
void setCursorLCD(uint8_t row, uint8_t col);
//clear screen and set cursor for home
void clearLCD();
//scroll screen to the left by one space
void scrollDisplayLeftLCD();
//scroll screen to the right by one space
void scrollDisplayRightLCD();
//set cursor to the home position
void homeLCD();
//turn the display off
void displayOffLCD();
//turn the display on
void displayOnLCD();
//turn the cursor off
void cursorOffLCD();
//turn the cursor on
void cursorOnLCD();
//turn blinking off
void blinkOffLCD();
//turn blinking on
void blinkOnLCD();
//set text flow left to right
void leftToRightLCD();
//set text flow right to left
void rightToLeftLCD();
//turn off autoscroll
void autoscrollOffLCD();
//turn on autoscroll
void autoscrollOnLCD();


#endif /* LCD_H_ */
