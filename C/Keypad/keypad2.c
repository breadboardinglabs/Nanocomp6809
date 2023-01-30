// cmoc --srec --org=0000 --data=1000 --void-target keypad2.c 
// lwasm --list=keypad.lst --obj --symbols --map -o=keypad.o keypad.s
// /mnt/c/temp/6809DASM/6809dasm -i/mnt/c/6809/code/keypad.bin -d/mnt/c/6809/code/keypaddata.txt -l > /mnt/c/6809/code/keypad_disasm.txt
// cmoc --void-target keypad.c 

/* Nanocomp 6809 Key Definitions for original keypad */
#define KEY_CN 0x25
#define KEY_G 0x35
#define KEY_I 0x32
#define KEY_L 0x05
#define KEY_M 0x31
#define KEY_P 0x15 /* P Punch replaced with S Save */
#define KEY_S 0x15
#define KEY_R 0x30

#define KEY_0 0x22
#define KEY_1 0x24
#define KEY_2 0x02
#define KEY_3 0x12
#define KEY_4 0x14
#define KEY_5 0x00
#define KEY_6 0x10
#define KEY_7 0x04
#define KEY_8 0x01
#define KEY_9 0x11
#define KEY_A 0x03
#define KEY_B 0x13
#define KEY_C 0x23
#define KEY_D 0x33
#define KEY_E 0x21
#define KEY_F 0x20


/* Nanocomp 6809 7 Segment Display Definitions */
#define SEG_0 0x7E
#define SEG_1 0x06
#define SEG_2 0x5B
#define SEG_3 0x1F
#define SEG_4 0x27
#define SEG_5 0x3D
#define SEG_6 0x7D
#define SEG_7 0x0E
#define SEG_8 0x7F
#define SEG_9 0x3F
#define SEG_A 0x6F
#define SEG_B 0x75
#define SEG_C 0x78
#define SEG_D 0x57
#define SEG_E 0x79
#define SEG_F 0x69
#define SEG_G 0x7C
#define SEG_M 0x6E /* (upside down U) */
#define SEG_P 0x6B
#define SEG_S 0x3D /* (Same as 5) */
#define SEG_U 0x76
#define SEG_X 0x67
#define SEG_Y 0x37
#define SEG_DASH 0x01
#define SEG_SPACE 0x00

// Should these be segment 0 and 5??
#define SEG_SEGMENT1 0x04
#define SEG_SEGMENT6 0x09


#define PIA_PORTA 0xD000
#define PIA_PORTB 0xD001
#define PIA_CTRLA 0xD002
#define PIA_CTRLB 0xD003

#define PIA_DATA_DIRECTION_REG 0x00
#define PIA_DATA_REG 0x04

#ifndef _CMOC_BASIC_TYPES_
#define _CMOC_BASIC_TYPES_
typedef unsigned char byte;
typedef signed char   sbyte;
typedef unsigned int  word;
typedef signed int    sword;
typedef unsigned long dword;
typedef signed long   sdword;
#endif


/* Array to convert keycode into Hex Nibble */
byte keyCode[16]={KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,
                  KEY_8,KEY_9,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F};

/* Array to convert Hex nibble into 7 Segment display code */
byte segmentCode[16]={SEG_0,SEG_1,SEG_2,SEG_3,SEG_4,SEG_5,SEG_6,SEG_7,
                      SEG_8,SEG_9,SEG_A,SEG_B,SEG_C,SEG_D,SEG_E,SEG_F};
/* Can't force Array to specific address so that we can use DISPRESH */
byte displayBuffer[6]={SEG_SPACE,SEG_SPACE,SEG_SPACE,SEG_SPACE,SEG_SPACE,SEG_SPACE};

/* Initialise Pointers for PIA Ports A & B and Control Registers A & B */
byte *portA=PIA_PORTA;
byte *portB=PIA_PORTB;

byte *ctrlA=PIA_CTRLA;
byte *ctrlB=PIA_CTRLB;

// Need to calibrate based on current clock 4 Mhz
void Delay(int w)
{
    int n;
    for( n = 0; n < w; n++)
	  continue;
}
 
/* Call the current Nanocomp Monitor 7 Segment Display routine  */
void DisplayRefresh ()
{
    // Set up PIA I/O Ports
    // Select Data Direction Registers
    *ctrlA=PIA_DATA_DIRECTION_REG;
    *ctrlB=PIA_DATA_DIRECTION_REG;
    // Port A Direction Output bits 0-6 for 7 Segments
    *portA=0x7F;
    // Port B Direction Output lower nibble bits 0-3 for 74LS145 decoder (0-9) Seg 1 at 4, Seg 6 at 9
    *portB=0x0F;
    // Select PIA Data Registers
    *ctrlA=PIA_DATA_REG;
    *ctrlB=PIA_DATA_REG;
    
    int bufferIndex = 0;
    int display=0;
    // For each Segment output decoder value on Port B and complement of segment value from display buffer to Port A
    for (display=SEG_SEGMENT1; display<=SEG_SEGMENT6; display++){
        *portA=~displayBuffer[bufferIndex++]; // Need complement of Segment value from Display Buffer
        *portB=(byte)display;  // Set the decoder value for display segment
        Delay(80);
    }

}

void ClearDisplay ()
{
    displayBuffer[0]=SEG_SPACE;
    displayBuffer[1]=SEG_SPACE;
    displayBuffer[2]=SEG_SPACE;
    displayBuffer[3]=SEG_SPACE;
    displayBuffer[4]=SEG_SPACE;
    displayBuffer[5]=SEG_SPACE;
    /* int n;
    for( n = 0; n < 6; n++)
	  displayBuffer[n]=SEG_SPACE;
    */
}


void DisplayHex(byte inputByte, int offset ){
    if (offset>4) {
        offset=4;
    }
    byte highNibble=inputByte >> 4;
    byte lowNibble=inputByte&0x0F;
    displayBuffer[offset]=segmentCode[highNibble];
    displayBuffer[offset+1]=segmentCode[lowNibble];
}

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
        DisplayRefresh();
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

int main()
{
    displayBuffer[0]=SEG_6;
    displayBuffer[1]=SEG_8;
    displayBuffer[2]=SEG_0;
    displayBuffer[3]=SEG_9;
    displayBuffer[4]=SEG_SPACE;
    displayBuffer[5]=SEG_DASH;

    byte keyPressed;
    while (1) {
        keyPressed=GetKey();
        ClearDisplay();
        DisplayHex(keyPressed,4);
    }    
    return 0;
}

