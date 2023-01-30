/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan

  @date         Created Wednesday, 10 June 2015

  @brief        Main program for tetris.

  @copyright    Copyright (c) 2015, Stephen Brennan.  Released under the Revised
                BSD License.  See LICENSE.txt for details.

                Modified by Dave Henry for Nanocomp 6809 VGA Controller
                
*******************************************************************************/

#include "cmoc.h"
#include "cmocextra.h"
#include "nanocomp.h"
#include "nccurses.h"

#include "tetris.h"
//#include "util.h"

/*
  2 columns per cell makes the game much nicer.
 */
#define COLS_PER_CELL 2
/*
  Macro to print a cell of a specific type to a window.
 */
#define ADD_BLOCK(w,x) PrintColChr(colourPair[x],'\xF9');     \
                       PrintColChr(colourPair[x],'\xFA')
#define ADD_EMPTY(w) PrintChr(' '); PrintChr(' ')

// Define array for Ncurses like colour mapping via COLOR_PAIR(x)
byte colourPair[NUM_TETROMINOS+1];

/*
  Print the tetris board onto the ncurses window.
  Note the default version refreshes the whole board including box
 */
void display_board(WINDOW *w, tetris_game *obj)
{
  int i, j;
  box(w, 0, 0);
  for (i = 0; i < obj->rows; i++) {
    wmove(w, 1 + i, 1);
    for (j = 0; j < obj->cols; j++) {
      if (TC_IS_FILLED(tg_get(obj, i, j))) {
        ADD_BLOCK(w,tg_get(obj, i, j));
      } else {
        ADD_EMPTY(w);
      }
    }
  }
  //wnoutrefresh(w);
}

/*
  Display a tetris piece in a dedicated window.
*/
void display_piece(WINDOW *w, tetris_block block)
{
  int b;
  tetris_location c;
  wclear(w);
  box(w, 0, 0);
  if (block.typ == -1) {
    //wnoutrefresh(w);
    return;
  }
  for (b = 0; b < TETRIS; b++) {
    c = TETROMINOS[block.typ][block.ori][b];
    wmove(w, c.row + 1, c.col * COLS_PER_CELL + 1);
    ADD_BLOCK(w, TYPE_TO_CELL(block.typ));
  }
  //wnoutrefresh(w);
}

/*
  Display score information in a dedicated window.
 */
void display_score(WINDOW *w, tetris_game *tg)
{
  wclear(w);
  box(w, 0, 0);
  wprintw(w, "Score %d", tg->points);
  wmove(w,2,0);
  wprintw(w, "Level %d", tg->level);
  wmove(w,4,0);
  wprintw(w, "Lines %d", tg->lines_remaining);
  //wnoutrefresh(w);
}

/*
  Do the NCURSES initialization steps for color blocks.
 */
void init_colors()
{
  //start_color();
  //init_color(COLOR_ORANGE, 1000, 647, 0);
  colourPair[TC_CELLI]=init_pair(COLOR_CYAN, COLOR_BLUE);
  colourPair[TC_CELLJ]=init_pair(COLOR_LIGHT_BLUE, COLOR_BLUE);
  colourPair[TC_CELLL]=init_pair(COLOR_ORANGE, COLOR_BLUE);
  colourPair[TC_CELLO]=init_pair(COLOR_YELLOW, COLOR_BLUE);
  colourPair[TC_CELLS]=init_pair(COLOR_GREEN, COLOR_BLUE);
  colourPair[TC_CELLT]=init_pair(COLOR_MAGENTA, COLOR_BLUE);
  colourPair[TC_CELLZ]=init_pair(COLOR_RED, COLOR_BLUE);
}

/*
  Main tetris game!
 */
int main(int argc, char **argv)
{
  // Change C default output handler to use Nanocomp output routines
  setConsoleOutHook(newOutputRoutine);
  ClearScreen();      // Nanocomp Clearscreen
  CopyCharGenROM();   // Load Tetris Characters in Charater Generator RAM
  
  tetris_game *tg;
  tetris_move move = TM_NONE;
  bool running = true;
  WINDOW *board, *next, *hold, *score;

  // Create new game. We removed all the load and save logic
  tg = tg_create(22, 10);

  // NCURSES initialization:
  //initscr();             // initialize curses
  //cbreak();              // pass key presses to program, but not signals
  //noecho();              // don't echo key presses to screen
  //keypad(stdscr, TRUE);  // allow arrow keys
  //timeout(0);            // no blocking on getch()
  //curs_set(0);           // set the cursor to invisible
  init_colors();         // setup tetris colors

  // Create windows for each section of the interface.
  // Layout modified for Nanocomp
  board = newwin(tg->rows, (2 * tg->cols + 2), 2, 30);
  next  = newwin(3, 9, 2, 2 * (tg->cols + 1) + 38);
  hold  = newwin(3, 9, 2, 12);
  score = newwin(5, 9, 19, 12);
  
  // Game loop
  while (running) {
    running = tg_tick(tg, move);
    display_board(board, tg);
    display_piece(next, tg->next);
    display_piece(hold, tg->stored);
    display_score(score, tg);
    //doupdate();
    //sleep_milli(10);
    byte pollChr;
    pollChr = PollChr(5);
    
    switch (pollChr) {
    case KEY_LEFT:
      move = TM_LEFT;
      break;
    case KEY_RIGHT:
      move = TM_RIGHT;
      break;
    case KEY_UP:
      move = TM_CLOCK;
      break;
    case KEY_DOWN:
      move = TM_DROP;
      break;
    case 'N':
      running = false;
      move = TM_NONE;
      break;
    case 'S':
      wclear(board);
      box(board, 0, 0);
      wmove(board, tg->rows/2, (tg->cols*COLS_PER_CELL-6)/2);
      wprintw(board, "SUSPENDED",0); // Need 0 as arg since not implemented ... arg handling
      //wrefresh(board);
      //timeout(-1);
      GetKey();
      //timeout(0);
      move = TM_NONE;
      break;
    case '5':
      move = TM_HOLD;
      break;
    default:
      move = TM_NONE;
    }
  }

  // Deinitialize NCurses
  //wclear(stdscr);
  //endwin();


  // Output ending message.
  ncMove(28,37);
  printf("Game over!\n");
  ncMove(29,25);
  printf("You finished with %d points on level %d.\n", tg->points, tg->level);

  // Deinitialize Tetris
  tg_delete(tg);
  
  // Return to Nanocomp Monitor (no OS for return 0)
  returnNC();
  return 0;
}
