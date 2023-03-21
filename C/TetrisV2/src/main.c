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

// Define array for Ncurses like colour mapping via COLOR_PAIR(x)
byte colourPair[NUM_TETROMINOS+1];
// Counter for general loop debugging
unsigned int debug_count=0;

/*
  Print the tetris board onto the ncurses window.
  Note the default version refreshes the whole board including box
 */
void display_board(WINDOW *w, tetris_game *obj)
{
  if (obj->refresh_board){
    WaitForScreenBlank();
    int i, j;
    box(w, 0, 0);
    for (i = 0; i < obj->rows; i++) {
      WaitForScreenBlank();
      // Move to next line, note AddBlock and ClearBlock increment DISPCOL, move to next cell
      wmove(w, i, 0);
      for (j = 0; j < obj->cols; j++) {
        if (TC_IS_FILLED(tg_get(obj, i, j))) {
          AddBlock(colourPair[tg_get(obj, i, j)]);
        } else {
          ClearBlock();
        }
      }
    }
    //ncMove(26,50);
    //printf("display_board refesh %d", debug_count++);

    obj->refresh_board=false;
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
    WaitForScreenBlank();
    AddBlock(colourPair[TYPE_TO_CELL(block.typ)]);
  }
  //wnoutrefresh(w);
}

/*
  Display score information in a dedicated window.
 */
void display_score(WINDOW *w, tetris_game *tg)
{
  if (tg->refresh_score){
    wclear(w);
    box(w, 0, 0);
    WaitForScreenBlank();
    wprintw(w, "Score %d", tg->points);
    wmove(w,2,0);
    wprintw(w, "Level %d", tg->level);
    WaitForScreenBlank();
    wmove(w,4,0);
    wprintw(w, "Lines %d", tg->lines_remaining);
    tg->refresh_score=false;
  }
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
  CopyCharGenRAM();   // Load Tetris Characters in Charater Generator RAM
  DisableCursor();

  tetris_game *tg;
  tetris_move move = TM_NONE;
  bool running = true;
  WINDOW *board, *next, *hold, *score;

  ClearScreen();      // Nanocomp Clearscreen
  WaitForScreenBlankInit();  //Full Nanocomp routine initialised PIA Ports

  // This code initialises random numbers from number of times PollChr waits for S for Start
  unsigned int randSeed = 3715;
  ncMove(12,30);
  printf("Press S to Start\n");
  while (PollChr(2)==(byte) 0xff){
    randSeed++;
  }
  srand(randSeed);  // seed random numbers from count above

  ClearScreen();      // Nanocomp Clearscreen

  ncMove(9,16);
  printf("HOLD");
  ncMove(7,63);
  printf("NEXT");
  
  //ncMove(29,30);
  //WaitForScreenBlank();
  //printf("Random Seed %u", randSeed);


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
  board = newwin(tg->rows, (2 * tg->cols), 2, 30);
  next  = newwin(4, 10, 2, 2 * (tg->cols + 1) + 38);
  hold  = newwin(6, 10, 2, 13);
  score = newwin(5, 12, 19, 12);
  
  // Game loop
  while (running) {
    running = tg_tick(tg, move, board);
    display_board(board, tg);
    // Only update Next and Hold if need refresh
    if (tg->refresh_next){
      display_piece(next, tg->next);
      tg->refresh_next=false;
    }
    if (tg->refresh_hold){
      display_piece(hold, tg->stored);
      tg->refresh_hold=false;
    }
    display_score(score, tg);
    //doupdate();
    //sleep_milli(10);
    // Need to vary delay and poll count for responsiveness
    Delay(1000);
    byte pollChr;
    pollChr = PollChr(200);
    
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
      tg->refresh_board=true;

      move = TM_NONE;
      break;
    case '2':
      move = TM_HOLD;
      break;
    default:
      move = TM_NONE;
      //WaitForScreenBlank();
      //ncMove(27,60);
      //printf("Move None %d", pollChr);
    }
  }

  // Deinitialize NCurses
  //wclear(stdscr);
  //endwin();


  // Output ending message.
  ncMove(28,37);
  WaitForScreenBlank();
  printf("Game over!\n");
  ncMove(29,25);
  WaitForScreenBlank();
  printf("You finished with %d points ", tg->points);
  WaitForScreenBlank();
  printf("on level %d.\n", tg->level);

  // Deinitialize Tetris
  tg_delete(tg);
  
  // Return to Nanocomp Monitor (no OS for return 0)
  returnNC();
  return 0;
}
