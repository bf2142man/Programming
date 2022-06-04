/*
 * File:   LCD_Main.c
 * Author: Graham Dyke
 *
 * Created on 28/05/2022
 */

#include "config.h"
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _XTAL_FREQ 4194304 // 4.194304MHz
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
// set up the timing for the LCD delays
#define LCD_delay 5 // ~5mS
#define LCD_Startup 15 // ~15mS

// Command set for Hitachi 44780U LCD display controller
#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_CURSOR_BACK 0x10
#define LCD_CURSOR_FWD 0x14
#define LCD_PAN_LEFT 0x18
#define LCD_PAN_RIGHT 0x1C
#define LCD_CURSOR_OFF 0x0C
#define LCD_CURSOR_ON 0x0E
#define LCD_CURSOR_BLINK 0x0F
#define LCD_CURSOR_LINE2 0xC0

// display controller setup commands from page 46 of Hitachi datasheet
#define FUNCTION_SET 0x28 // 4 bit interface, 2 lines, 5x8 font
#define ENTRY_MODE 0x06 // increment mode
#define DISPLAY_SETUP 0x0C // display on, cursor off, blink off


// single bit for selecting command register or data register
#define instr 0
#define data 1

// These #defines create the pin connections to the LCD in case they are changed on a future demo board

#define LCD_EN PORTCbits.RC5 // LCD enable blue wire
#define LCD_RS PORTBbits.RB6 // LCD register select line white wire
#define LCD_RW PORTBbits.RB7 // LCD read/write line green wire

#define NB_LINES 2 // Number of display lines
#define NB_COL 16 // Number of characters per line

void LCD_Initialize(void);
void LCDPutChar(char ch);
void LCDPutCmd(char ch);
void LCDPutStr(const char *); 
void LCDWriteNibble(char ch, char rs);
void LCDGoto(char pos, char ln);
void ToggleEnable(void);


void LCD_Initialize()
{
// clear latches before enabling TRIS bits
PORTB = 0;
PORTC = 0b00010000;
TRISB = 0x00;
TRISC = 0x00; //Port C pins as Outputs for RC4 (Relay) and RC5 (LCD Enable)

// required by display controller to allow power to stabilize
__delay_ms(LCD_Startup);
LCDPutCmd(0x02);//02H
// set interface size, # of lines and font
LCDPutCmd(FUNCTION_SET);//28H
// turn on display and sets up cursor
LCDPutCmd(DISPLAY_SETUP);//0CH
// set cursor movement direction
LCDPutCmd(ENTRY_MODE);//06H
// required by initialisation
LCDPutCmd(0x01);//01H

LCDPutCmd(0x80);//80H
}

void ToggleEnable(void)
{
__delay_us(10);
LCD_EN = 1; 
__delay_us(10);
LCD_EN = 0;
__delay_us(10);
}


void LCDWriteNibble(char ch, char rs)
{   
// set data/instr bit to 0 = instructions; 1 = data
LCD_RS = rs; 

// RW - set write mode
LCD_RW = 0;

//HIGH BITS

RB0 = 0x00;
RB1 = 0x00;
RB2 = 0x00;
RB3 = 0x00;

    if ((ch&0x10) == 0x10)
            RB0 = 1;
    if ((ch&0x20) == 0x20)
            RB1 = 1;
    if ((ch&0x40) == 0x40)
            RB2 = 1;
    if ((ch&0x80) == 0x80)
            RB3 = 1;
    
ToggleEnable();

//LOW BITS

RB0 = 0x00;
RB1 = 0x00;
RB2 = 0x00;
RB3 = 0x00;
   
    if ((ch&0x01) == 0x01)
            RB0 = 1;
    if ((ch&0x02) == 0x02)
            RB1 = 1;
    if ((ch&0x04) == 0x04)
            RB2 = 1;
    if ((ch&0x08) == 0x08)
            RB3 = 1;
    
 ToggleEnable();   
 

}




void LCDPutChar(char ch)
{
__delay_ms(LCD_delay);

//Send higher nibble first
LCDWriteNibble(ch,data);


}


void LCDPutCmd(char ch)
{
__delay_ms(LCD_delay);

//Send the higher nibble
LCDWriteNibble(ch,instr);

//get the lower nibble
//ch = (ch << 4);

//Now send the lower nibble
//LCDWriteNibble(ch,instr);
}


void LCDPutStr(const char *str)
{
char i=0;

// While string has not been fully traversed
while (str[i])
{
// Go display current char
LCDPutChar(str[i++]);
}

}

void LCDGoto(char pos,char ln)
{
// if incorrect line or column
if ((ln > (NB_LINES-1)) || (pos > (NB_COL-1)))
{
// Just do nothing
return;
}

// LCD_Goto command
LCDPutCmd((ln == 1) ? (0xC0 | pos) : (0x80 | pos));

// Wait for the LCD to finish
__delay_ms(LCD_delay);
}



void ADC_Initialize()
{
  ANSELH = 0x00;
  ADCON0 = 0b01000001; //ADC ON and Fosc/16 is selected

  ADCON1 = 0b11000000; // Internal reference voltage is selected

}

unsigned int ADC_Read(unsigned char channel)

{

  ADCON0 &= 0x11000101; //Clearing the Channel Selection Bits

  ADCON0 |= channel<<3; //Setting the required Bits

  __delay_ms(2); //Acquisition time to charge hold capacitor

  GO_nDONE = 1; //Initializes A/D Conversion

  while(GO_nDONE); //Wait for A/D Conversion to complete

  return ((ADRESH<<8)+ADRESL); //Returns Result

}


void main(void)
{
ADC_Initialize();
int adc; //ADC value stored
int ExposureTime = 1;
LCD_Initialize();
int i;
char Buffer[7];
char ExpTime[6];
char iTime[6];


LCDGoto(0,0); // first line
LCDPutStr("    ENLARGER  ");
LCDGoto(0,1); // first line
LCDPutStr("    TIMER V1  ");
__delay_ms(4000);
LCDPutCmd(LCD_CLEAR);

while(1)
  {
    adc = (ADC_Read(0));
 
    
    itoa(Buffer, adc, 10);

    
    if (adc > 700 & adc < 800)
    {
        ExposureTime++;// Increasing exposure time
        if (ExposureTime >99) ExposureTime = 99;
        itoa(ExpTime, ExposureTime, 10); 
        LCDGoto(0,0); // first line
        LCDPutStr(ExpTime);
        LCDPutStr(" sec exposure ");
        LCDPutCmd(LCD_HOME);
        __delay_ms(200);
    }
    else if (adc > 600 & adc < 700)
    {
        ExposureTime--;// Decrease exposure time
        if (ExposureTime <1) ExposureTime = 1;
        itoa(ExpTime, ExposureTime, 10);         
        LCDGoto(0,0); // first line
        LCDPutStr(ExpTime);
        LCDPutStr(" sec exposure  ");
        LCDPutCmd(LCD_HOME);
        __delay_ms(200);
        LCDGoto(0,0); // first line

    }
    else if (adc > 500 & adc < 600)
    {
        // RESET CONDITION
        LCDPutCmd(LCD_CLEAR);
        LCDGoto(5,0); // first line
        LCDPutStr("RESET");
        __delay_ms(1000);
        ExposureTime = 1;
        memset(ExpTime, '\0', sizeof(ExpTime));
        ExpTime[0] = '1';
        LCDPutCmd(LCD_HOME);
        LCDPutStr("    ENLARGER  ");
        LCDGoto(0,1); // first line
        LCDPutStr("    TIMER V1  ");
        __delay_ms(4000);
        LCDPutCmd(LCD_CLEAR);
    }
    else if (adc < 10)
    {
        LCDGoto(0,0); // first line
        itoa(ExpTime, ExposureTime, 10); 
        LCDPutStr(ExpTime);
        LCDPutStr(" sec exposure ");
        LCDGoto(0,1); // second line
        LCDPutStr("                ");
        LCDPutCmd(LCD_HOME);
        RC4 = 0;  // Relay on
        i = 0;
        for (i=0;i< ExposureTime;i++)
        {
            __delay_ms(915);// 1 second when combined with loop processing time
            itoa(iTime, i+1, 10); 
            LCDGoto(0,1); // second line
            LCDPutStr(iTime);
            LCDPutStr(" sec last exp");
        }
         
        RC4 = 1; // Relay off
        LCDPutCmd(LCD_HOME);
    }
    else
    {
     RC4 = 1; // Relay off
    }
    
   // LCDGoto(0,1); //Go to column 0 of second line
   // LCDPutStr(Buffer); //Display String



  }
  return;
}