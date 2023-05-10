#include "lander.h"
#include "nanocomp.h"
#include "lander_game_ext.h"

// screen size variables, note zero indexed versions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_WIDTH_0 = 639; // 0 Indexed width
const int SCREEN_HEIGHT_0 = 479; // 0 indexed width
const int HALF_WIDTH = 320;
const int HALF_HEIGHT = 240; 

const int MARGINX  = 128; // SCREEN_WIDTH *0.2;
const int MARGINTOP = 96; // SCREEN_HEIGHT * 0.2;
const int MARGINBOTTOM = 72; // SCREEN_HEIGHT * 0.3;
const int SCREEN_WIDTH_MINUS_MARGINX = 512;
const int SCREEN_HEIGHT_MINUS_MARGINBOTM = 408;

lander_game_type *lg_create(int rows, int cols)
{
  lander_game_type *obj = (lander_game_type*)sbrk(sizeof(lander_game_type));
  lg_init(obj);
  return obj;
}

void lg_init(lander_game_type *obj)
{
  // Initialization logic, decimal variales x 1000 (was 10000 in JS)
  obj->score = 0;
  obj->time = 0;
  obj->counter = 0;
  obj->zoomedIn = false;
  obj->zoomFactor = 4;
  obj->gravity = 15; // 0.0005, use 3 digits for decimal == 0.005
  obj->thrustAcceleration = 5; // Multiplied by 5 or 15 for abort 0.015
  obj->topSpeed = 6000; //0.35, 
  obj->drag = 997;   // 0.9997 0.03% depends on how many loops per s
  obj->view.x = 0;
  obj->view.y = 0;
  obj->view.scale = 1;
  obj->view.left = 0;
  obj->view.right = 0;
  obj->view.top = 0;
  obj->view.bottom = 0;
  obj->view.changed = false;
  obj->oldView.x = 0;
  obj->oldView.y = 0;
  obj->oldView.scale = 1;
  obj->oldView.left = 0;
  obj->oldView.right = 0;
  obj->oldView.top = 0;
  obj->oldView.bottom = 0;
  obj->currentCombi = 0;
  obj->textLabels = true;
}

lander_type *lander_create()
{
  lander_type *obj = (lander_type*)sbrk(sizeof(lander_type));
  lander_init(obj);
  return obj;
}

  void lander_init(lander_type *obj)
  {
	// Initialization logic
  obj->altitude = 0;
  obj->posx = 0;
  obj->deltax = 0;
  obj->posy = 0;
  obj->deltay = 0;
  obj->velx = 0;
  obj->vely = 0;
  obj->oldposx = 0;
  obj->oldposy = 0;
  obj->scale = false; // false=Small true=Large 
  obj->left = 0;
  obj->right = 0;
  obj->top = 0;
  obj->bottom = 0;
  obj->bottomLeftx = 0;
  obj->bottomLefty = 0;
  obj->bottomRightx = 0;
  obj->bottomRighty = 0;
  obj->thrustBuild = 0;
  obj->exploding = false;
  obj->bouncing = 0;
  obj->thrusting = 0;  
  obj->rotation = 5;
  obj->oldRotation = 5;
  obj->targetRotation = 5;
  obj->lastRotationTime = 0;
  obj->abortCounter = -1;
  obj->lastAbort = 0;
  obj->fuel = 0;
  obj->active = true; //?
  }
  
void lander_reset() { 

	lander->abortCounter = -1; 
	lander->lastAbort = lg->counter; 
	lander->velx = 0;
	lander->vely = 0;
	lander->posx = 120; //120 or 370
	lander->posy = 120; //120 or 280
	lander->deltax = 0;
	lander->deltay = 0; 
	lander->altitude = 0;
	lander->rotation = 5;
	lander->targetRotation = 7; 
	lander->oldRotation = 7;
	lander->scale = 1; 
	lander->thrustBuild = 0; 
	lander->bouncing = false; 
	lander->active = true; 
	lander->exploding = false; 
	lander->thrusting = 0;
	lander->scale = false; 
	lander->lastRotationTime = 0;
	lander->abortCounter = -1; // -1; 
	lander->lastAbort = -1; // -1; 

};

	
void lander_rotate(int direction) { 

    // Wil need to adjust > 8 for speed of rotation - was 80 in JS
	if(lg->counter - lander->lastRotationTime > 2) {
		
		lander->targetRotation+=direction; 
		lander->targetRotation = clamp(lander->targetRotation, 0, 10); 
		
		lander->lastRotationTime = lg->counter; 
	}
		
};

// Used to turn thrust on = 5 and  off = 0
void lander_thrust(int power) { 
	lander->thrusting = power; 
};

void lander_abort() {
	// was 10000
	if(lg->counter - lander->lastAbort > 50) { 
		// Was 100 in JS
		lander->abortCounter = 5;
		lander->lastAbort = lg->counter; 
	} 
}
	
	void lander_update() { 
	
		lander->oldRotation = lander->rotation;
		// Rotation logic may need simplification
		lander->rotation += ((lander->targetRotation - lander->rotation) >> 1);  // was *0.3 now /2
		if ((lander->rotation - lander->targetRotation) <= 1) lander->rotation = lander->targetRotation; 
		
		//if(exploding) this.updateShapesAnimation(); 
		
		if(lander->active) { 
			
			if(lander->abortCounter > -1) { 
				 
				lander->targetRotation = 5; 
				
				if(lander->fuel > 0) lander->thrustBuild = 15; // Was 3 when standard thrust was 1
				lander->abortCounter --; 
				lander->fuel-=10; // Abort counter was 100 now 10 so need 10 x as much fuel used
				
			}
			
			if(lander->fuel <= 0) lander->thrusting = 0; 
		    // Original code was thrustBuild += ((this.thrusting-thrustBuild)*0.2); which reduced thrust over 5 goes when thrusting=0
			// Sets thrustBuild to 5 normally, reduces by 1 per turn when thrustBuild > 0 rather than reduce by 0.2
			lander->thrustBuild += ((lander->thrusting - (lander->thrustBuild!=0)));  // This reduces thrustBuild 1 at a time assuming now 5 or 15 Abort
		
			if(lander->thrustBuild > 0) { 
				// Need to adjust depending on loop timing
				long thrustVec = lg->thrustAcceleration * lander->thrustBuild; 
				// TODO Need to work of change in vel by rotation
				// Thrust Cos(rotation) = Y component, Sin(Rotation) X component.
				// Rot=0 Sin=-1, Cos=0, Rot=5 Sin=0, Cos=1 Rot=10 Sin=1, Cos=0
				lander->velx = lander->velx  - (thrustVec * sin[lander->rotation])/1000 ;
				lander->vely -= thrustVec * cos[lander->rotation]/1000;
				lander->fuel -= (lander->thrustBuild);
				//printf("lander_update thrustbuild=%d, thrustVec=%ld rotation=%d \n", lander->thrustBuild, thrustVec, lander->rotation);
			}
			lander->oldposx = lander->posx;
			lander->oldposy = lander->posy;
            lander->velx = (lander->velx * lg->drag / 1000);
			lander->deltax += lander->velx;  // Add this turn velocity to fraction remander from last turn
			int deltaint = (int)(lander->deltax/1000);
			lander->posx += deltaint; // Add 'integer' amount to X
			lander->deltax = lander->deltax - deltaint * 1000; // Leave any fraction for next turn, avoid doing % 1000 as already done calc above
  		    //printf("lander->posx=%d, deltaint=%d, lander->deltax=%ld, lander->velx=%ld \n", lander->posx, deltaint, lander->deltax, lander->velx);

			// Wrap lander from right hand side to left hand side, only for small lander
			if (lander->posx > (SCREEN_WIDTH_0 - 5)){
				lander->posx -= SCREEN_WIDTH;
			}
			// Wrap lander from left hand side to right hand side , only for small lander
			if (lander->posx < 5){
				lander->posx += SCREEN_WIDTH;
			}

			lander->deltay += lander->vely;  // Add this turn velocity to fraction remander from last turn
			deltaint = (int)(lander->deltay/1000);		
			lander->posy += deltaint;
			lander->deltay = lander->deltay - (deltaint * 1000); // Leave any fraction for next turn, avoid doing % 1000 as already done calc above

			lander->vely += lg->gravity; 
			
			if(lander->vely > lg->topSpeed) {
				lander->vely = lg->topSpeed; 
			} else if (lander->vely < -lg->topSpeed) {
				lander->vely = -lg->topSpeed; 
			}

			//if (lander->posy < 96){
			//	lander->posy = 96;
			//}

			// Scale true - Large Lander, false - Small Lander
			// May be avoid scrolling left, right, top, bottom if performance an issue
			if (lander->scale){
				lander->left = lander->posx - 9; 
				lander->right = lander->posx + 9; 
				lander->bottom = lander->posy + 8; 
				lander->top = lander->posy - 12; 
				lander->bottomLeftx = lander->left;
				lander->bottomLefty = lander->bottom; 
				lander->bottomRightx = lander->right;
				lander->bottomRighty = lander->bottom; 
			} else {
				lander->left = lander->posx - 5; 
				lander->right = lander->posx + 5; 
				lander->bottom = lander->posy + 4; 
				lander->top = lander->posy - 6; 
				lander->bottomLeftx = lander->left;
				lander->bottomLefty = lander->bottom; 
				lander->bottomRightx = lander->right;
				lander->bottomRighty = lander->bottom; 
			}
			
		} else if (lander->bouncing >= 0) {
			
			lander->posy  += (cos[lander->bouncing]>>5);  // was 0.07 cos is x 1000 , divide by 32
			lander->bouncing--;
			
		}
		if(lander->fuel < 0) lander->fuel = 0; 
		//setThrustVolume(Math.min(1,thrustBuild)); 
		//printf("lander_update end status Counter=%d, x=%d, y=%d, velx=%ld, vely=%ld, fuel=%d \n",lg->counter, lander->posx, lander->posy, lander->velx, lander->vely,lander->fuel);
		
	};
	
	void lander_render () { 
		printf("Lander Render scale=%d, %d, %d, %d, %d, %d, %d \n", lander->scale, lander->oldposx, lander->oldposy, lander->oldRotation,
										lander->posx, lander->posy, lander->rotation );

		if (lander->oldposx!=lander->posx ||
		    lander->oldposy!=lander->posy ||
			lander->oldRotation!=lander->rotation)
		{
				// Large or Small lander
				if (lander->scale){
  					/*printf("Large Lander View adj oldx=%d, oldy=%d, Vposx=%d, Vposy=%d \n",((lander->oldposx - lg->oldView.left) << 2),
										((lander->oldposy - lg->oldView.top) << 2),
										((lander->posx - lg->view.left) << 2),
										((lander->posy - lg->view.top) << 2));*/
  					/*printf("View  oldview.left=%d, oldviewtop=%d, viewleft=%d, viewtop=%d \n",lg->oldView.left,
										lg->oldView.top,
										lg->view.left,
										lg->view.top); */
					if (lg->oldView.left == 0 && lg->oldView.right == 0 ){
						// Don't need to remove previous large lander
						largeLanderRasterMove(0, 
											0, 
											lander->oldRotation,
											((lander->posx - lg->view.left) << 2), 
											((lander->posy - lg->view.top) << 2)+ 28,  
											lander->rotation);

					} else {
						largeLanderRasterMove(((lander->oldposx - lg->oldView.left) << 2), 
											((lander->oldposy - lg->oldView.top) << 2)+28, 
											lander->oldRotation,
											((lander->posx - lg->view.left) << 2), 
											((lander->posy - lg->view.top) << 2)+28,  
											lander->rotation);
					}
				} else {
					smallLanderRasterMove(lander->oldposx, lander->oldposy, lander->oldRotation,
										lander->posx, lander->posy, lander->rotation);
				}
		}

//       Draw flames TBD		
//		if((lander->thrustBuild > 0) && (lander->active)) {
//			c.lineTo(0,11+(Math.min(thrustBuild,1)*20*((((lg->counter>>1)%3)*0.2)+1)));
//			c.closePath(); 
//		}	
	};

	void lander_crash() { 
		lander->rotation = 0;
		lander->targetRotation = 0; 
		lander->active = false; 
		lander->exploding = true; 
		lander->thrustBuild = 0; 
	};

	void lander_land() { 
		lander->active  = false; 
		lander->thrustBuild = 0; 
	};

	void lander_makeBounce() { 
		// May want to make this 2 cycles - 20 same as 2 PI with Nanocomp Cos
		lander->bouncing = 10;	
	};
