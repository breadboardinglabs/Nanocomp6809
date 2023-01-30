/***************************************************************************//**

  @file         nccurses.h

  @author       Dave Henry

  @date         Created Friday 27 Jan 2023

  @brief        Nanocomp implementation of Ncurses functions used by Tetris

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#ifndef NCCURSES_H
#define NCCURSES_H

#include "cmoc.h"
#include "cmocextra.h"


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

#define KEY_LEFT '7'
#define KEY_RIGHT '9'
#define KEY_UP '8'
#define KEY_DOWN '4'


/*
  Redefined ncurses Window 
 */
typedef struct {
  int wrow;
  int wcol;
  int wheight;
  int wwidth;
  int disprow;
  int dispcol;
} WINDOW;

// ncurses replacements
void box(WINDOW* w, int a, int b);
void wmove(WINDOW* w, int wrow, int wcol);
void wclear(WINDOW* w);
void wprintw(WINDOW* w, char* outString,int arg1);
WINDOW* newwin(int wrow, int wcol, int wheight, int wwidth);
byte init_pair(int foreground, int background);

#endif // NCCURSES_H
