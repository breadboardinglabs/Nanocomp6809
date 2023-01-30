/***************************************************************************//**

  @file         nanocomp.h

  @author       Dave Henry

  @date         Created Friday, 20 January 2023

  @brief        Nanocomp 6809 specific declarations.

  @copyright    Based on Tetris C source by Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/
#include "cmoc.h"
#include "cmocextra.h"

#ifndef NANOCOMP_H
#define NANOCOMP_H

/*
  Nanocomp VGA controller colours
 */

#define COLOR_BLACK 0
#define COLOR_DARK_GREY 1
#define COLOR_BLUE 2
#define COLOR_LIGHT_BLUE 3
#define COLOR_GREEN 4
#define COLOR_LIGHT_GREEN 5
#define COLOR_DARK_CYAN 6
#define COLOR_CYAN 7
#define COLOR_RED 8
#define COLOR_ORANGE 9
#define COLOR_MAGENTA 10
#define COLOR_PINK 11
#define COLOR_DARK_YELLOW 12
#define COLOR_YELLOW 13
#define COLOR_GREY 14
#define COLOR_WHITE 15

/*
  Nanocomp memory map constants
 */
 
#define RAM_START           0x0000
#define PIA_PORTA           0xD000
#define PIA_PORTB           0xD001
#define PIA_CTRLA           0xD002
#define PIA_CTRLB           0xD003
#define CRTC_ADDRESS        0xD400
#define CRTC_DATA_REGISTER  0xD401
#define VIDEO_RAM           0x8000 // Start of Video RAM
#define VIDEO_RAM_END       0xBFFF // End of Video RAM
#define CG_RAM              0xC000 // Start of character generator RAM
#define CG_RAM_END          0xCFFF // End of character generator RAM
#define CG_RAM_F0           0xCF00 // End of character generator RAM
#define CG_ROM              0xE000 // Start of character generator ROM

/*
  Reserved Variables above stack for Video Controller
*/
#define DISP_ROW            0x7FC0 // Current Cursor Row 0-29
#define DISP_COL            0x7FC1 // Current Cursor Column 0-79
#define DISP_ROW_OFFSET     0x7FC2 // Starting byte of current row, will increment by 160 for each row
#define DISP_CLS            0x7FC4 // Clear screen word Colour Byte + Character $F320 White on Blue space
#define DISP_START_ADDRESS  0x7FC6 //Current CRTC Start Address value R12 High, R13 Low, scroll display buffer 
#define DISP_CURSOR_H       0x7FC8 //
#define DISP_CURSOR_L       0x7FC9 //
#define DISP_BYTES_PER_ROW  0xA0;  // 160 Bytes per row

/* Nanocomp 6809 Key Definitions for original keypad */
#define KEY_CN 0x25
#define KEY_G 0x35
#define KEY_I 0x32
#define KEY_L 0x05
#define KEY_M 0x31
#define KEY_P 0x15 /* P Punch replaced with S Save */
#define KEY_S 0x15
#define KEY_R 0x30

#define KEY_0 0x22
#define KEY_1 0x24
#define KEY_2 0x02
#define KEY_3 0x12
#define KEY_4 0x14
#define KEY_5 0x00
#define KEY_6 0x10
#define KEY_7 0x04
#define KEY_8 0x01
#define KEY_9 0x11
#define KEY_A 0x03
#define KEY_B 0x13
#define KEY_C 0x23
#define KEY_D 0x33
#define KEY_E 0x21
#define KEY_F 0x20


/* Nanocomp 6809 7 Segment Display Definitions */
#define SEG_0 0x7E
#define SEG_1 0x06
#define SEG_2 0x5B
#define SEG_3 0x1F
#define SEG_4 0x27
#define SEG_5 0x3D
#define SEG_6 0x7D
#define SEG_7 0x0E
#define SEG_8 0x7F
#define SEG_9 0x3F
#define SEG_A 0x6F
#define SEG_B 0x75
#define SEG_C 0x78
#define SEG_D 0x57
#define SEG_E 0x79
#define SEG_F 0x69
#define SEG_G 0x7C
#define SEG_M 0x6E /* (upside down U) */
#define SEG_P 0x6B
#define SEG_S 0x3D /* (Same as 5) */
#define SEG_U 0x76
#define SEG_X 0x67
#define SEG_Y 0x37
#define SEG_DASH 0x01
#define SEG_SPACE 0x00


byte HexToAscii(byte inputByte);
void Delay(int w);
void ClearScreen();
void PrintChr(byte ch);
void PrintColChr(byte colour, byte ch);
void newOutputRoutine();
byte GetKey();
byte PollChr(int wait);
void ncMove (int ncRow, int ncCol);
void returnNC();
void CopyCharGenROM();


#endif // NANOCOMP_H
