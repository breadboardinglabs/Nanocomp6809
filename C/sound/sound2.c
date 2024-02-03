//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Nanocomp 6809 Programmable Sound Generator & Joystick Test Program
// 
// cd /mnt/c/6809/code/sound
// cmoc --void-target --org=0000 --data=7200 --limit=7f80 -i --intdir=int --srec sound2.c nanocomp.c
// --limit is required to warn if hitting the top of memory for stack
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmoc.h"
#include "cmocextra.h"
#include "nanocomp.h"


int main(int argc, char **argv)
{
  // Enables debug output from printf() to serial port
  //setConsoleOutHook(serialOutputRoutine);
  //printf("Test Serial Debug \n\r");
  // Set Display to Character VGA with default VGA Palette
  CRTCInit(0);
  ClearScreen();
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
  

  // Gunshot Fig 28
  sound_address_write(PSG_NOISE_PERIOD_REG, 15);
  sound_address_write(PSG_ENABLE_REG, PSG_MIXER_TONES_DISABLE | PSG_MIXER_NOISE_C_DISABLE | PSG_MIXER_NOISE_B_DISABLE);
  sound_address_write(PSG_CHA_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
  sound_address_write(PSG_CHB_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
  sound_address_write(PSG_CHC_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
  sound_address_write(PSG_ENV_PERIOD_COARSE_REG, 8);
  sound_address_write(PSG_ENV_SHAPE_CYCLE, PSG_ENVELOPE_CONTROL_DECAY);
  Delay(2000);
 
  // Explosion Fig 29
  sound_address_write(PSG_NOISE_PERIOD_REG, 31);
  sound_address_write(PSG_ENABLE_REG, PSG_MIXER_TONES_DISABLE | PSG_MIXER_NOISE_C_DISABLE | PSG_MIXER_NOISE_B_DISABLE);
  sound_address_write(PSG_CHA_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
  sound_address_write(PSG_CHB_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
  sound_address_write(PSG_CHC_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
  sound_address_write(PSG_ENV_PERIOD_COARSE_REG, 40);
  sound_address_write(PSG_ENV_SHAPE_CYCLE, PSG_ENVELOPE_CONTROL_DECAY);
  Delay(4000);

  // Tetris start
  // E B C D C B A
  // A C E D C B
  // C D E C A A
  // D F A G F E
  // C E D C B
  // B C D E C A A
  byte A=142; // A4 440 142
  byte B=127;
  byte C=119; // C5
  byte D=106;
  byte E=95;
  byte F=89;
  byte G=80; 
  byte A5=71; // A5 

  sound_address_write(PSG_CHA_TONE_PERIOD_COARSE_REG, 0);
  // Only enable Tone A 
  sound_address_write(PSG_ENABLE_REG, PSG_MIXER_NOISES_DISABLE | PSG_MIXER_TONE_B_DISABLE | PSG_MIXER_TONE_C_DISABLE);
  sound_address_write(PSG_CHA_AMPLITUDE, 15);
  // Korobeiniki AKA Tetris theme
  // E B C D C B A
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, E);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, B);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, D);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, B);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, A);
  Delay(180);
  sound_address_write(PSG_CHA_AMPLITUDE, 0);
  Delay(20);
  sound_address_write(PSG_CHA_AMPLITUDE, 15);
  
  // A C E D C B
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, A);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, E);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, D);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, B);
  Delay(400);
  // C D E C A A
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, D);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, E);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, A);
  Delay(180);
  sound_address_write(PSG_CHA_AMPLITUDE, 0);
  Delay(20);
  sound_address_write(PSG_CHA_AMPLITUDE, 15);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, A);
  Delay(200 );

  sound_address_write(PSG_CHA_AMPLITUDE, 0);
  Delay(400);
  sound_address_write(PSG_CHA_AMPLITUDE, 15);

  // D F A G F E
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, D);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, F);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, A5);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, G);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, F);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, E);
  Delay(300);

  // C E D C B
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, E);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, D);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, B);
  Delay(180);
  sound_address_write(PSG_CHA_AMPLITUDE, 0);
  Delay(20);
  sound_address_write(PSG_CHA_AMPLITUDE, 15);

  // B C D E C A A
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, B);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(200);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, D);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, E);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, C);
  Delay(400);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, A);
  Delay(180);
  sound_address_write(PSG_CHA_AMPLITUDE, 0);
  Delay(20);
  sound_address_write(PSG_CHA_AMPLITUDE, 15);
  sound_address_write(PSG_CHA_TONE_PERIOD_FINE_REG, A);
  Delay(200);

  sound_address_write(PSG_CHA_AMPLITUDE, 0);
  printf("End sound \n\r");
  byte joystickValue=0;
  
  while(true)  {
    joystickValue = read_joystick();
    ncMove (10, 40);
    printf("joystickValue=%02X\n",joystickValue);

    if (joystickValue & 0b00010){
      ncMove (15, 37);
      PrintColChr (0x2F, 0x4c); // L   
    } else {
      ncMove (15, 37);
      PrintColChr (0xF2, 0x4c);    
    }
    if (joystickValue & 0b00100){
      ncMove (15, 41);
      PrintColChr (0x2F, 0x52); // R
    } else {
      ncMove (15, 41);
      PrintColChr (0xF2, 0x52);    
    }
    if (joystickValue & 0b01000){
      ncMove (16, 39);
      PrintColChr (0x2F, 0x44);    //D
    } else {
      ncMove (16, 39);
      PrintColChr (0xF2, 0x44);    
    }
    if (joystickValue & 0b10000){
      ncMove (14, 39);
      PrintColChr (0x2F, 0x55);    //U
    } else {
      ncMove (14, 39);
      PrintColChr (0xF2, 0x55);    
    }
    // Complement value as button/fire 0 when pressed
    if (~joystickValue & 0b00001){
      ncMove (15, 39);
      PrintColChr (0x2F, 0x46); // F
      sound_address_write(PSG_NOISE_PERIOD_REG, 15);
      sound_address_write(PSG_ENABLE_REG, PSG_MIXER_TONES_DISABLE | PSG_MIXER_NOISE_C_DISABLE | PSG_MIXER_NOISE_B_DISABLE);
      sound_address_write(PSG_CHA_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
      sound_address_write(PSG_CHB_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
      sound_address_write(PSG_CHC_AMPLITUDE, PSG_AMPLITUDE_CONTROL_MODE);
      sound_address_write(PSG_ENV_PERIOD_COARSE_REG, 8);
      sound_address_write(PSG_ENV_SHAPE_CYCLE, PSG_ENVELOPE_CONTROL_DECAY);      
      Delay(1000);
      sound_address_write(PSG_CHA_AMPLITUDE, 0);
    } else {
      ncMove (15, 39);
      PrintColChr (0xF2, 0x46);    
    }

  }

  returnNC();  // Causes SWI for Nanocomp to return to monitor
  return 0;
}