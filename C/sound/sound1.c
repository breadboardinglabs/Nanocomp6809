//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Nanocomp 6809 Programmable Sound Generator & Joystick Test Program
// 
// cd /mnt/c/6809/code/sound
// cmoc --void-target --org=0000 --data=7200 --limit=7f80 -i --intdir=int --srec sound1.c nanocomp.c
// --limit is required to warn if hitting the top of memory for stack
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmoc.h"
#include "cmocextra.h"
#include "nanocomp.h"


int main(int argc, char **argv)
{
  // Enables debug output from printf() to serial port
  setConsoleOutHook(serialOutputRoutine);
  printf("Test Serial Debug \n\r");
  // Set Display to Character VGA with default VGA Palette
  CRTCInit(0);
  PaletteInit();

  sound_init();
  printf("\n\rStart sound \n\r");

  // Siren Fig 27
  // Set start frequency
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, 140);
  sound_address_write(PSG_CHA_TONE_PERIOD_COARSE_REG, 0);
  // Only enable Tone A
  sound_address_write(PSG_ENABLE_REG, PSG_MIXER_NOISES_DISABLE | PSG_MIXER_TONE_B_DISABLE | PSG_MIXER_TONE_C_DISABLE);
  // Set A Channel to maximum
  sound_address_write(PSG_CHA_AMPLITUDE, 15);
  Delay(350);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, 250);
  Delay(350);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, 140);
  Delay(350);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, 250);
  Delay(350);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, 140);
  Delay(350);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, 250);
  Delay(350);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, 140);
  Delay(350);
  sound_address_write(PSG_CHA_AMPLITUDE, 0);
  Delay(1000);
  printf("End sound \n\r");


  returnNC();  // Causes SWI for Nanocomp to return to monitor
  return 0;
}