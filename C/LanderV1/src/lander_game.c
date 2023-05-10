//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Nanocomp 6809 Lunar Lander
// 
// cd /mnt/c/6809/code/Lander
// cmoc --void-target --org=0000 --data=7200 --limit=7f80 -i --intdir=int --srec lander_game.c nanocomp.c lander.c landscape.c
// --limit is required to warn if hitting the top of memory for stack
// Copied from Lander12 merged with C'fied Javascript from Javascript game
// Lander11 Optimise code for address paging to avoid long - skipped array optimisation in 10
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "lander_game.h"

#include "cmoc.h"
#include "cmocextra.h"
#include "nanocomp.h"
#include "lander.h"
#include "landscape.h"

#include "landerraster.h"

// Key pad definitions until we implement joystick
const char leftKey = '1'; 
const char thrustKey = '2'; 
const char rightKey = '3'; 
const char startKey = 'S';
const char selectKey = 'I'; 
const char abortKey = '8';
const char exitKey = 'N';
const char fuelKey = 'F';

const char startMessage[] = "INSERT COINS S TO PLAY 1,2,3 KEYS TO MOVE";

// game state
game_state	gameState;
// Global lander game variables
lander_game_type *lg;
// Global lander craft variables
lander_type *lander;

bool exit_game;

void checkKeys() { 

  // May have to tune this argument for speed vs responsiveness until Joystick implemented
  byte key = PollChr(50); // was 500 at first
  // Returns -1 if nothing pressed, clear thrust
  if (key != (byte) 0xff){
	  //printf("Key=%c\n",key);
    if (key == leftKey) {
      lander_rotate(-1);	
    } else if(key == rightKey) {	
      lander_rotate(1); 
    }
    if(key == abortKey) { 
      lander_abort();
    }

    if(key == thrustKey){
      lander_thrust(5); // Make this 5 so we don't reduce in steps of 0.2 in lander_update
    }
    if (key == exitKey){  // CN Key returns N ASCII for CN
      exit_game = true;
    }
    if (key == fuelKey){  // F adds Fuel/credit
      lander->fuel += 500;
    }

    // Start
    if(key == 'S') { 
      // Wait until S is released
      while (PollChr(10)=='S');
      gameState = PLAYING;
   	  //printf("Key S calling newGame()\n");
      restartLevel();
      newGame();
    }

  } else {
    lander_thrust(0);
 	  // printf("checkKeys no key\n");
  }
}

	
void loop() {
	// Counter is effectively the game clock as we have no Timer in Nanocomp (unless use VS interupt for 1/60th secs)	
  // printf("loop counter=%d, Gamestate=%d\n",lg->counter, gameState);

  // If Game Over Do clearscreen and landscape and restartLevel
  if (gameState==GAMEOVER){
    ClearScreenGraphics();
    landscape(1);
    lg->textLabels = true; // Make sure labels displayed for info
    restartLevel();
  }
  
	if(gameState == WAITING || gameState == PLAYING) { 
		checkKeys(); 
	}

	lander_update(); 
	
  // Restarts title screen waiting for Start
	if((gameState == WAITING) && (lander->altitude > 0) && (lander->altitude < 50) ) {
	  gameState=GAMEOVER;
  }

	if((gameState== PLAYING) || (gameState == WAITING)){
		checkCollisions(); 
  } 	
	
	updateView(); 
	render(); 
  lg->counter++; 

}

// Orgiginal Javascript Render redraws screen for every update, 6809 not fast enough to do this
void render() { 
  // This is why view.x and view.y are -ve, Javascript 2D context (c) applies translate and scale
	//c.translate(view.x, view.y); 
	//c.scale(view.scale, view.scale); 
	// landscape_render(1);
	lander_render();

//	if((lg->counter == 0) || (lg->counter%4 == 0 && gameState == PLAYING)) updateTextInfo(); 
	if (lg->counter%4 == 0 || lg->textLabels) updateTextInfo(); 
	
}



void updateView() 
{
  bool zoomOff = false;
	if((!lg->zoomedIn) && (lander->altitude < 70)) {
		setZoom(true);
  	} else if((lg->zoomedIn) && (lander->altitude > 160)) {
		setZoom(false);
    zoomOff = true;	
	}
  // Save old view needed for Zoomed Large Lander to remove old Lander correctly
	lg->oldView.x = lg->view.x;
  lg->oldView.y = lg->view.y;
  lg->oldView.scale = lg->view.scale;
  lg->oldView.left = lg->view.left;
  lg->oldView.right = lg->view.right;
  lg->oldView.top = lg->view.top;
  lg->oldView.bottom = lg->view.bottom;
 /* printf("updateView start  oldview.x=%d, oldview.y=%d, view.x=%d, view.y=%d \n",lg->oldView.x,
										lg->oldView.y,
										lg->view.x,
										lg->view.y);*/

	//Zoomed scale=4  or << 2 otherwise =1
	// View x and y when zoomed are x 4, left, right, top, bottom are not scaled
	if (lg->zoomedIn){
    if(((lander->posx <<2) + lg->view.x < MARGINX)){
       //printf("updateView 1 view.x=%d, newview.x=%d \n",lg->view.x,-(lander->posx <<2) + MARGINX);
      lg->view.x = -(lander->posx <<2) + MARGINX << 1;
      lg->view.changed = true;
    } else if (((lander->posx <<2) + lg->view.x > SCREEN_WIDTH_MINUS_MARGINX)) {
      //printf("updateView 2 view.x=%d, newview.x=%d \n",lg->view.x,-(lander->posx <<2) + SCREEN_WIDTH_MINUS_MARGINX);
      lg->view.x = -(lander->posx <<2) + SCREEN_WIDTH_MINUS_MARGINX - MARGINX;
      lg->view.changed = true;
    }
    
    if((lander->posy <<2) + lg->view.y < MARGINTOP) {
      lg->view.y = -(lander->posy <<2) + 2 * MARGINTOP;
      lg->view.changed = true;
    } else if ((lander->posy<<2) + lg->view.y > SCREEN_HEIGHT_MINUS_MARGINBOTM) {
      lg->view.y = -(lander->posy <<2) + SCREEN_HEIGHT_MINUS_MARGINBOTM - (MARGINBOTTOM << 1);
      lg->view.changed = true;
    }
    //if (lg->view.changed){
      lg->view.left = -(lg->view.x >> 2); 
      lg->view.top = -(lg->view.y >> 2);
      lg->view.right = lg->view.left + 159; // 0 indexed 640/4
      lg->view.bottom = lg->view.top + 119;  // 0 indexed 480/4
    //}
    /*printf("updateView end  changed=%d, oldview.x=%d, oldview.y=%d, view.x=%d, view.y=%d \n",lg->view.changed,lg->oldView.x,
										lg->oldView.y,
										lg->view.x,
										lg->view.y); */

	
  } else {
    //zoomedin false, scale=1
    lg->view.left = 0; 
    lg->view.top = 0;
    lg->view.right = SCREEN_WIDTH_0; 
	  lg->view.bottom = SCREEN_HEIGHT_0; 
    lg->view.changed = true;
  }
  // Need to draw landscapes after view has been updated for change in Zoom, 
  // When zoomed if the view changes need to redraw landscape etc

  if (lg->zoomedIn && lg->view.changed){
    ClearScreenGraphics();
    landscapeZoom(1);
    lg->textLabels = true; 
    lg->view.changed = false;  // Stop from updating next turn
  }

  if (zoomOff){
    zoomOff = false;
    ClearScreenGraphics();
    landscape(1);
  }
}



void setLanded(int line) { 
	
	int multiplier = landscape_getMultiplier(line); 

	lander_land(); 
  int x = 18;
  int y = 11;
  char sprintfmsg[60];

	int points = 0; 
	if(lander->vely < 500) { 
		points = 50 * multiplier; 
		// show message - "a perfect landing";
    //printf("a perfect landing \n"); 
    sprintf(sprintfmsg, "CONGRATULATIONS  A PERFECT LANDING %d POINTS", points);
    DrawMessage(x, y, sprintfmsg)	;
		lander->fuel+=50;
	} else {
    //printf("HOPELESSLY MAROONED \n"); 

		points = 15 * multiplier; 
		// YOU LANDED HARD
    sprintf(sprintfmsg, "YOU LANDED HARD YOU ARE HOPELESSLY MAROONED %d POINTS", points);
    DrawMessage(x, y, sprintfmsg)	;

		lander_makeBounce(); 
	}
	
	lg->score += points; 

	gameState = LANDED; 
  Delay(10000);
  ClearMessage(x, y, sprintfmsg)	;
  if (lander->scale){
    largeLanderRasterMove(lander->oldposx, lander->oldposy, lander->oldRotation,
                          0, 0, 5);
  } else {
    smallLanderRasterMove(lander->oldposx, lander->oldposy, lander->oldRotation,
                          0, 0, 5);
  }
  restartLevel();
  
}

void setCrashed() { 
char msg[60]; 

if (gameState!=WAITING){
  
    lander_crash(); 
    
    // show crashed message
    // subtract fuel
    // rand() gives number between 0 and 32767 so divide by 163 gives 0 - 200
    int fuellost = ((rand() / 164) + 200);
    lander->fuel -= fuellost;

    if(lander->fuel < 1) { 
      setGameOver();
      strncpy(msg, "OUT OF FUEL GAME OVER", 60);
    } else {
      int rnd  = rand();
      strncpy(msg, "                                                      ", 60);
      if(rnd < 10000){
        strncpy(msg, "YOU JUST DESTROYED A 100 MEGABUCK LANDER", 60);
      } else  if(rnd < 20000){
        strncpy(msg, "DESTROYED", 60);
      } else {
        strncpy(msg, "YOU CREATED A TWO MILE CRATER", 60);
      }
      
      // msg = "AUXILIARY FUEL TANKS DESTROYED " + fuellost + " FUEL UNITS LOST" + msg;
      
      gameState = CRASHED;
    
    }
    // Don't display message when waiting for game to start
      DrawMessage(20, 13, msg);
      Delay(10000);
      ClearMessage(20, 13, msg);
      if (lander->scale){
        largeLanderRasterMove(lander->oldposx, lander->oldposy, lander->oldRotation,
                              0, 0, 5);
      } else {
        smallLanderRasterMove(lander->oldposx, lander->oldposy, lander->oldRotation,
                              0, 0, 5);
      }

  }
  setGameOver();
	
	//samples.explosion.play(); 
}


void setGameOver() { 
		gameState = GAMEOVER; 
}

void newGame() { 
	lander->fuel = 1000;

	lg->time = 0;
	lg->score = 0;
	
	//gameStartTime = Date.now(); 
}

void restartLevel() { 
	lg->counter = 0; 
  //printf("restartLevel gameState=%d \n",gameState);

	if(gameState==GAMEOVER) { 
		gameState = WAITING; 
		showStartMessage(); 
	} else {
		gameState = PLAYING; 
		clearStartMessage();
    //printf("Return from ClearStartMessage Scale=%d", lander->scale);
    if (lander->scale){
      largeLanderRasterMove(lander->posx, lander->posy, lander->rotation,
                0, 0, 0);
    } else {
      //printf("Clear Small Lander x=%d, y=%d, rot=%d \n",lander->posx, lander->posy, lander->rotation);
      smallLanderRasterMove(lander->posx, lander->posy, lander->rotation,
                0, 0, 0);
    }
	}
	lander_reset(); 
	lander->velx = 2000; //2000 
	landscape_setZones(); 
	setZoom(false); 
}

void checkCollisions() {
  // Check for point i to i+n
  int iLeft = xToPoint[lander->left];
  int iRight = xToPoint[lander->right]; 
  if (iLeft == iRight){
    iRight++; // If the same then take next point to right
    printf("**** iLeft == iRight so iRight++");
  }

	lander->altitude = ((landscapeY[iLeft] + landscapeY[iRight] )>> 1) - lander->bottom; // Take average of left & right Y

  printf("checkCollisions Left=%d, iLeft=%d, Right=%d, iRight=%d, Altitude=%d \n",lander->left,iLeft,lander->right,iRight, lander->altitude);
  printf("checkCollisions landscapeY[iLeft]=%d, landscapeY[iRight]=%d, lander->bottom=%d \n",landscapeY[iLeft],landscapeY[iRight], lander->bottom );

	// if the line's horizontal 
	if(landscapeY[iLeft] == landscapeY[iRight]) { 
		// and the lander's bottom is overlapping the line
		if(lander->bottom >= landscapeY[iLeft]) { 
			printf("Lander overlapping ground landerBottom=%d, iLeft=%d, landscapeY=%d, landscapeXL=%d, iRight=%d, landscapeXR=%d \n", lander->bottom, iLeft, landscapeY[iLeft], landscapeX[iLeft], iRight, landscapeX[iRight]); 
  		printf("Lander overlapping ground rotation=%d, VelY=%ld \n", lander->rotation, lander->vely);
			// and the lander is completely within the line
			if((lander->left >= landscapeX[iLeft]) && (lander->right <= landscapeX[iRight])) {
				printf("Lander within line, rotation=%d, VelY=%ld \n", lander->rotation, lander->vely);
				// and we're horizontal and moving slowly
				if((lander->rotation == 5) && (lander->vely < 1500)) {
					    printf("horizontal and slow < 1500 \n");
							setLanded(iLeft);
						} else {
					    printf("Crashed not horizontal or too fast \n");
							setCrashed(); 
						} 
					} else {
						// if we're not within the line
				    printf("Crashed not with in horizontal line \n");
						setCrashed(); 
					}
				}
		// Not Horizontal - if lander's bottom is below either of the two y positions
  } else if(( lander->bottom > landscapeY[iLeft]) || (lander->bottom > landscapeY[iRight])) {
    if (( lander->bottom > landscapeY[iLeft]) && (lander->bottom > landscapeY[iRight])){
	    printf("Both Y less than line  \n");
      setCrashed(); 
    } else if( pointIsLessThanLine(lander->bottomLeftx, lander->bottomLefty, iLeft, iRight) || 	
               pointIsLessThanLine(lander->bottomRightx, lander->bottomRighty, iLeft, iRight)) {
  	    printf("Not horizontal pointIsLessThanLine  \n");
        setCrashed(); 
    }
  }
};


bool pointIsLessThanLine(int pointx, int pointy, int linepoint1, int linepoint2) {

	// so where is the y of the line at the point of the corner? 
	// first of all find out how far along the xaxis the point is
  // Multiply dist by 100 since we do not have fp and + 50 to round up
  printf("pointIsLessThanLine pointx=%d, pointy=%d, linepoint1=%d, linepoint2=%d \n ", pointx, pointy, linepoint1, linepoint2);
	int dist = (pointx - landscapeX[linepoint1]) * 100 + 50 / ((landscapeX[linepoint2]) - landscapeX[linepoint1]);
  printf("pointIsLessThanLine landscapeX[linepoint1]=%d, landscapeX[linepoint2]=%d, landscapeY[linepoint1]=%d, landscapeY[linepoint2]=%d \n ", landscapeX[linepoint1], landscapeX[linepoint2],landscapeY[linepoint1],landscapeY[linepoint2]);
	int yhitpoint  = landscapeY[linepoint1] + ((landscapeY[linepoint2] - landscapeY[linepoint1]) * dist) / 100;
  printf("pointIsLessThanLine dist=%d, yhitpoint=%d, pointy=%d \n ", dist, yhitpoint, pointy);
	return ((dist > 0) && (yhitpoint <= pointy) && (dist < 100)) ; // did have && (dist<1) as well
 }


void updateTextInfo() {

  char sprintfmsg[30];
  int x = 5;
  int y = 1;
  if (lg->textLabels){
    sprintf(sprintfmsg, "SCORE %5d ", lg->score);
    DrawMessage(x, y++, sprintfmsg)	;
    sprintf(sprintfmsg, "FUEL  %5d ", lander->fuel);
    DrawMessage(x, y++, sprintfmsg)	;
    sprintf(sprintfmsg, "TIME  %5d ", lg->counter);
    DrawMessage(x, y++, sprintfmsg)	;
    x = 55;
    y = 1;
    sprintf(sprintfmsg, "ALTITUDE         %5d ", lander->altitude);
    DrawMessage(x, y++, sprintfmsg)	;
    sprintf(sprintfmsg, "HORIZONTAL SPEED %5ld%s",  (lander->velx), (lander->velx < 0) ? "<-":"->");  // Note these arrows are redefined in Lander Font
    DrawMessage(x, y++, sprintfmsg)	;
    sprintf(sprintfmsg, "VERTICAL SPEED   %5ld%s", (lander->vely), (lander->vely < 0) ? "^":"_");
    DrawMessage(x, y++, sprintfmsg);
    lg->textLabels = false;
  } else {
    x = 11;
    y = 1;
    sprintf(sprintfmsg, "%5d", lg->score);
    DrawMessage(x, y++, sprintfmsg)	;
    sprintf(sprintfmsg, "%5d", lander->fuel);
    DrawMessage(x, y++, sprintfmsg)	;
  	if(gameState == PLAYING) {
      sprintf(sprintfmsg, "%5d", lg->counter);
      DrawMessage(x, y++, sprintfmsg)	;
    }
    x = 72;
    y = 1;
    sprintf(sprintfmsg, "%5d", lander->altitude);
    DrawMessage(x, y++, sprintfmsg)	;
    sprintf(sprintfmsg, "%5ld%s",  (lander->velx), (lander->velx < 0) ? "<-":"->");  // Note these arrows are redefined in Lander Font
    DrawMessage(x, y++, sprintfmsg)	;
    sprintf(sprintfmsg, "%5ld%s", (lander->vely), (lander->vely < 0) ? "^":"_");
    DrawMessage(x, y++, sprintfmsg);
  }


	if((lander->fuel < 300) && (gameState == PLAYING)) {
    x = 30;
    y = 4;
		if((lg->counter%50)<30) { 
			if(lander->fuel <= 0) {
				DrawMessage(x, y, (char *) "OUT OF FUEL"); 
			} else {
				DrawMessage(x, y, (char *) "LOW ON FUEL");
			} 
		} else {
		    ClearMessage(x, y, (char *) "LOW ON FUEL");
		}
	}

}

void showStartMessage() {
	DrawMessage(20,6, startMessage);
}

void clearStartMessage() {
	ClearMessage(20,6, startMessage);
}

void setZoom(bool zoom ) 
{
	if(zoom){
		lg->view.scale = 4;				
		lg->zoomedIn = true;
		lg->view.x = -lander->posx * lg->view.scale + (SCREEN_WIDTH / 2);
		lg->view.y = -lander->posy * lg->view.scale + (SCREEN_HEIGHT / 4);
		lander->scale = true; // Large Lander True
    lg->textLabels = true;
	} else {
		lg->view.scale = 1;
		lg->zoomedIn = false;
		lander->scale = false; // Small Lander False
		lg->view.x = 0;
		lg->view.y = 0;
    lg->textLabels = true; 
	}

	
}

// returns a random number between the two limits provided 
// CMOC rand returns 0-32767 15 bits
int  randomRange(int min, int max){
  long temp = rand() * (max-min);
  temp = (temp >> 15) + min ; // rotate right 15 bits == divide by 32768 but faster
	return ((int) temp); 
}

// Calculates for each Large and Small Lander what the blank start and end rows are for optimization
void landerArrayInit(){
/*  int startNonZero;
  int endNonZero;
  bool stillZero = true;

  for (int rot=0; rot <=10; rot++){
    for (int offset=0; offset <=7; offset++){

        startNonZero = 0;
        endNonZero = 23;
        stillZero = true;

      for (int i=0; i <=23; i++){
        if (LanderRasterSmall[rot][offset][i] == 0) {
          // Once we find first non zero the this will not be true
          if (stillZero){
            startNonZero = i;
          }
        } else {
          // This will get updated to the last non zero index
          endNonZero = i;
          stillZero = false;
        }
      }
      LanderIndexSmall[rot][offset][0][0] = (startNonZero + 1);  // Need to start at first non zero
      LanderIndexSmall[rot][offset][0][1] = endNonZero; 
      //printf("Small Rot=%d, Offset=%d, 0 (0-23), Start=%d, End=%d\n", rot, offset, (startNonZero + 1), endNonZero);

      stillZero = true;
      startNonZero = 24;
      endNonZero = 47;
      
      for (int i=24; i <=47; i++){
        if (LanderRasterSmall[rot][offset][i] == 0) {
          // Once we find first non zero the this will not be true
          if (stillZero){
            startNonZero = i;
          }
        } else {
          // This will get updated to the last non zero index
          endNonZero = i;
          stillZero = false;
        }
      }
      LanderIndexSmall[rot][offset][1][0] = startNonZero + 1;  // Need to start at first non zero
      LanderIndexSmall[rot][offset][1][1] = endNonZero; 

      //printf("Small Rot=%d, Offset=%d, 1 (24-47), Start=%d, End=%d\n", rot, offset, (startNonZero + 1), endNonZero);
    }
  }
  
  // Now do Large
  startNonZero = 0;
  endNonZero = 31;
  stillZero = true;
  
  for (int rot=0; rot <=10; rot++){
    for (int offset=0; offset <=7; offset++){

      startNonZero = 0;
      endNonZero = 31;
      stillZero = true;

      for (int i=0; i <=31; i++){
        if (LanderRasterLarge[rot][offset][i] == 0) {
          // Once we find first non zero the this will not be true
          if (stillZero){
            startNonZero = i;
          }
        } else {
          // This will get updated to the last non zero index
          endNonZero = i;
          stillZero = false;
        }
      }
      LanderIndexLarge[rot][offset][0][0] = (startNonZero + 1);  // Need to start at first non zero
      LanderIndexLarge[rot][offset][0][1] = endNonZero; 
      //printf("Large Rot=%d, Offset=%d, 0 (0-31), Start=%d, End=%d\n", rot, offset, (startNonZero + 1), endNonZero);

      stillZero = true;
      startNonZero = 32;
      endNonZero = 63;
      
      for (int i=32; i <=63; i++){
        if (LanderRasterLarge[rot][offset][i] == 0) {
          // Once we find first non zero the this will not be true
          if (stillZero){
            startNonZero = i;
          }
        } else {
          // This will get updated to the last non zero index
          endNonZero = i;
          stillZero = false;
        }
      }
      LanderIndexLarge[rot][offset][1][0] = startNonZero + 1;  // Need to start at first non zero
      LanderIndexLarge[rot][offset][1][1] = endNonZero; 

      //printf("Large Rot=%d, Offset=%d, 1 (32-63), Start=%d, End=%d\n", rot, offset, (startNonZero + 1), endNonZero);

      stillZero = true;
      startNonZero = 64;
      endNonZero = 95;
      
      for (int i=64; i <=95; i++){
        if (LanderRasterLarge[rot][offset][i] == 0) {
          // Once we find first non zero the this will not be true
          if (stillZero){
            startNonZero = i;
          }
        } else {
          // This will get updated to the last non zero index
          endNonZero = i;
          stillZero = false;
        }
      }
      LanderIndexLarge[rot][offset][2][0] = startNonZero + 1;  // Need to start at first non zero
      LanderIndexLarge[rot][offset][2][1] = endNonZero; 

      //printf("Large Rot=%d, Offset=%d, 2 (64-95), Start=%d, End=%d\n", rot, offset, (startNonZero + 1), endNonZero);
    }
  }
*/
}

void moveLargeBlock(int pixelBit,int realx, int realy, int rot, int yCharRow, int istart, int iend, int ioffset, bool old)
{
  word address;
  byte *vramaddress;
  byte rasterData;

  int yStartLine = 0;
  int yLinesInRow = 0;
  bool topRow = true;
  
  
  // Address of first line, can increment until change of 8 line character row
  yStartLine = realy  & 7; // This is the start line of row in 8 line Char Row 0-7
  yLinesInRow = 8 - yStartLine;  // This shows how many rows can just increment address rather than full calc when 0 need new calc
  address = (realx + yStartLine + yLookup[yCharRow]);
  vramaddress = (byte *) rawAddressToPaged(address);

  //printf("Init Address %X, pixelBit=%X, yCharRow=%X, yStartLine=%d, yLinesInRow=%d,  \n\r", vramaddress, pixelBit, yCharRow, yStartLine, yLinesInRow);

  // These two variables 0,23 will come from LanderIndexSmall to optimize lines processed
  // We are processing 1 row of bytes from Lander but may split over 2 Character Rows
  for (int i=istart; i <=iend; i++){
    if (yLinesInRow <= 0 ){
      if ((yStartLine<=0) || (topRow==false)) {
        // This is the case when the Lander and Row match up 8 to 8
        address = (realx + ((i - ioffset) & 0x18) + yStartLine + yLookup[yCharRow]);
        vramaddress = (byte *) rawAddressToPaged(address);
        yLinesInRow = 8-yStartLine; // Start next block to right on same row
        topRow = !topRow;  // Flip Top row, if on top row wil indicate lower row, if lower row will indicate top
        //printf("topRow=%d, yStartLine=0 i=%d, yLookup=%X, address=%X, yLinesInRow=%d, yStartLine=%d, (i & 0x18)=%d \n", topRow, i, yLookup[yCharRow],address,yLinesInRow,yStartLine, (i & 0x18));
      } else {
        // This is the case when the Lander and Row don't match up 8 7+1, 6+2,7+3,4+4 etc across row and next row
        yLinesInRow = yStartLine;  // Need to do the remaining lines in next row
        topRow = false; // Need to set this to tha when yLinesInRow = 0 we can move to next block on right
        // The i & 0x18 will mask the lower 3 bits and add 8 or 16 depending on i to move to second and third colummn
        // No startline for continuation from above
        address = (realx + ((i - ioffset) & 0x18) + yLookup[yCharRow+1]);
        //printf("topRow=%d, yStartLine!=0 i=%d, yLookup=%X, address=%X, yLinesInRow=%d, yStartLine=%d, (i & 0x18)=%d \n", topRow, i, yLookup[yCharRow+1],address,yLinesInRow, yStartLine, (i & 0x18));
        vramaddress = (byte *) rawAddressToPaged(address);
      }
    } 
    // This will set bits for the new lander
    rasterData = LanderRasterLarge[rot][pixelBit][i];
    if (old){
      *vramaddress = *vramaddress & ~rasterData;
    } else {
      *vramaddress = *vramaddress | rasterData;
    }
    //printf("R1 i=%d, yLinesInRow=%d (before dec), Address %X, data=%X \n\r", i, yLinesInRow, vramaddress, rasterData);
    // Can increment vramaddress until next 8 line character row
    vramaddress++;
    yLinesInRow--; // When this is 0 need to calc address and  address calc with yCharRow+1

  }
}

void largeLanderRasterMove(int oldx, int oldy, int oldrot, int x, int y, int rot){

  int xrasteroffset = 12;
  int yrasteroffset = 12;
  int yCharRow;

  /* Raster lander drawn at   int x=6;int y=7; for raster capture */
  if (oldrot > 10){
    oldrot = 10;
  }
  if (rot > 10){
    rot = 10;
  }

  int pixelBit = (x - xrasteroffset) & 7;        // The bit from left to right 0 left 7 right
  int oldPixelBit = (oldx - xrasteroffset) & 7;  // The bit from left to right 0 left 7 right

  // By subtracting pixelBit the x values will increase in multiples of 8
  // For x=6 pixelBit=0 so raster offset=0 for example x=8 pixelBit=2 so need offset=2
  // Real Y is the top of the Lander raster image, and is 16 lines (0-15)
  int realy = oldy - yrasteroffset;
  int realx = oldx - xrasteroffset - oldPixelBit; // this is the X Column in units of 8, 0=0, 1=8, 2=16
  
  // Only remove previous lander position if old coords are > 0
  if (oldx > 0 && oldy > 0){
    //printf("Lander old centre x=%d, y=%d\n", oldx, oldy);
    //printf("Top Left realx=%d, realy=%d\n", realx, realy);
    yCharRow = realy >> 3;
    moveLargeBlock(oldPixelBit, realx, realy, oldrot, yCharRow, 0, 31, 0, true);
    yCharRow++;
    moveLargeBlock(oldPixelBit, realx, realy, oldrot, yCharRow, 32, 63, 32, true);
    yCharRow++;
    moveLargeBlock(oldPixelBit, realx, realy, oldrot, yCharRow, 64, 95, 64, true);
  }

  if ( x > 0 && y > 0){
    realy = y - yrasteroffset;
    realx = x - xrasteroffset - pixelBit;

    yCharRow = realy >> 3;
    moveLargeBlock(pixelBit, realx, realy, rot, yCharRow, 0, 31, 0, false);
    yCharRow++;
    moveLargeBlock(pixelBit, realx, realy, rot, yCharRow, 32, 63, 32, false);
    yCharRow++;
    moveLargeBlock(pixelBit, realx, realy, rot, yCharRow, 64, 95, 64, false);
  }

}

void moveSmallBlock(int pixelBit,int realx, int realy, int rot, int yCharRow, int istart, int iend, int ioffset, bool old)
{
  word address;
  byte *vramaddress;
  byte rasterData;

  int yStartLine = 0;
  int yLinesInRow = 0;
  bool topRow = true;
  
  
  // Address of first line, can increment until change of 8 line character row
  yStartLine = realy  & 7; // This is the start line of row in 8 line Char Row 0-7
  yLinesInRow = 8 - yStartLine;  // This shows how many rows can just increment address rather than full calc when 0 need new calc
  address = (realx + yStartLine + yLookup[yCharRow]);
  vramaddress = (byte *) rawAddressToPaged(address);

  //printf("Init 0-23 Address %X, pixelBit=%X, yCharRow=%X, yStartLine=%d, yLinesInRow=%d,  \n\r", vramaddress, pixelBit, yCharRow, yStartLine, yLinesInRow);

  // These two variables 0,23 will come from LanderIndexSmall to optimize lines processed
  // We are processing 1 row of bytes from Lander but may split over 2 Character Rows
  for (int i=istart; i <=iend; i++){
    if (yLinesInRow <= 0 ){
      if ((yStartLine<=0) || (topRow==false)) {
        // This is the case when the Lander and Row match up 8 to 8
        address = (realx + ((i - ioffset) & 0x18) + yStartLine + yLookup[yCharRow]);
        vramaddress = (byte *) rawAddressToPaged(address);
        yLinesInRow = 8-yStartLine; // Start next block to right on same row
        topRow = !topRow;  // Flip Top row, if on top row wil indicate lower row, if lower row will indicate top
        //printf("topRow=%d, yStartLine=0 i=%d, yLookup=%X, address=%X, yLinesInRow=%d, yStartLine=%d, (i & 0x18)=%d \n", topRow, i, yLookup[yCharRow],address,yLinesInRow,yStartLine, (i & 0x18));
      } else {
        // This is the case when the Lander and Row don't match up 8 7+1, 6+2,7+3,4+4 etc across row and next row
        yLinesInRow = yStartLine;  // Need to do the remaining lines in next row
        topRow = false; // Need to set this to tha when yLinesInRow = 0 we can move to next block on right
        // The i & 0x18 will mask the lower 3 bits and add 8 or 16 depending on i to move to second and third colummn
        // No startline for continuation from above
        address = (realx + ((i - ioffset) & 0x18) + yLookup[yCharRow+1]);
        //printf("topRow=%d, yStartLine!=0 i=%d, yLookup=%X, address=%X, yLinesInRow=%d, yStartLine=%d, (i & 0x18)=%d \n", topRow, i, yLookup[yCharRow+1],address,yLinesInRow, yStartLine, (i & 0x18));
        vramaddress = (byte *) rawAddressToPaged(address);
      }
    } 
    // This will set bits for the new lander
    rasterData = LanderRasterSmall[rot][pixelBit][i];
    if (old){
      *vramaddress = *vramaddress & ~rasterData;
    } else {
      *vramaddress = *vramaddress | rasterData;
    }
    //printf("R1 i=%d, yLinesInRow=%d (before dec), Address %X, data=%X \n\r", i, yLinesInRow, vramaddress, rasterData);
    // Can increment vramaddress until next 8 line character row
    vramaddress++;
    yLinesInRow--; // When this is 0 need to calc address and  address calc with yCharRow+1

  }
}


void smallLanderRasterMove(int oldx, int oldy, int oldrot, int x, int y, int rot) {

  int xrasteroffset = 6;
  int yrasteroffset = 7;
  int yCharRow;

  /* Raster lander drawn at   int x=6;int y=7; for raster capture */
  if (oldrot > 10){
    oldrot = 10;
  }
  if (rot > 10){
    rot = 10;
  }

  int pixelBit = (x - xrasteroffset) & 7;        // The bit from left to right 0 left 7 right
  int oldPixelBit = (oldx - xrasteroffset) & 7;  // The bit from left to right 0 left 7 right

  // By subtracting pixelBit the x values will increase in multiples of 8
  // For x=6 pixelBit=0 so raster offset=0 for example x=8 pixelBit=2 so need offset=2
  // Real Y is the top of the Lander raster image, and is 16 lines (0-15)
  int realy = oldy - yrasteroffset;
  int realx = oldx - xrasteroffset - oldPixelBit; // this is the X Column in units of 8, 0=0, 1=8, 2=16
  
  // Only remove previous lander position if old coords are > 0
  if (oldx > 0 && oldy > 0){
    //printf("Lander old centre x=%d, y=%d\n", oldx, oldy);
    //printf("Top Left realx=%d, realy=%d\n", realx, realy);
    yCharRow = realy >> 3;
    moveSmallBlock(oldPixelBit, realx, realy, oldrot, yCharRow, 0, 23, 0, true);
    yCharRow++;
    moveSmallBlock(oldPixelBit, realx, realy, oldrot, yCharRow, 24, 47, 24, true);
  }

  if ( x > 0 && y > 0){
    realy = y - yrasteroffset;
    realx = x - xrasteroffset - pixelBit;

    yCharRow = realy >> 3;
    moveSmallBlock(pixelBit, realx, realy, rot, yCharRow, 0, 23, 0, false);
    yCharRow++;
    moveSmallBlock(pixelBit, realx, realy, rot, yCharRow, 24, 47, 24, false);
  }

}  



int main(int argc, char **argv)
{
  // Enables debug output from printf() to serial port
  setConsoleOutHook(serialOutputRoutine);
  printf("Test Serial Debug \n\r");
  graphicsInit();
  exit_game = false;

  // Initialises PIA so we can check Screen Blanking
  //WaitForScreenBlankInit();
  
  // Create Lander Game which includes most global variables
  // We won't pass this between functions, use a global to avoid overhead
  lg = lg_create();

  // Create Lander which includes all craft related variables
  // We won't pass this between functions, use a global to avoid overhead
  lander = lander_create();
  setGameOver();
  landscape_init(); 

  while(true){
    loop();
    if (exit_game) break;
  }

  // Set VRAM to page 0 for debugging
 *pageRegister = (byte)(GMODE2);

  returnNC();  // Causes SWI for Nanocomp to return to monitor
  return 0;
}
