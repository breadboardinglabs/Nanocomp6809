//#pragma org 0x1800
//#pragma data 0x1000

// cmoc -S --org=1800 --data=1000 keypad.c 
// lwasm --list=keypad.lst --obj --symbols --map -o=keypad.o keypad.s
// /mnt/c/temp/6809DASM/6809dasm -i/mnt/c/6809/code/keypad.bin -d/mnt/c/6809/code/keypaddata.txt -l > /mnt/c/6809/code/keypad_disasm.txt
// cmoc --void-target keypad.c 

#include "nanocomp.h"

// Keys Hex  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  L  S CN   G  M  R  I
//  Code    22 24 02 12 14 00 10 04 01 11 03 13 23 33 21 20 05 15 25  35 31 30 32
//           00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0A, 0B, 0C, 0D, OE, 0F, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 1A, 1B, 1C, 1D, 1E, 1F, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 2A, 2B, 2C, 2D, 2E, 2F, 30, 31, 32, 33, 35
//          '5','8','2','A','7','L',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','6','9','3','B','4','S',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','F','E','0','C','1','N',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','R','M','I','D',' ','G'
// Nanocomp Keypad to ASCII Lookup
unsigned char keyCharLookup[0x36]={'5','8','2','A','7','L',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                      '6','9','3','B','4','S',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                      'F','E','0','C','1','N',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                      'R','M','I','D',' ','G'};
    
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

// Need to calibrate based on current clock 4 Mhz
void Delay(int w)
{
    int n;
    for( n = 0; n < w; n++)
	  continue;
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
void CopyCharGenRAM (){
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
