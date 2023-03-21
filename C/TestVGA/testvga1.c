//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CCS C Compiler
// Demonstration mathematical and graphics drawing program for VGA card.
// Initial prototype code based on examples from www.glensstuff.com
// 21/04/2021
// 
// cd /mnt/c/Users/Dave/Documents/_Breadboarding/VideoController/v11_640x480_Graphics
// cmoc --void-target --org=1000 --data=5000 -i --intdir=int --srec testvga1.c 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmoc.h"
#include "cmocextra.h"
#include "nanocomp.h"

#define xoffset      320;                                               // Screen center x
#define yoffset      240;                                               // Screen center y
const unsigned long VRAMOFFSET = 0x8000;
const unsigned long VRAMPAGE   = 0x4000;
const unsigned long VRAMTOP    = 0xC000;
const unsigned long VRAMPAGE3TOP = 0x9600; 
const unsigned int GMODE2 = 0x08; 
const unsigned int PIXELSPERLINE = 640;

byte *pageRegister = 0xD480;
byte  page;                                                             // Global VRAM page so don't have to keep updating
//unsigned int yLookup[59];                                               // This stores the pre-calculated addresses for each yrow
unsigned long yLookup[59];                                               // This stores the pre-calculated addresses for each yrow


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                             
void plotPoint(int x, int y, byte colour)                                     // Routine to write pixel data to video RAM
{  
   byte *vramAddress;                                                             // Video memory address
   long address;
   byte pixelBit = (byte) x & 7;                                                     // The bit from left to right 0 left 7 right
   unsigned int xcol = x - pixelBit;                                                   // x is the character position as 8 raster lines
   unsigned int yrow = y >> 3;                                                         // Divide y by 8 - shift right x 3
   unsigned int rasterRow = y & 7;
   byte newPage = 0;
   address = (unsigned long) xcol + rasterRow + yLookup[yrow];
   // Calculate RAM address from pre-calc array   
   // Adjust address for VRAM page
   if (address > VRAMTOP )
   {                                              
     newPage++;
     address -= VRAMPAGE;
     if (address > VRAMTOP)
     {
       newPage++;
       address-=VRAMPAGE;
     }
   }

   if (newPage != page)
   {
     page = newPage;
     *pageRegister = (byte) (GMODE2 + newPage);
   }
   vramAddress = (byte *) address;
   byte pixelByte = *vramAddress;
   int shiftCount = (7 - pixelBit);                                               // Could pre-calc mask for each bit
   byte pixelMask = (byte) 1 << shiftCount;
   if (colour > 0){
     pixelByte = pixelByte | pixelMask;
   } else {
     pixelByte = pixelByte & ~pixelMask;
   }
   *vramAddress = pixelByte;
}

// Bresenham's line algorithm
void DrawLine(int x0, int y0, int x1, int y1, byte colour)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    // No optimisation for verticle or horizontal lines
    while (true)
    {
        plotPoint(x0, y0, colour);
        if (x0 == x1 && y0 == y1) break;
        int e2 = error << 1;    // Multiply by 2 same as shift left
        if (e2 >= dy)
        {
            if (x0 == x1) break;
            error += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            if (y0 == y1) break;
            error += dx;
            y0 += sy;
        }
    }
}

void    EightWaySymmetricPlot(int xc,int yc,int x,int y, byte colour)  
   {  
    plotPoint(x+xc,y+yc,colour);  
    plotPoint(x+xc,-y+yc,colour);  
    plotPoint(-x+xc,-y+yc,colour);  
    plotPoint(-x+xc,y+yc,colour);  
    plotPoint(y+xc,x+yc,colour);  
    plotPoint(y+xc,-x+yc,colour);  
    plotPoint(-y+xc,-x+yc,colour);  
    plotPoint(-y+xc,x+yc,colour);  
   }  

// Bresenham's circle algorithm
  void DrawCircle(int xc,int yc,int r, byte colour)  
   {  
    int x=0,y=r,d=3-(2*r);  
    EightWaySymmetricPlot(xc,yc,x,y, colour);  
  
    while(x<=y)  
     {  
      if(d<=0)  
      {  
        d=d+(4*x)+6;  
      }  
     else  
      {  
        d=d+(4*x)-(4*y)+10;  
        y=y-1;  
      }  
       x=x+1;  
       EightWaySymmetricPlot(xc,yc,x,y, colour);  
      }  
    }  

                                                           // Not very efficient!

void DrawFill(int x0, int y0, int x1, int y1, byte colour)                                             // Routine for drawing a rectangular fill.  
{                                                           // (x0, y0) = upper left corner. (x1, y1) = lower left corner
   for (int y = y0; y <= y1; y++) {   
      for (int x = x0; x <= x1; x++) {
         plotPoint(x, y, colour);
      }
   }
}


void ClearScreenGraphics()                                                         // Fast routine for clearing video memory
{                                                                       // Does not wait for !H_SYNC
   byte *vramAddress;                                                       // Video memory address
   byte clear = 0x0;
   page = 0;
   *pageRegister = (byte)(GMODE2 + page);
   // Clear Page 0 all 16K
   for (unsigned int i = VRAMOFFSET; i <= VRAMOFFSET + VRAMPAGE; i++)
   {                 
     vramAddress = (byte *) i;
     *vramAddress = clear;
   }
   page++;
   *pageRegister = (byte)(GMODE2 + page);
   for (unsigned int i = VRAMOFFSET; i<=VRAMOFFSET + VRAMPAGE; i++){                 // Clear Page 1 all 16K
     vramAddress = (byte *)i;
     *vramAddress = clear;
   }
   page++;
   *pageRegister = (byte)(GMODE2 + page);
   for (unsigned int i = VRAMOFFSET; i<=VRAMPAGE3TOP; i++){                          // Clear Page 2 upto 0x9600
     vramAddress = (byte *) i;
     *vramAddress = clear;
   }
   page = 0;
   *pageRegister = (byte)(GMODE2 + page);
  
}

void DrawLinePattern()
{
   byte colour = 0x1;
   int    x1 = 0;
   int    y1 = 0;
   int    x2 = 0;
   int    y2 = 0;
   int    start = 0;
   while (x2 < 639)  {
      x1 = 0; y1 = 479; 
//      x1 = 0; y1 = 406; 
      DrawLine(x1, y1, x2, y2, colour);
      x2 = x2 + 20;
   }   
   while (y2 < 479)  {
      x1 = 0; y1 = 479; 
//      x1 = 0; y1 = 406; 

      DrawLine(x1, y1, x2, y2, colour);
      y2 = y2 + 20;
   }  
   x1 = 0; y1 = 479; y2 = 479; 
//   x1 = 0; y1 = 406; y2 = 406; 

   DrawLine(x1, y1, x2, y2, colour);
   x2 = 639; y2 = 0; start = 0;                                        
   while (start <= 31)  {
      x1 = 639; y1 = 479; 
//      x1 = 639; y1 = 406; 
      DrawLine(x1, y1, x2, y2, colour);
      x2 = x2 -20; start++;
   }  
   x2 = 0;                                          
   while (y2 <479)   {
      x1 = 639; y1 = 479;
//   while (y2 <406)   {
//      x1 = 639; y1 = 406;
      DrawLine(x1, y1, x2, y2, colour);
      y2 = y2 + 20;
   }  
   x1 = 639; x2 = 0; y1 = 479; y2 = 479; 
//   x1 = 639; y1 = 406; y2 = 406; 
   DrawLine(x1, y1, x2, y2, colour);

   x1 = 639; x2 = 639; y1 = 0; y2 = 479;
   DrawLine(x1, y1, x2, y2, colour);

   x1 = 0; x2 = 0; y1 = 0; y2 = 479;
   DrawLine(x1, y1, x2, y2, colour);

   x1 = 0; x2 = 639; y1 = 0; y2 = 0;
   DrawLine(x1, y1, x2, y2, colour);

}

void DrawCirclePattern()
{
   byte colour = 0x1;
   int    x1 = 320;
   int    y1 = 240;
   int    r = 10;
   while (r < 220)  {
      DrawCircle(x1, y1, r, colour);
      r += 20;
   }
}   
    
void DrawCascadingSquares()
{  
   byte colour = 0; 
   int x1 = 0;
   int y1 = 0;
   while (colour <= 63) {
      int x2 = x1 + 72;  int y2 = y1 + 38;
      DrawFill(x1, y1, x2, y2, colour);
      x1 = x1 + 9;   y1 = y1 + 7;   colour++;   
   }
   colour = 0; x1 = 567;   y1 = 0;
   while (colour <=63)  {
      int x2 = x1 + 72;  int y2 = y1 + 38;
      DrawFill(x1, y1, x2, y2, colour);
      x1 = x1 - 9;   y1 = y1 + 7;   colour++;
   }
}

void DrawChequerBoard()
{ 
   byte colour = 1;       
   for (int x1 = 120; x1 <= 480; x1 = x1 + 40)   {
      int x2 = x1 + 39; 
      colour = 1 - colour;
      for (int y1 = 40; y1 <= 400;   y1 = y1 + 40)  {
         int y2 = y1 + 39;
         DrawFill(x1, y1, x2, y2, colour);
         colour = 1 - colour;  
      }
   }
}


void WipeOut()
{
   int x1 = 0; 
   int y1 = 0; 
   int x2 = 639; 
   int y2 = 479; 
   byte colour = 0x00;
   DrawFill(x1, y1, x2, y2, colour);
}


int main()
{
   for (unsigned int i = 0; i<=59; i++){
     yLookup[i] = (unsigned long) (VRAMOFFSET + i * PIXELSPERLINE);
   }
   ClearScreenGraphics();
   plotPoint(320, 40, 0x01);
   plotPoint(321, 40, 0x01);
   plotPoint(320, 41, 0x01);

   plotPoint(320, 240, 0x01);
   plotPoint(321, 240, 0x01);
   plotPoint(320, 241, 0x01);

   plotPoint(320, 407, 0x01);
   plotPoint(321, 407, 0x01);
   plotPoint(320, 408, 0x01);

   plotPoint(320, 417, 0x01);
   plotPoint(321, 417, 0x01);
   plotPoint(320, 418, 0x01);

   //DrawPolarRoses();
   DrawLinePattern();
   DrawCirclePattern();

   DrawCircle( 320, 240, 105,0);
   
   //DrawCascadingSquares();
   //DrawChequerBoard();
   //DrawSinXdivXfunction();
   //delay_ms(1000);
   //DrawLandscape();
   //ComputeAndPlotLorenzAttractor();
   //delay_ms(2000);
   //WipeOut();
   
   returnNC();  // Causes SWI for Nanocomp to return to monitor
   return 0;
}
