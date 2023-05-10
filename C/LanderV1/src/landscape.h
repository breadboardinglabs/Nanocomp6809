#include "Lander.h"

#ifndef _H_LANDSCAPE
#define _H_LANDSCAPE

void landscape_render(byte colour);
void landscape_setZones ();
int landscape_getMultiplier (int linex);
void landscape(byte colour);
void landscapeZoom(byte colour);
bool pointIsLessThanLine(int pointx, int pointy, int linepoint1, int linepoint2);
void landscape_init();

extern const int numberOfPoints;
extern const word landscapeX[];
extern const word landscapeY[];
extern int xToPoint[];

#endif  /* _H_LANDSCAPE */