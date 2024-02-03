//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Nanocomp 6809 256 Colour 320 x 200 VGA Palette Test Program
// 
// cd /mnt/c/6809/code/256colour
// cmoc --void-target --org=0000 --data=7200 --limit=7f80 -i --intdir=int --srec test256c.c nanocomp.c
// --limit is required to warn if hitting the top of memory for stack
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "test256c.h"

#include "cmoc.h"
#include "cmocextra.h"
#include "nanocomp.h"


int main(int argc, char **argv)
{
  // Enables debug output from printf() to serial port
  setConsoleOutHook(serialOutputRoutine);
  printf("Test Serial Debug \n\r");
  // Don't want to init 256C yet until done all latches etc
  //CRTCInit(3);
  
  // Initialises PIA so we can check Screen Blanking
  //WaitForScreenBlankInit();
  //CRTCInit(3);
  
  // Set Video DAC to default VGA Palette
  PaletteInit();


  // Check current video DAC values against default
  //PaletteTest();

  // Output blocks of colour for the Palette and switch to video mode 3 Hi Colour 320x200
  CRTCInit(3);

  byte *colourTest = 0x8000;
  byte *colourTest2 = 0x8280;
  // This will need to do bigger blocks, but this will be 1 character wide, 1 character height
  for (int i = 0; i < 255; i++ ){
      // This does 4 pixels on line 0, then 4 on line 1
      *colourTest++ = (byte) i;
      /**colourTest++ = (byte) i;
      *colourTest++ = (byte) i;
      *colourTest++ = (byte) i;*/
      // Next Block of 4 pixels on line 2, then 4 on line 3
/*
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;
      *colourTest2++ = (byte) i;*/
  }   
  /*byte *pngPalette = 0x4000;
  byte *crtcPaletteAddressWr = CRTC_DAC_ADD_WR;
  byte *crtcPaletteColourValue = CRTC_DAC_COL_VALUE;

  for (int i = 0; i < 239; i++ ){
      *crtcPaletteAddressWr = (byte) i;
      *crtcPaletteColourValue = *pngPalette++;  // Red
      *crtcPaletteColourValue = *pngPalette++;  // Green 
      *crtcPaletteColourValue = *pngPalette++;  // Blue
      //printf("PaletteInit Index=%d Address=%d R=%d, G=%d, B=%d\n", i, *crtcPaletteAddressWr, VGAPalette[i][0],VGAPalette[i][1],VGAPalette[i][2]);

   } 
  
  */
  // Set VRAM to page 0 for debugging using current video mode
 //*pageRegister = (byte)(current_video_mode);

  returnNC();  // Causes SWI for Nanocomp to return to monitor
  return 0;
}
