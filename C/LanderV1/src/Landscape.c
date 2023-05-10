#include "landscape.h"
#include "lander.h"
#include "cmocextra.h"
#include "nanocomp.h"
#include "lander_game_ext.h"

// const definition means gets populated from assembley FCB rather than ldb # stb > saves a lot of bytes
const word landscapeX[171] = {0, 5, 6, 11, 12, 15, 16, 19, 20, 22, 29, 31, 33, 38, 43, 47, 52, 57, 61, 64, 68, 70, 75, 80, 85, 89, 94, 99, 103, 104, 108, 109, 112, 113,
117, 122, 125, 129, 132, 136, 138, 145, 146, 149, 150, 154, 163, 168, 173, 175, 180, 182, 187, 189, 192, 196, 201, 205, 210, 213, 217, 219, 221, 224, 229, 234,
238, 243, 245, 246, 247, 248, 252, 257, 258, 261, 266, 267, 272, 273, 275, 279, 282, 285, 292, 299, 303, 308, 309, 312, 314, 317, 322, 323, 327, 328, 331, 332, 336, 338,
340, 345, 346, 349, 350, 351, 354, 359, 361, 364, 368, 373, 410, 413, 417, 420, 422, 425, 429, 433, 438, 443, 447, 449, 455, 456, 459, 463, 466, 468,
476, 485, 495, 504, 522, 525, 527, 528, 532, 534, 539, 541, 542, 546, 550, 553, 555, 560, 564, 573, 578, 580, 583, 588, 589, 592, 597, 601, 605, 606, 611, 612,
615, 616, 619, 619, 622, 629, 631, 633, 638};

// Old version from bottom const word landscapeY[171] = {144, 144, 141, 141, 136, 136, 124, 112, 108, 100, 96, 88, 83, 80, 80, 83, 91, 96, 100, 104, 108, 112, 114, 120, 121, 124, 124, 123, 120, 
//116, 112, 108, 104, 100, 96, 96, 104, 106, 104, 100, 92, 88, 75, 63, 59, 55, 55, 59, 63, 67, 71, 75, 78, 88, 96, 98, 102, 108, 116, 124, 128, 132, 136, 141, 141, 
//144, 152, 157, 165, 177, 186, 193, 204, 205, 210, 214, 214, 210, 210, 205, 205, 204, 200, 192, 187, 169, 168, 165, 161, 157, 153, 149, 149, 136, 124, 120, 120, 
//116, 112, 104, 100, 96, 83, 71, 67, 63, 59, 59, 51, 47, 43, 39, 39, 51, 59, 67, 67, 75, 78, 83, 85, 88, 88, 83, 69, 65, 61, 59, 55, 47, 43, 43, 42, 39, 39, 47, 59
//, 67, 67, 75, 80, 83, 88, 92, 92, 102, 110, 112, 108, 108, 112, 120, 131, 132, 136, 141, 144, 144, 144, 141, 141, 136, 136, 124, 112, 108, 100, 96, 88, 83, 80};
// Y from screen 0 at top
const word landscapeY[171] = {335, 335, 338, 338, 343, 343, 355, 367, 371, 379, 383, 391, 396, 400, 400, 396, 389, 383, 379, 375, 371, 367, 365, 359, 358, 355, 355, 356, 359,
363, 367, 371, 375, 379, 383, 383, 375, 373, 375, 379, 387, 391, 404, 416, 420, 424, 424, 420, 416, 412, 408, 404, 402, 391, 383, 381, 377, 371, 363, 355, 351, 347, 343, 338,
338, 335, 327, 322, 314, 302, 294, 286, 276, 274, 269, 265, 265, 269, 269, 274, 274, 276, 279, 287, 292, 310, 311, 314, 318, 322, 326, 330, 330, 343, 355, 359, 359, 363, 367, 
375, 379, 383, 396, 408, 412, 416, 420, 420, 428, 432, 436, 440, 440, 428, 420, 412, 412, 404, 401, 396, 394, 391, 391, 396, 410, 414, 418, 420, 424, 432, 436, 436, 437, 440, 
440, 432, 420, 412, 412, 404, 400, 396, 392, 387, 387, 377, 369, 367, 371, 371, 367, 359, 348, 347, 343, 339, 335, 335, 335, 338, 338, 343, 343, 355, 367, 371, 379, 383, 391, 396, 400};

// The start points for landing zones and their score multipliers
const int availableZones[11][2] = {
    {0, 4},
    {13, 3},
    {25, 4},
    {34, 4},
    {63, 5},
    {75, 4},
    {106, 5},
    {111, 2},
    {121, 5},
    {133, 2},
    {148, 3}
  };

// For each game currentCombi iterates through these values which select which available zone multipliers are used for the current game
// This array references the zones, which then references the point which can then be resolved to x value!!
const int zoneCombi[7][4] = {
    {2,3,7,9}, 
    { 7,8,9,10}, 
    { 2,3,7,9 },
    { 1,4,7,9 },
    { 0,5,7,9 },
    { 6,7,8,9 },
    { 1,4,7,9 }
  };	
    

int xToPoint[640];
const int numberOfPoints = 170;


void landscape_init()
{
  // build a mapping between x cordinate and closest point to left
  int currentPoint = 0;

  // iterate through every X value  
  for (int i = 0; i<=639; i++){
//    if ((i >= landscapeX[currentPoint]) && (landscapeX[currentPoint] > currentPoint)){
    if ((i == landscapeX[currentPoint + 1])){
        currentPoint++;
    }
    xToPoint[i] = currentPoint;
	printf("i=%d, currentPoint=%d, landscapeX[currentPoint],%d, landscapeY[currentPoint],%d \n", i,currentPoint, landscapeX[currentPoint], landscapeY[currentPoint]  );
  }

  lg->currentCombi = 0;
  // To find multiplier Lookup Point for x for LeftBottom and RightBottom of lander
  // Compare point Y left and point Y right if == then landable
  // Line 0 goes from point 0 -1, line 1 goes from point 1-2

}

void drawScale(int y){
  DrawLine(0,y,639,y,1) ;
  for (int i = 10; i<=639; i+=10){
	DrawLine(i,y+1, i, y+2,1);
  }

  for (int i = 0; i<=639; i+=50){
	DrawLine(i,y+1, i, y+5,1);
  }
}

void landscapeZoom(byte colour)
{
  int offset = lg->view.left;
  int offsety = lg->view.top;

//  int xlow = 42;
//  int xhigh = 127;
  int xlow = xToPoint[lg->view.left];
  int xhigh = xToPoint[lg->view.right]-1;
  int multiplier = 1;

  //offset = landscapeX[xlow];

  //printf("view.left=%d, view.right=%d, view.top=%d, xlow=%d, xhigh=%d, offset=%d \n", lg->view.left, lg->view.right, lg->view.top, xlow, xhigh, offset);

  for (int i = xlow; i<=xhigh; i++){
	int y1 = (landscapeY[i]-offsety)<<2 ;
	int y2 = (landscapeY[i+1]-offsety)<<2 ;
    if ((y2 <= SCREEN_HEIGHT_0) && ( y1 <= SCREEN_HEIGHT_0) && (y1 > 0) && (y2 > 0)) {
    	DrawLine(((landscapeX[i]-offset)<<2), y1, ((landscapeX[i+1]-offset) << 2), y2, colour);
	 	multiplier = 1;
		for (int zone =0 ; zone < 11; zone++ ){
			//printf("Zone loop i=%d, zone=%d, availableZone=%d \n", i, zone, availableZones[zone][0]);

			if (availableZones[zone][0]==i){
				//printf("Zone available i=%d, zone=%d \n", i, zone);
				for (int combi = 0; combi < 4; combi++){
					//printf("Combi check CurrentCombi=%d, combi=%d \n", lg->currentCombi, combi);
					if (zoneCombi[lg->currentCombi][combi]==zone){
						multiplier = availableZones[zone][1];
					//printf("Multiplier=%d \n", multiplier);
					}
				}

			}
		}
		if (multiplier > 1){
			int y = ((y1+15) >> 4); // Divide pixels by 16 (>> 4) +15 spaces out a bit better
			int x = ((((landscapeX[i+1] - landscapeX[i]) * 2) + (landscapeX[i]-offset) * 4) / 8); // + 8 spaces out a bit better
			printf("DrawChar x=%d, y=%d, multiplier=%d,i=%d,offset=%d, [i+1]=%d,[i]=%d \n", x, y, multiplier,i,offset, landscapeX[i+1], landscapeX[i]);
			if (colour > 0){
				DrawChar(x, y, (byte) (multiplier + 0x30)); // Convert multiplier 2-5 to ASCII
				DrawChar(x+1, y, (byte) ('x'));
			} else {
				DrawChar(x, y, (byte) (' ')); // Convert multiplier 2-5 to ASCII
				DrawChar(x+1, y, (byte) (' '));
			}
		}
        /*char sprintfmsg[32];

		if (i==111){
			sprintf(sprintfmsg, "X1=%d,Y1=%d,x[i]=%d,OFF=%d", ((landscapeX[i]-offset)<<2), y1, landscapeX[i], offset);
			DrawMessage(5, 28, sprintfmsg);
			sprintf(sprintfmsg, "X2=%d,Y2=%d,x[i+1]=%d,OFF=%d", ((landscapeX[i+1]-offset)<<2), y2, landscapeX[i+1], offset);
			DrawMessage(45, 28, sprintfmsg);
		}*/

	}
  }
  /*drawScale(420);
	largeLanderRasterMove(0, 
						0, 
						lander->oldRotation,
						375, 
						420,  
						5);
  delay(60000); */

}




void landscape(byte colour)
{
  //printf("Draw Landscape updated\n");
  int multiplier = 1;

  for (int i = 0; i<=169; i++){                          // Draw landscape lines
    DrawLine(landscapeX[i], landscapeY[i], landscapeX[i+1], landscapeY[i+1], colour);
  	//printf("Drawline loop i=%d \n", i);

 	multiplier = 1;
	for (int zone =0 ; zone < 11; zone++ ){
    	//printf("Zone loop i=%d, zone=%d, availableZone=%d \n", i, zone, availableZones[zone][0]);

		if (availableZones[zone][0]==i){
			//printf("Zone available i=%d, zone=%d \n", i, zone);
			for (int combi = 0; combi < 4; combi++){
				//printf("Combi check CurrentCombi=%d, combi=%d \n", lg->currentCombi, combi);
				if (zoneCombi[lg->currentCombi][combi]==zone){
					multiplier = availableZones[zone][1];
  				   //printf("Multiplier=%d \n", multiplier);
				}
			}

		}
	}
	if (multiplier > 1){
		int y = ((landscapeY[i]+15) >> 4); // Divide pixels by 16 (>> 4) +15 spaces out a bit better
		int x = (((landscapeX[i+1] - landscapeX[i]+4) >> 1) + landscapeX[i]) >> 3; // + 4 spaces out a bit better
		//printf("DrawChar x=%d, y=%d, multiplier=%d \n", x, y, multiplier);
		if (colour > 0){
			DrawChar(x, y, (byte) (multiplier + 0x30)); // Convert multiplier 2-5 to ASCII
			DrawChar(x+1, y, (byte) ('x'));
		} else {
			DrawChar(x, y, (byte) (' ')); // Convert multiplier 2-5 to ASCII
			DrawChar(x+1, y, (byte) (' '));
		}
	}

  }
  //drawScale(440);
}

void landscape_render(byte colour) { 
/*
    printf("Landscape Render %d", colour);
    // the view.x and view.y and view.scale will have been applied to 2D context before this call
	int offset = 0; 

	// These two loops handled scrolling on page left and right of current page, ie overflow x=640 or under x=0
	(lg->view.left-offset > SCREEN_WIDTH_0) { 
		offset+=SCREEN_WIDTH; 
	}
	
	while(lg->view.left-offset<0) { 
		offset-=SCREEN_WIDTH;
	}
	
	int startOffset = offset; 
    printf("Offset %d", offset);
	
	int i = 0; 
	
	// lines[i].p2.x+offset<view.left
	// 
	while(lines[i].p2.x+offset<lg->view.left) { 
		i++; 
		if(i > numberOfPoints) {
			i=0; 
			offset+=SCREEN_WIDTH; 
		}
	}
	
	c.moveTo(line.p1.x + offset, line.p1.y);  
	
	var zoneInfoIndex = 0; 

	while((line = lines[i]).p1.x+offset<lg->view.right) { 
		
		var point = line.p2; 
		c.lineTo(point.x+offset, point.y);
		
		if((counter%20>10) && (line.multiplier!=1)){ 
			var infoBox; 
			
			if(!zoneInfos[zoneInfoIndex]) { 
				infoBox = zoneInfos[zoneInfoIndex] = new InfoBox(1,50); 
				document.body.appendChild(infoBox.domElement); 
			} else {
				infoBox = zoneInfos[zoneInfoIndex]; 
				infoBox.show(); 
			}
			infoBox.setText(line.multiplier+'x'); 
			infoBox.setX(((((line.p2.x-line.p1.x)/2)+line.p1.x+offset)*view.scale)+view.x); 
			infoBox.setY(((line.p2.y+2) *view.scale)+view.y); 
			zoneInfoIndex++; 
			
		}
		
		i++; 
		if(i>=lines.length) {
			i=0; 
			offset+=rightedge; 
		}
		

	}
	
	
	for(var i=zoneInfoIndex; i<zoneInfos.length; i++) { 
		zoneInfos[i].hide(); 
	}
	
*/	
	
};

// This looks up the X coordinate to find the multiplier for the point
// Looks up X to point then matches the available zones by point
// A match for the point then is checked vs the current zoneCombi
int landscape_getMultiplier (int linex) {
	//printf("landscape_getMultiplier line=%d \n", linex);
	int multiplier = 1;
	// Lookup x value to find point at start of line
	int pointx = xToPoint[linex];
	for (int zone; zone < 11; zone++ ){
		if (availableZones[zone][0]==pointx){
			for (int combi = 0; combi < 4; combi++){
				if (zoneCombi[lg->currentCombi][combi]==zone){
					multiplier = availableZones[zone][1];
				}
			}

		}
	}
	//printf("landscape_getMultiplier multiplier=%d \n", multiplier);
	return multiplier;
}	

void landscape_setZones () { 
	lg->currentCombi++;
	if(lg->currentCombi >= 6) lg->currentCombi = 0;
};


		
	

