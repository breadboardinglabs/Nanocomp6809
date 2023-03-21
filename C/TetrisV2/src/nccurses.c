/***************************************************************************//**

  @file         nccurses.c

  @author       Dave Henry

  @date         Created Friday 27 Jan 2023

  @brief        Nanocomp implementation of Ncurses functions used by Tetris

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

*******************************************************************************/

#include "cmoc.h"
#include "nanocomp.h"
#include "nccurses.h"
#include "stdarg.h"

// Draw box around outside of Window, must have 1 chracter gap from edge
void box(WINDOW* w, int a, int b){
  // Only refresh box when required
  if (w->refreshBox){
    ncMove(w->wrow-1, w->wcol-1);
    PrintChr('\xF5'); // Top Left
    int r,c;
    for (c = 0; c < (w->wwidth); c++) {
      ncMove(w->wrow-1, w->wcol+c);
      WaitForScreenBlank();
      PrintChr('\xF3');  //Top
    }
    PrintChr('\xF6'); // Top Right
    for (r = 0; r < (w->wheight); r++) {
      ncMove(w->wrow+r, w->wcol-1);
      WaitForScreenBlank();
      PrintChr('\xF0'); // Side Left
      ncMove(w->wrow+r, w->wcol+w->wwidth);
      PrintChr('\xF1'); // Side Right
    }
    ncMove(w->wrow+w->wheight, w->wcol-1);
    PrintChr('\xF7'); // Low Left
    for (c = 0; c < (w->wwidth); c++) {
      WaitForScreenBlank();
      ncMove(w->wrow+w->wheight, w->wcol+c);
      PrintChr('\xF2');  //Low
    }
    PrintChr('\xF8'); // Low Right
    w->refreshBox=false;
  }
  
}
// Move the window row/col and also sync the Nanocomp row/col
void wmove(WINDOW* w, int wrow, int wcol){
  w->disprow=wrow;
  w->dispcol=wcol;
  // Move the current Nanocomp display position as well, note if do wmove for another window before PrintChr then will be wrong
  ncMove(w->wrow+wrow, w->wcol+wcol);
}

// Clear the specified window with spaces
void wclear(WINDOW* w){
  int r, c;
  for (r = 0; r < (w->wheight); r++) {
    WaitForScreenBlank();
    for (c = 0; c < (w->wwidth);) {
      ncMove(w->wrow+r, w->wcol+c);
      ClearBlock();
      c=c+2;
    }
  }
  w->disprow=0; // set the current row/col back to 0/0
  w->dispcol=0;
}

// Note does not handle variable args, hard coded to 1
void wprintw(WINDOW* w, char* outString, int arg1){
  // Move the current Nanocomp display position as well, note if do wmove for another window before PrintChr then will be wrong
  ncMove(w->wrow+w->disprow,w->wcol+w->dispcol);
  printf(outString, arg1);
  // Move to start of next line in Window by default. Start at top if at end of Window
  if (w->disprow==w->wheight){
    w->disprow=0;
  } else {
    w->disprow=w->disprow+1;
  }
  w->dispcol=0;
}

/* newwin creates Window similar to NCURSES so can be used for board, score, hold and next */
WINDOW* newwin(int num_lines, int num_columns, int begy, int begx){

  WINDOW *w = (WINDOW*)sbrk(sizeof(WINDOW));
  w->wrow=begy;
  w->wcol=begx;
  w->wheight=num_lines;
  w->wwidth=num_columns;
  w->disprow=0;
  w->dispcol=0;
  w->refreshBox=true;
  return w;
}

/* Combine 4 bit Foreground and Background Colours into 1 byte */
byte init_pair(int foreground, int background){
    byte colourByte;
    colourByte = (byte)foreground << 4;
    colourByte += (byte) background;
    //printf("Foregound %d Background %d ",foreground, background);
    //printf("colour byte %d",colourByte);
    return colourByte;
}
