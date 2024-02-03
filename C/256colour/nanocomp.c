#include "nanocomp.h"

const word VRAMOFFSET = 0x8000;
const word VRAMPAGE   = 0x4000;
const word VRAMTOP    = 0xC000;
const word VRAMPAGE3TOP = 0x9600;
 
const word PIXELSPERLINE = 0x0280;
const word BYTESPERROW = 0x280;
const word BYTESPERCHARROW = 0x500;

const word CHARSPERROW = 80;
const word ROWSPERPAGE = 30;
const byte LINEFEED = 0x0a;


const unsigned int CRTCAR = 0xD400;
const unsigned int CRTCDR = 0xD401;
const unsigned int COLOURLATCH = 0xD500;

/*  Sin  Cos
0	    0	1000
18	309	 951
36	588	 809
54	809	 588
72	951	 309
90 1000	   0 */
//  0=-90, 5=0, 11=90 -90, -72, -54, -36, -18, 0, 18, 36, 54, 72, 90 
int sin[11] = {-1000, -951, -809, -588, -309, 0, 309, 588, 809, 951,1000};
int cos[11] = {0, 309, 588, 809, 951, 1000, 951, 809, 588, 309, 0};

byte *pageRegister = PAGE_REGISTER;
byte *crtcPaletteAddressRd = CRTC_DAC_ADD_RD;
byte *crtcPaletteAddressWr = CRTC_DAC_ADD_WR;
byte *crtcPaletteColourValue = CRTC_DAC_COL_VALUE;
byte *crtcPalettePixelMask = CRTC_DAC_PIXEL_MASK;

int  page;                                                             // Global VRAM page so don't have to keep updating
//unsigned int yLookup[59];                                               // This stores the pre-calculated addresses for each yrow
word yLookup[60];                                               // This stores the pre-calculated addresses for each yrow
byte pixelmaskLookup[8];
byte current_video_mode;

// Keys Hex  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  L  S CN   G  M  R  I
//  Code    22 24 02 12 14 00 10 04 01 11 03 13 23 33 21 20 05 15 25  35 31 30 32
//           00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0A, 0B, 0C, 0D, OE, 0F, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 1A, 1B, 1C, 1D, 1E, 1F, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 2A, 2B, 2C, 2D, 2E, 2F, 30, 31, 32, 33, 35
//          '5','8','2','A','7','L',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','6','9','3','B','4','S',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','F','E','0','C','1','N',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','R','M','I','D',' ','G'
// Nanocomp Keypad to ASCII Lookup
unsigned char keyCharLookup[0x36]={'5','8','2','A','7','L',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                      '6','9','3','B','4','S',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                      'F','E','0','C','1','N',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                      'R','M','I','D',' ','G'};
    
// Round Long number +ve add 500 -ve subtract 500 for integer arithmatic with 3 decimal places (x 1000)
// This effectively rounds up or down before dividing by 1000

long roundl(long x)
{
    return (x + (x >= 0 ? 500l : -500l));
}

// This code does the common conversion from a raw address starting at 0x8000 for upto 64K
// Now excludes 8000 start address to avoid using long, uses 0000 as base 4000 as top
// Will subtract 16K/4000 and increase video page. 

word rawAddressToPaged (word address){
  int newPage = 0;
  //printf("Address before paging %X\n\r", address);
   // Adjust address for VRAM page
   if (address > VRAMPAGE )
   {
     newPage++;
     address -= VRAMPAGE;
     if (address > VRAMPAGE)
     {
       newPage++;
       address-=VRAMPAGE;
     }
   }
   if (newPage != page)
   {
     page = newPage;
     *pageRegister = (byte) (current_video_mode + newPage);
   }
   // Now add 0x8000 after we have done calcs to avoid long
   address += VRAMOFFSET;
   //printf("Address after paging page=%d, %X\n\r", page, address);
   return address;
}

// x and y are in character line coordinates, not graphics coordinates
void DrawChar (int x, int y, byte character){
  word address;
  byte *videoram;
  // VRAMOFFSET now added by rawAddressToPaged to avoid long
  address = (word)(BYTESPERCHARROW * y + (x << 3));
  //printf("DrawChar x=%d, y=%d, Char=%d, Raw Address=%lX\n\r", x , y, character,address);
  // Skip top 3 rows and last 4 rows as blank for Lander font
  videoram = (byte *) (rawAddressToPaged(address) + 2);

  byte *cgram;
  cgram = (byte *)(CG_RAM + 2 + (int)character * 16); // Raster lines per character
  //printf("Character %d, CGRAM=%X\n\r", character, cgram);
  int l;
  for (l = 2; l < 8; l++) {
    *videoram = *cgram++;
    videoram++;
  }
   // Now add 0x8000 after we have done calcs to avoid long
  address = (word)(BYTESPERCHARROW * y + (x << 3) + BYTESPERROW);
  
  videoram = (byte *) rawAddressToPaged(address);
  for (l = 8; l < 12; l++) {
    *videoram = *cgram++;
    videoram++;
  }
}

// x and 7 are in characters not pixels
void DrawMessage(int x, int y, char * msg){
  //printf("DrawMessage (%d,%d) = %s\n", x, y, msg);

  while (*msg){
    while (x>=CHARSPERROW){
      x -= CHARSPERROW;
      y += 1;
    }
    while (y >= ROWSPERPAGE){
      y -= ROWSPERPAGE;
    }
    if (*msg == LINEFEED){
      x = 0;
      y++;
      msg++;
    } else {
      DrawChar(x++, y, (byte) *msg++);
    }
  }
}

// x and 7 are in characters not pixels
void ClearMessage(int x, int y, char * msg){
  //printf("ClearMessage (%d,%d) = %s\n", x, y, msg);
  char space = ' ';
  while (*msg){
    while (x>=CHARSPERROW){
      x -= CHARSPERROW;
      y += 1;
    }
    while (y >= ROWSPERPAGE){
      y -= ROWSPERPAGE;
    }
    if (*msg == LINEFEED){
      x = 0;
      y++;
      msg++;
    } else {
      DrawChar(x++, y, space);
      msg++;
      //printf("ClearMessage (%d,%d, %X)\n", x, y, msg);
    }
  }
}

void ClearScreenGraphics()                                               // Fast routine for clearing video memory
{                                                                       // Does not wait for !H_SYNC
   word *vramAddress;                                                       // Video memory address
   word clear = 0x0000;
   page = 0;
   int pageRegisterValue = 0;
   pageRegisterValue = current_video_mode + page;
   *pageRegister = (byte)(pageRegisterValue);
   printf("Cls 1 Set Page Register to %02X\n", pageRegisterValue);
   // Clear Page 0 all 16K
   vramAddress = (word *) VRAMOFFSET;
   while (true){
     *vramAddress = clear;
     vramAddress++;
     //printf("CLS1=%04X,%02X\n", vramAddress,*vramAddress);
     if ((word) vramAddress >= (VRAMTOP)) break;
   }
   page++;
   pageRegisterValue = current_video_mode + page;
   *pageRegister = (byte)(pageRegisterValue);
   printf("Cls 2 Set Page Register to %02X\n", pageRegisterValue);
   vramAddress = (word *) VRAMOFFSET;
   while (true){
     *vramAddress = clear;
     vramAddress++;
     //printf("CLS2=%04X,%02X\n", vramAddress,*vramAddress);
     if ((word) vramAddress >= (VRAMTOP)) break;
   }

   page++;
   pageRegisterValue = current_video_mode + page;
   *pageRegister = (byte)(pageRegisterValue);
   printf("Cls 3 Set Page Register to %02X\n", pageRegisterValue);
   vramAddress = (word *)VRAMOFFSET;
   while (true){
     *vramAddress = clear;
     vramAddress++;
     //printf("CLS3=%04X,%02X\n", vramAddress,*vramAddress);
     if ((word) vramAddress >= (VRAMTOP)) break;
   }
   page++;
   pageRegisterValue = current_video_mode + page;
   *pageRegister = (byte)(pageRegisterValue);
   printf("Cls 4 Set Page Register to %02X\n", pageRegisterValue);
   vramAddress = (word *)VRAMOFFSET;
   while (true){
     *vramAddress = clear;
     vramAddress++;
     //printf("CLS3=%04X,%02X\n", vramAddress,*vramAddress);
     if ((word) vramAddress >= (VRAMTOP)) break;
   }

   page = 0;
   pageRegisterValue = current_video_mode + page;
   *pageRegister = (byte)(pageRegisterValue);
   printf("Cls End Set Page Register to %02X\n", pageRegisterValue);
  
}

void PaletteInit()
{
  // Start at first Palette address, auto increments after writing R, G, B values
   //*crtcPaletteAddressWr = (byte) 0;
   for (int i = 0; i < PALETTE_SIZE; i++ ){
      *crtcPaletteAddressWr = (byte) i;
      *crtcPaletteColourValue = VGAPalette[i][0];  // Red
      *crtcPaletteColourValue = VGAPalette[i][1];  // Green 
      *crtcPaletteColourValue = VGAPalette[i][2];  // Blue
      //printf("PaletteInit Index=%d Address=%d R=%d, G=%d, B=%d\n", i, *crtcPaletteAddressWr, VGAPalette[i][0],VGAPalette[i][1],VGAPalette[i][2]);

   } 
   printf("PaletteInit Finish Address=%d\n", *crtcPaletteAddressWr);

}

void PaletteTest()
{
  // Start at first Palette address, auto increments after writing R, G, B values
  byte red = 0;
  byte green = 0;
  byte blue = 0;
  byte addr = 0;

   //*crtcPaletteAddressRd = (byte) 0;
   for (int j=0; j < PALETTE_SIZE; j++ ){
       printf("Test Index=%d \n", j);
      *crtcPaletteAddressRd = (byte) j;
      addr = *crtcPaletteAddressRd;
      red = *crtcPaletteColourValue;
      green = *crtcPaletteColourValue;
      blue = *crtcPaletteColourValue;
      // 
      //if (VGAPalette[j][0] != red) {
        printf("Palette Index=%d Address=%d, Red mismatch DAC=%d, Default=%d\n", j, addr, red, VGAPalette[j][0]);
      //}
      //if (VGAPalette[j][1] != green) {
        printf("Palette Index=%d  Address=%d, Green mismatch DAC=%d, Default=%d\n", j, addr, green, VGAPalette[j][1]);
      //}
      //if (VGAPalette[j][2] != blue) {
        printf("Palette Index=%d  Address=%d, Blue mismatch DAC=%d, Default=%d\n", j, addr, blue, VGAPalette[j][2]);
      //}
   } 
   printf("Palette Test Finish Address=%d\n", *crtcPaletteAddressRd);
   printf("Palette Test Completed \n");

}

// Mode 0 = Character 80 x 30, Mode 1 = Character 80 x 25, Mode 2 = Graphics 640 x 480 2 colour, Mode 3 = Graphics 320 x 200 256 Colour                                            
void CRTCInit(int video_mode)
{
  byte CRTC_REGISTERS[4][16]={
                   {  0x63,         // Character 80x30 Mode 0
                      0x50,         // R1 H Displayed 80
                      0x53,         // R2 H from was 53 
                      0x06,         // R3 H Sync Width 6
                      0x1F,         // R4 V Total 31 
                      0x14,         // R5 V Total Adjust 
                      0x1E,         // R6 V Displayed 30
                      0x1F,         // R7 V Sync Position 61
                      0x00,         // R8 Interlace mode - Non Interlaced
                      0x0F,         // R9 Maximum Scan Line Address 
                      0x00,         // R10 Cursor Start - Slow Blink C0 + Line 13 Start
                      0x00,         // R11 Cursor End - Slow Blink C0 + Line 15 Finish
                      0x00,0x00,    // R12,R13 Start Address
                      0x00,0x00},
                   {  0x63,         // Character 80x25 Mode 1 Capture default, R2 & R5 for Monitor
                      0x50,         // R1 H Displayed 80
                      0x53,         // R2 H from was 53 (52 for Monitor, 53 Capture card) 
                      0x0C,         // R3 H Sync Width 12
                      0x1B,         // R4 V Total 25 
                      0x03,         // R5 V Total Adjust (01 for Monitor, 03 Capture card)
                      0x19,         // R6 V Displayed 30
                      0x1A,         // R7 V Sync Position 61
                      0x00,         // R8 Interlace mode - Non Interlaced
                      0x0F,         // R9 Maximum Scan Line Address 
                      0x00,         // R10 Cursor Start - Slow Blink C0 + Line 13 Start
                      0x00,         // R11 Cursor End - Slow Blink C0 + Line 15 Finish
                      0x00,0x00,    // R12,R13 Start Address
                      0x00,0x00},
                   {  0x63,         // 640x 480 Mode 2 R0 H 62 to 64 Total 98 was 99 (Changed to 98 with 25Mhz as 59Hz not 60Hz)
                      0x50,         // R1 H Displayed 80
                      0x56,         // R2 H from was 53 Sync Position 86
                      0x06,         // R3 H Sync Width 6
                      0x40,         // R4 V Total 64 
                      0x05,         // R5 V Total Adjust (was 14)
                      0x3C,         // R6 V Displayed 60 (was 30)
                      0x3D,         // R7 V Sync Position 61
                      0x00,         // R8 Interlace mode - Non Interlaced
                      0x07,         // R9 Maximum Scan Line Address 
                      0x00,         // R10 Cursor Start - Slow Blink C0 + Line 13 Start
                      0x00,         // R11 Cursor End - Slow Blink C0 + Line 15 Finish
                      0x00,0x00,    // R12,R13 Start Address
                      0x00,0x00},
                   {  0x63,         // 320x 200 Mode 3 Capture default, based on mode 1 80x25 70 Hz, R2 & R5 for Monitor
                      0x50,         // R1 H Displayed 80
                      0x52,         // R2 H from was 53 (52 for Monitor, 53 Capture card) 
                      0x0C,         // R3 H Sync Width 12
                      0x6f,         // R4 V Total 101 
                      0x01,         // R5 V Total Adjust (01 for Monitor, 03 Capture card)
                      0x64,         // R6 V Displayed 100
                      0x65,         // R7 V Sync Position 65
                      0x00,         // R8 Interlace mode - Non Interlaced
                      0x03,         // R9 Maximum Scan Line Address 03 = 4 Raster Lines 4 x 100 lines
                      0x00,         // R10 Cursor Start - Slow Blink C0 + Line 13 Start
                      0x00,         // R11 Cursor End - Slow Blink C0 + Line 15 Finish
                      0x00,0x00,    // R12,R13 Start Address
                      0x00,0x00},
  };   // R14,R15 Cursor Address
  current_video_mode = 0;              
  byte *CRTCAddressRegister = (byte *) CRTCAR;
  byte *CRTCDataRegister = (byte *) CRTCDR;
  byte *GraphicsColour = (byte *) COLOURLATCH;

  for (int r = 0; r <= 15; r++)
  {
    *CRTCAddressRegister = (byte) r;
    *CRTCDataRegister = (byte) CRTC_REGISTERS[video_mode][r];
  }
  // Pre-calculate address for each character row (y), exclude VRAMOFFSET will add after page calcs in rawAddressToPaged()
  for (unsigned int i = 0; i<=59; i++){
    yLookup[i] = (word) (i * BYTESPERROW);
    //printf("y Lookup i=%d, Lookup=%X \n\r",i, yLookup[i]);
  }
  // Optimize pixelmask for plotPoint
  for (word i = 0; i<=7; i++){
    pixelmaskLookup[i] =  (byte) 1 << (7 - i);
  }

  if (video_mode == 0){
     // Character 80 x 30 60Hz 16 Colour with Page 0 selected
     *pageRegister = (byte)(VIDEO_MODE0);
     current_video_mode = (byte)(VIDEO_MODE0);
  } else if (video_mode == 1){
     // Character 80 x 25 70Hz 16 Colour with Page 0 selected
     *pageRegister = (byte)(VIDEO_MODE1);
     current_video_mode = (byte)(VIDEO_MODE1);
  } else if (video_mode == 2){
     // HI Res 640x480 2 Colour with Page 0 selected
     *pageRegister = (byte)(VIDEO_MODE2);
     current_video_mode = (byte)(VIDEO_MODE2);
     *crtcPalettePixelMask = (byte) 0xFF;
     // Set to White Foreground, Black background after setting Graphics Mode
     *GraphicsColour = (byte) 0xF0;
  } else if (video_mode == 3){
     // HI Colour 320x200 256 Colour with Page 0 selected
     *pageRegister = (byte)(VIDEO_MODE3);
     current_video_mode = (byte)(VIDEO_MODE3);
     *crtcPalettePixelMask = (byte) 0xFF;
  }
  printf("Init Video Mode=%d, Current Video mode=%d\n", video_mode, current_video_mode);

  ClearScreenGraphics();
  DisableCursor();
  printf("After CLS Init Video Mode=%d, Current Video mode=%d\n", video_mode, current_video_mode);

}                                             
void plotPoint(int x, int y, byte colour)                                     // Routine to write pixel data to video RAM
{  
   byte *vramAddress;                                                             // Video memory address
   word address;
   int pixelBit = x & 7;                                                     // The bit from left to right 0 left 7 right
   byte newPage = 0;
   address = (word)((x - pixelBit) + (y & 7) + yLookup[y >> 3]);

   vramAddress = (byte *) rawAddressToPaged(address);
   if (colour > 0){
     //int data = *vramAddress | pixelmaskLookup[pixelBit];
     //printf("Writing address ,%X, data ,%X\n\r", address, data);
     *vramAddress = *vramAddress | pixelmaskLookup[pixelBit];
   } else {
     *vramAddress = *vramAddress & ~pixelmaskLookup[pixelBit];
   }

}

// Bresenham's line algorithm
void DrawLine(int x0, int y0, int x1, int y1, byte colour)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    // No optimisation for vertical or horizontal lines
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
/*
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
*/
int clamp(int value, int min, int max) { 
	return (value<min) ? min : (value>max) ? max : value; 
}

// Check for Key press for wait loops, and for key release, return Nanocomp key code or $FF if nothing pressed during wait
byte PollKey(int wait){

/* Initialise Pointers for PIA Ports A & B and Control Registers A & B */
    unsigned int waitcount = wait;
    //printf("Start Wait=%d \n", waitcount);
    byte *portA;
    portA = PIA_PORTA;
    byte *portB;
    portB = PIA_PORTB;

    byte *ctrlA;
    ctrlA = PIA_CTRLA;
    byte *ctrlB;
    ctrlB = PIA_CTRLB;
    int row;

   // Do While key not pressed
   do {
        //     Set up I/O Ports
        // Select Data Direction Registers
        *ctrlA=0x00;
        *ctrlB=0x00;
        // Port A Input, Port B Lower nibble output
        *portA=0x00;
        *portB=0x0F;
        // Select Data Registers
        *ctrlA=0x04;
        *ctrlB=0x04;
        
        //GetKey2 - (want Port B to select 0,1,2,or 3 from 74LS145 Decoder Open Collector)
        for (row=0; row<4; row++){ 
            *portB= (byte)row;
            // ~ is complement, key pressed will pull port bit to zero, so ~ makes this 1 - key pressed
            if (~*portA!=0){
                break;
            }
        }
        waitcount--;
    } while (~*portA==0 && waitcount>0 );
    
    byte returnChar = (byte) 0xff; // We return 0xff if no character pressed
    
    if (waitcount>0){
        
        int msNibble=row << 4; // shift left by 4 (multiply by 16)
        int portABits=~(int)*portA; // Complement of Port,key pressed bit=1
        int testBit=1;
        int bit;
        
        for (bit=0; bit <8; bit++ ){  // If bit 0 set bit=0, bit 1 set bit=1 etc
            if (testBit&portABits) {
                break;
            }
            testBit = testBit<<1;
        }
        returnChar=(byte) (msNibble+bit);

        while (~*portA!=0 && waitcount >0 ){
            waitcount--;
        }
    }
    
    return returnChar;
}



// This waits until a key is pressed then released, returning Nanocomp key code
byte GetKey(){

    byte *portA;
    portA = PIA_PORTA;
    byte *portB;
    portB = PIA_PORTB;

    byte *ctrlA;
    ctrlA = PIA_CTRLA;
    byte *ctrlB;
    ctrlB = PIA_CTRLB;

    int row;

    /* Do While waiting for key to be pressed 
    *  When a key is pressed it pulls the portA bit to 0 so ~ complements port value so key press=1
    *  
    */
    do {
        //     Set up I/O Ports
        // Select Data Direction Registers
        *ctrlA=0x00;
        *ctrlB=0x00;
        // Port A Input, Port B Lower nibble output
        *portA=0x00;
        *portB=0x0F;
        // Select Data Registers
        *ctrlA=0x04;
        *ctrlB=0x04;
        
        //GetKey2 - (want Port B to select 0,1,2,or 3 from 74LS145 Decoder Open Collector)
        for (row=0; row<4; row++){ 
            *portB= (byte)row;
            // ~ is complement, key pressed will pull port bit to zero, so ~ makes this 1
            if (~*portA!=0){
                break;
            }
        }
    } while (~*portA==0);

    int msNibble=row << 4; // shift left by 4 (multiply by 16)
    int portABits=~(int)*portA; // Complement of Port,key pressed bit=1
    int testBit=1;
    int bit;
    
    for (bit=0; bit <8; bit++){  // If bit 0 set bit=0, bit 1 set bit=1 etc
        if (testBit&portABits) {
            break;
        }
        testBit = testBit<<1;
    }
    byte returnChar=(byte) (msNibble+bit);

    while (~*portA!=0){
        continue;
    }
    //wait 10ms
    Delay(4000);
    
    return returnChar;
}

// Wait for character for wait attempts then return $FF if none pressed or the ASCII code
byte PollChr(int wait){
    byte returnChar = PollKey(wait);
    if (returnChar != (byte) 0xff) {
        int keyIndex = (int) returnChar;
        returnChar = keyCharLookup[keyIndex];
    }
    return returnChar;
}

// Wait for a character to be entered on Keypad and convert to ASCII
byte GetChr(){
    byte returnChar = GetKey();
    if (returnChar != (byte) 0xff) {
        int keyIndex = (int) returnChar;
        returnChar = keyCharLookup[keyIndex];
    }
    return returnChar;
}

// Need to calibrate based on current clock 4 Mhz, w wait in ms
void Delay(int w)
{
    int m;
    int n;
    for( n = 0; n < w; n++){
       for( m = 0; m < 36; m++){
    	   continue;
       }
    }
}

// Call the Nanocomp Monitor clear screen routine
void ClearScreen ()
{
    asm
    {
        lbsr $f6de
    }
}

// Take 4 bit nibble and convert to ASCII for 0-F
byte HexToAscii(byte inputByte){
    byte asciiChar = 0x30; // Numeric in range 0x30 to 0x39
    if (inputByte > 0x09) {
        asciiChar = 0x37; //Alpha in range 0x41-0x46 for dec 10 (A) get 0x37 + 0x0A = 0x41
    }
    asciiChar=asciiChar + inputByte;
    return asciiChar;
}

// Output character and define colour, when colour is 00 it remains the same
void PrintColChr (byte colour, byte ch)
{
    asm
    {
        pshs a,b
        ldb  :ch
        lda  :colour
        lbsr $f695
        puls a,b
    }
}
// Basic output character without changing colour
void PrintChr (byte ch)
{
    asm
    {
        pshs b
        ldb  :ch
        lda  #$f2
        lbsr $f695
        puls b
    }
}
/*
// Will draw block of colour at current DISPROW and DISPCOL, faster version of Macro ADD_BLOCK
// Will increment DISPROW but won't wrap at end of line (OK for Tetris)
void AddBlock (byte colour)
{
    asm
    {
        lda  :colour
        ldx  #$F793 ; Get base added for Column Address table in ROM
        ldb  $7FC0  ; Get DISPROW which is used for offset
        aslb        ; Multiple row x 2 to convert rows to bytes
        leax [b,x]  ; Load X with the address in the table for the given row, note < 128 so sign not an isse
        ldb  $7FC1  ; Now get DISPCOL the column (x)
        aslb        ; Multiply by 2 to allow for 2 bytes per character (colour + chracter)
        abx         ; Add Column to row address in X, note values over 64 will be 8 bits signed so need abx which b is unsigned
        ldb  #$F9   ; Draw left half of tetronimo cell
        std  ,x++   ;
        ldb  #$FA   ;
        std  ,x++   ; Draw right half of tetronimo cell
        inc  $7FC1  ; Add 2 to DISPCOL to move to next cell
        inc  $7FC1  ; Add 2 to DISPCOL to move to next cell
    }
}

void ClearBlock ()
{
    asm
    {
        ldx  #$F793 ; Get base added for Column Address table in ROM
        lda  $7FC0  ; Get DISPROW which is used for offset
        asla        ; Multiple row x 2 to convert rows to bytes
        leax [a,x]  ; Load X with the address in the table for the given row, note < 128 so sign not an isse
        ldb  $7FC1  ; Now get DISPCOL the column (x)
        aslb        ; Multiply by 2 to allow for 2 bytes per character (colour + chracter)
        abx         ; Add Column to row address in X, note values over 64 will be 8 bits signed so need abx which b is unsigned
        ldb  #$20   ; 
        stb  1,x    ; Clear left half of tetronimo cell
        stb  3,x    ; Clear right half of tetronimo cell
        inc  $7FC1  ; Add 2 to DISPCOL to move to next cell
        inc  $7FC1  ; Add 2 to DISPCOL to move to next cell
    }
}
*/
// Allows CMOC to use Nanocomp screen output routine CHROUT when registered with  setConsoleOutHook(newOutputRoutine)
void newOutputRoutine()
{
    asm
    {
        pshs a,b
        tfr  a,b
        lda  #$f2
        lbsr $f653
        puls a,b
    }
}

// Allows CMOC to use Nanocomp serial output routine when registered with  setConsoleOutHook(serialOutputRoutine)
void serialOutputRoutine()
{
    asm
    {
        lbsr $ffa6
    }
}

// Move the Nanocomp display row/column to supplied values
void ncMove (int ncRow, int ncCol){
  byte *row;
  row=DISP_ROW;  // Address for DISPROW
  byte *col;
  col=DISP_COL;  // Address for DISPCOL
  *row=(byte)ncRow;    // Set values, no validation at the moment
  *col=(byte)ncCol;
}

 // Return to Nanocomp Monitor via SWI
void returnNC(){
  asm
  {
    swi
  }
}

// Waits for the end of frame when Display Enable is high 1.6ms between 16.7ms page frame enables PIA settings
void WaitForScreenBlankInit()
{
    asm
    {
        lbsr $F75B
    }
}

// Waits for the end of frame when Display Enable is high 1.6ms between 16.7ms page frame, quicker routine
void WaitForScreenBlank()
{
    asm
    {
@TESTDE 
              lda   #$80 ; Mask PB7 on Port B of PIA
              bita  $D001        ; Test PB7 for Display Enable, when 1 then screen blanked
              beq   @TESTDE
              nop
              nop
              bita  $D001        ; Test PB7 again
              beq   @TESTDE       ; If 0 then only HS or end of VS, so continue to wait for DE=1 for more than 6ms

    }
}

// Disable cursor
void DisableCursor()
{
    asm
    {
        lbsr $F74D
    }
}


//Define Tetris custom characters and replace F0-FF in CG RAM
/* void CopyCharGenRAM (){
  byte charGen[16][16]=
    {{0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E},         // F0 Side vertical bar 4 wide, 1 in left
     {0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78,0x78},         // F1 Side vertical bar 4 wide, 1 in right
     {0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},         // F2 Low bar 4 deep 1 down
     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00},         // F3 Top bar 4 deep 1 up
     {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},         // F4 Background
     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x0F,0x1F,0x1F,0x1E},         // F5 Top Right Corner
     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xF0,0xF8,0xF8,0x78},         // F6 Top Left Corner
     {0x1E,0x1F,0x1F,0x0F,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},         // F7 Lower Left Corner
     {0x78,0xF8,0xF8,0xF0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},         // F8 Lower Right Corner
     {0x00,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x00},         // F9 Tetris Square Left
     {0x00,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0x00},         // FA Tetris Square Right
     {0x00,0x7F,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7F,0x00},         // FB Tetris Square Outline Left
     {0x00,0xFE,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0xFE,0x00},         // FC Tetris Square Outline Right
     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},         // FD Blank
     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},         // FE Blank
     {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}          // FF Blank
  };
  
  byte *cgram;
  cgram = CG_RAM_F0;

  int c, l;
  for (c = 0; c < 16; c++) {
    for (l = 0; l < 16; l++) {
      // printf("R%dC%d B%d  ", c, l, (int) charGen[c][l]);
      *cgram = charGen[c][l];
      cgram++;
    }
  }
  
}
*/