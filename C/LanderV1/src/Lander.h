#include "cmoc.h"
#include "cmocextra.h"

#ifndef _H_LANDER
#define _H_LANDER


 /*
  Game State
 */
 
  typedef enum {
	WAITING, PLAYING, LANDED, CRASHED, GAMEOVER
  } game_state;

  typedef struct {
  /*
	Game View - handles Zoomed view
  */
    int x;
    int y;
    int scale;
    int left;
    int right;
    int top;
    int bottom;
	bool changed;
  } view_type;

/*
  A Lander Game object!
 */
  typedef struct {
	/*
	  Game global variables
	 */
	int score;
	int time;
	int counter;
	bool zoomedIn;
	int zoomFactor;
    int gravity; // 0.0005,
    int thrustAcceleration; // 0.0015,
	int	topSpeed; //0.35, 
    int drag;   // 0.9997 0.03% depends on how many loops per s
	view_type view;
	view_type oldView;
	int currentCombi;
	bool textLabels;
  } lander_game_type;
/*
  A Lander object!
 */
  typedef struct {
	/*
	 */
    int altitude;
    int posx;
	long deltax;  // hold fraction of x between turns to 3 places
    int posy;
	long deltay;  // hold fraction of x between turns to 3 places
    long velx;    // 
    long vely;
    int oldposx;
    int oldposy;
	bool scale; // 0=Small 1=Large 
	int top;
	int bottom;
	int left;
	int right;
	int	bottomLeftx;
	int	bottomLefty;
	int	bottomRightx;
	int	bottomRighty;
	int thrustBuild;
	bool exploding; // false, 
	int bouncing;
	int thrusting;
	int rotation;
	int oldRotation;
	int targetRotation;
	int lastRotationTime;
	int	abortCounter; // -1; 
	int	lastAbort; // -1; 
	int fuel;
	bool active; 
  } lander_type;

// screen size variables, note zero indexed versions
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const int SCREEN_WIDTH_0; // 0 Indexed width
extern const int SCREEN_HEIGHT_0; // 0 indexed width
extern const int HALF_WIDTH;
extern const int HALF_HEIGHT; 

extern const int MARGINX; // SCREEN_WIDTH *0.2;
extern const int MARGINTOP; // SCREEN_HEIGHT * 0.2;
extern const int MARGINBOTTOM; // SCREEN_HEIGHT * 0.3;
extern const int SCREEN_WIDTH_MINUS_MARGINX;
extern const int SCREEN_HEIGHT_MINUS_MARGINBOTM;

lander_game_type *lg_create();
void lg_init(lander_game_type *obj);

lander_type *lander_create();

void lander_init(lander_type *obj);

void lander_reset();
void lander_rotate(int direction);
void lander_thrust(int power);
void lander_abort();
void lander_update();
void lander_render();
void lander_crash();
void lander_land();
void lander_makeBounce();

// Game functions
void largeLanderRasterMove(int oldx, int oldy, int oldrot, int x, int y, int rot);
void smallLanderRasterMove(int oldx, int oldy, int oldrot, int x, int y, int rot);

#endif  /* _H_LANDER */