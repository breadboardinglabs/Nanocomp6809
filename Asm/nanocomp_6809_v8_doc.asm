;C:\Users\Dave\Documents\_6809\asm6809-2.12-w64\asm6809.exe -B -l nanocomp_6809_v8_doc.txt nanocomp_6809_v8_doc.asm -o nanocomp_6809_v8_doc.bin
                      ; Condition Code Register
                      ; CC  7  6  5  4  3  2  1  0
                      ;     E  F  HC I  N  Z  O  C
                      ;
                      ; 0 C Carry/Borrow
                      ; 1 O Overflow
                      ; 2 Zero
                      ; 3 N Negative
                      ; 4 I IRQ Mask
                      ; 5 HC Half Carry
                      ; 6 F FIRQ Mask
                      ; 7 E Entire State on Stack
;
; Nanocomp 6809 Monitor V8 with CRTC Initialisation and Character Generator RAM loading from E000-EFFF of 8K EEPROM
; Nanocomp 6809 Monitor V3 with MC6850 ACIA and Motorola SREC Load and Save using 64K Memory Map
; with fix to DISPRESH to prevent ghosting
; Fix moves all routines below back by 5 Bytes. Reset Vector changes from 7D89 to 7D8E (+5)
;
;-           After power up, display - wait for command
;RST Reset   Is hard reset bringing /RST line low via switch debounce circuit
;AB  Abort   Is Non Maskable Interupt (NMI), vector at 13F2
;G  Go       Displays G to acknowledge command, enter 4 digit hex address and will run on 4th digit
;CN Continue After SWI or Abort (NMI) press CN to continue, values on the stack will be pulled including program counter
;M  Memory   Display M in far right display (upside down U),
;            4 digit Hex entered in left 4 digits, 
;            displays memory contents in right two digits
;            change by typing in new digits enter in from right shift to left
;            Press I to save and move to next 
;            No changes press I to advance or AB to abort
;            Invalid address or hex bytes will return to monitor - prompt
;S  Save     SAVE range to serial port in V2+ in S19/SREC format 7DCB 
;            Display S to prompt for Start address
;            Display F to prompt for for finish address
;            Displays F when finished
;L  Load     LOAD from serial port in V2+ 7DC4
;            When finished Display F Finished or E for Hex error or C for Checksum error
;AB Abort    Return to monitor start via NMI if not changed NMI vector
;R  Register Display register values via the pushed values on the stack (automatic after SWI)
;            Right digits show 
;            	C Condition Code register
;            	A ACC A
;            	b ACC B
;               d direct page register
;            	H X Register
;               Y Y Register
;            	U User Stack Pointer
;            	P Program Counter
;            	S Hardware Stack Pointer
;               Press I between values
;               After SP displayed returns to Monitor, AB to Abort to monitor

; Example programs
; F800 Hex to decimal converter
; G F800 Type L then decimal No then I to display Hex, press I to start again
;        Press P for hex to decimal then I

; FA00 Calculate offsets
; G FA00 Displays S enter start address  then I
;        Displays d then enter destination then twos complement offset displayed
;        If outside range displays --, press I for next

; FA80 Mastermind
;        I displayed after thinking of 4 digit number 4 digits 0-7
;        Press I enter guess 2 digits display Correct digits and correct numbers in wrong places (Bulls and Cows)
;        Press I for next number, after 4 bulls press I and displays number of tries

; F940 Duckshoot
; G F940 Store 0020 at memory 0000 and 0001 (1000 and 1001 for 6809)
;        Press number 1-6 for duck until none are left.

; Common Subroutines

; FC7B DISPRESH Displays 6 digits from 13FA (left) to 13FF (right) 
; Bit Segments, note that A-G seqments in 1981 design are different from modern LED displays (E&F swapped)
;       b3 A
;     -------
; b5 E|     | b1 B
;     | b0 G|
;     -------
; b6 F|     | b2 C
;     |     |
;     -------
;       b4 D  
; 7 Segment display values (see segxx EQU definitions below)
;  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  -  G  M  P  S  U  X  Y
; 7E 06 5B 1F 27 3D 7D 0E 7F 3F 6F 75 78 57 79 69 01 7C 6E 6B 3D 76 67 37

; FC20 GETKEY Scans for key and refreshes display until key pressed. Then waits to be released and returns key in A
; Key Codes in Hex (see keyxx EQU definitions below)

;  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  L SP CN G  M  R  I
; 22 24 02 12 14 00 10 04 01 11 03 13 23 33 21 20 05 15 25 35 31 30 32

; FCE7 HEXCON converts key code in A into Hex equivelent for the key and returns in A. If non Hex command key entered returns to monitor
; FCE4 KEYHEX Combines GETKEY and HEXCON

; FCB5 BADDR Build 4 digit hex address from keyboard refreshing display and return address in X index register

; FCFF L7SEG converts left hex digit of A into 7 segment code for display and returns in A

; FD03 R7SEG converts right hex digit of A into 7 segment code for display and returns in A

; FD15 SVNHEX Converts 7 segment hex code in A to hex value returned in A, default to monitor if code not hex

; FCCC HEXIN Use KEYHEX to accept two hex key entries and combines two hex digits in one byte in A


; 6809 Memory Map

;EPROM Monitor FFFF
;              F800
;Serial Port
;  Control W   D200
;  Status  R   D200
;  Data    R/W D201
;     Note the PIA RS0 and RS1 Pins are reversed and connected to RS0=A1 and RS1=A0
;     Register order does not match the 6821 datasheet
;PIA  Out/Dir A D000
; 	  Out/Dir B D001
;     Control A D002
;     Control B D003

;RAM           7FFF
;              0000

;Monitor RAM
;   Display buffer right 7FFF
;                  left  7FFA
;   Working Storage      7FE0
;   Monitor stack        7FAF (MONSTACK)
;                        7FB0
;   User Stack           7F90 (USERSTACK)

; Interupt vector        Address
; SWI3                   7FE5/6
; SWI2                   7FE7/8
; FIRQ                   7FE9/AB
; NMI                    7FF2/3
; IRQ                    7FF4/5

; Reset Vector           FFFE FD
;                        FFFF 89

PORTA                         EQU $D000
PORTB                         EQU $D001
CTRLA                         EQU $D002
CTRLB                         EQU $D003

;Keys Hex 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  L  S CN   G  M  R  I
;Current 22 24 02 12 14 00 10 04 01 11 03 13 23 33 21 20 05 15 25  35 31 30 32

keyCN                         EQU $25
keyG                          EQU $35
keyI                          EQU $32
keyL                          EQU $05
keyM                          EQU $31
keyP                          EQU $15 ; P Punch replaced with S Save
keyS                          EQU $15
keyR                          EQU $30
key0                          EQU $22
key1                          EQU $24
key2                          EQU $02
key3                          EQU $12
key4                          EQU $14
key5                          EQU $00
key6                          EQU $10
key7                          EQU $04
key8                          EQU $01
key9                          EQU $11
keyA                          EQU $03
keyB                          EQU $13
keyC                          EQU $23
keyD                          EQU $33
keyE                          EQU $21
keyF                          EQU $20

seg0                          EQU $7E
seg1                          EQU $06
seg2                          EQU $5B
seg3                          EQU $1F
seg4                          EQU $27
seg5                          EQU $3D
seg6                          EQU $7D
seg7                          EQU $0E
seg8                          EQU $7F
seg9                          EQU $3F
segA                          EQU $6F
segB                          EQU $75
segC                          EQU $78
segD                          EQU $57
segE                          EQU $79
segF                          EQU $69
segG                          EQU $7C
segM                          EQU $6E ; (upside down U)
segP                          EQU $6B
segS                          EQU $3D ; (Same as 5)
segU                          EQU $76
segX                          EQU $67
segY                          EQU $37
segDash                       EQU $01

asciiNull                     EQU $00
asciiLF                       EQU $0A
asciiCR                       EQU $0D

ascii0                        EQU $30
ascii1                        EQU $31
ascii3                        EQU $33
ascii9                        EQU $39
asciiA                        EQU $41
asciiC                        EQU $43
asciiF                        EQU $46
asciiS                        EQU $53

; Used by HEXTODEC for conversion between decimal and Hex
DEC10K                        EQU  $2710
DEC1K                         EQU  $03E8

; Starting values for Monitor/System and User Stack pointers
MONSTACK                      EQU  $7FAF
USERSTACK                     EQU  $7F90

; Direct page values used to initialise Direct Page Register
dpRam                         EQU  $7F
dpPIA                         EQU  $D0

;REGDISPCHAR2                  EQU  $FDEF ; Label for where Register Display characters go from 8 bit to 16 bit
;chksum                        EQU  $7FED ; ?? Used for Send and Receive Checksum calculation
allzeros                      EQU  $0000 ; Used to initialise some registers with $0000

; Serial Port Constants
SERIALCTRL                    EQU  $D200           ; Use DP $50   $5000
SERIALSTATUS                  EQU  $D200           ; Use DP $50   $5000
SERIALDATA                    EQU  $D201           ; Use DP $50   $5001

CTRLRESET                     EQU  %00000011       ; CR1, CR0 1,1 for Master Reset
CTRLDIVIDE16                  EQU  %00000001       ; CR1, CR0 0,1 for divide by 16
CTRLDIVIDE64                  EQU  %00000010       ; CR1, CR0 1,0 for divide by 64
CTRLWORD8N1S                  EQU  %00010100       ; CR4, CR3, CR2 1, 0, 1 for 8 Bits, 1 Stop No Parity
CTRLRTSLOW                    EQU  %00000000       ; CR6, CR5, 0,0 RTS low IRQ disabled, requests data
CTRLRTSHIGH                   EQU  %01000000       ; CR6, CR5, 1,0 RTS high IRQ disabled, prevents receiving data
CTRLIRQENABLED                EQU  %10000000       ; CR7, 1 IRQ enabled

STATUSRDRF                    EQU  %00000001       ; Receive Data Register Full Bit 0
STATUSTDRE                    EQU  %00000010       ; Transmit Data Register Empty Bit 1
STATUSDCD                     EQU  %00000100       ; Data Carrier Detect Bit 2
STATUSCTS                     EQU  %00001000       ; Clear To Send Bit 3
STATUSFE                      EQU  %00010000       ; Framing Error Bit 4
STATUSOVRN                    EQU  %00100000       ; Receiver Overrun Bit 5
STATUSPE                      EQU  %01000000       ; Parity Error Bit 6
STATUSIRQREQ                  EQU  %10000000       ; Interupt Request /IRQ Bit 7

EOS                           EQU  $00             ; end of string

               ORG $7FB0           ; CRTC Variables are $1A in length

; CRTC Variables
DISPR0TOR15                   RMB 16; Current R0 to R15 values, can be overridden from defaults, call DISPCRTCREG
DISPROW                       RMB 1; Current Cursor Row 0-29
DISPCOL                       RMB 1; Current Cursor Column 0-79
DISPROWOFFSET                 RMB 2; Starting byte of current row, will increment by 160 for each row
DISPCLS                       RMB 2; Clear screen word Colour Byte + Character $F320 White on Blue space
DISPSTARTADDRESS              RMB 2; Current CRTC Start Address value R12 High, R13 Low, scroll display buffer 
DISPCURSORH                   RMB 1;
DISPCURSORL                   RMB 1;


;chksum                        EQU  $13ED
               ORG $7FD0
usrsp                         RMB 1  ;
                     ORG $7FE3
;org needs to be 13E4 to have separate chksum, when key input uses chksum messes up debug of save and load
bytecount                     RMB 1  ; new variable for srec byte count as using tmp2 got used by key input
chksum                        RMB 1  ;
SWI3VEC                       RMB 2  ;
SWI2VEC                       RMB 2  ;
FIRQVEC                       RMB 2  ;
count                         RMB 1  ;
tmp2                          RMB 1  ;
tmp3                          RMB 1  ;
ptr                           RMB 2  ;
addr                          RMB 2  ;
NMIVEC                        RMB 2  ;
IRQVEC                        RMB 2  ;
tmpX                          RMB 2  ; Used by Load to parse target address for X
STACKSTART                    RMB 2  ;
                                     ;    4      5      6      7      8      9  (Port B)
                                     ;  7FFA   7FFB   7FFC   7FFD   7FFE   7FFF
dbuf                          RMB 6  ; Display Buffer LSB is Left, MSB is Right

                     ORG $F700

CRTCADDRESS                   EQU  $D400
CRTCDATAREGISTER              EQU  CRTCADDRESS+1
VRAM                          EQU  $8000 ; Start of Video RAM
VRAMEND                       EQU  $BFFF ; End of Video RAM
CGRAM                         EQU  $C000 ; Start of character generator RAM
CGRAMEND                      EQU  $CFFF ; End of character generator RAM
CGROM                         EQU  $E000 ; Start of character generator ROM
ASCII0                        EQU  $30
ASCIISPACE                    EQU  $20
COLSPERROW                    EQU  $50
ROWSPERPAGE                   EQU  $1E
ROWSPERPAGEBCD                EQU  $30
BYTESPERCHAR                  EQU  $02
CLSDEFAULT                    EQU  $F220 ; White on Blue, Space

DISPRESET                     ldx      #DISPR0TOR1530X80 ; Point to default 30x80 CRTC Register values
                              ldy      #DISPR0TOR15      ; CRTC RAM values to initialise
                              clrb                       ; Clear CRTC Register counter
CRTCRAMLOOP                   lda      ,X+               ; Get Rn value from CRTC Table single + for +1, ++ is +2 and wrong!
                              sta      ,Y+               ; Save Rn data value in RAM as defaults which can be overridden
                              incb
                              cmpb     #$10              ; Have we processed all register values
                              bne      CRTCRAMLOOP
                              ldd      #CLSDEFAULT       ; Get default Clear Screen Colour and Character
                              std      DISPCLS           ; Save in working RAM
                                                         ; Fall through to now save CRTC RAM Values to R0-R15
CRTCREGLOAD                   ldx      #DISPR0TOR15      ; Point to current RAM CRTC Register values
                              clrb                       ; Clear CRTC Register counter
CRTCLOOP                      stb      CRTCADDRESS       ; Save Rn in CRTC Address Register        
                              lda      ,X+               ; Get Rn value from CRTC Table single + for +1, ++ is +2 and wrong!
                              sta      CRTCDATAREGISTER  ; Save Rn data value in CRTC Data Register
                              incb
                              cmpb     #$10              ; Have we processed all register values
                              bne      CRTCLOOP
                              bsr      COPYCHARGENROM    ; Copy Character bitmap values from ROM to CG RAM
                              bsr      CLRSCREEN
                              ldx      #BOOTMESSAGE
                              ldy      #VRAM+1           ; First Character of Video RAM
                              bsr      DISPMSGX
                              rts

COPYCHARGENROM                pshs     a,b,x,y
                              ldx      #CGRAM
                              ldy      #CGROM
COPYROM                       ldd      ,y++
                              std      ,x++
                              cmpx     #CGRAMEND
                              ble      COPYROM
                              puls     a,b,x,y,pc
                              ; Take character in A and write to screen in position defined by DISPROW, DISPCOL
                              ; If CHAR=10 or 13 LF/CR then DISPCOL=0 DISPROW=DISPROW+1, DISPROWOFFSET+=COLSPEROW*BYTESPERCHAR
                              ; If Char=12 (FormFeed) Clearscreen
                              ; Increment DISPCOL, If greater than COLSPERROW then DISPCOL=0, DISPROW=DISPROW+1
                              ; If DISPROW > ROWSPERPAGE then DISPROW=0 (this needs to change to support paging buffer)
                              ; Take DISPCOL, LSL 1 (multiple by 2) Add to DISPROWOFFET=X, store A at X, Add 1 to DISPCOL
                              ; Update cursor to X 
;CHROUT                        pshs a,x

                              ; Display message pointed to by X until Null delimiter and send to Y and add 2 for each character                              
DISPMSGX                      lda      ,x+
                              beq      DISPMSGXEND  ; End if Null value terminating string
                              sta      ,y++
                              nop                 ; Allow SWI to be entered and continue
                              bra      DISPMSGX
DISPMSGXEND                   rts 

                              ; Clearscreen and initalise screen RAM variables
CLRSCREEN                     pshs     a,b,x
                              ldx      #VRAM
                              ldd      DISPCLS ; Get default colour and character
CLEARSCREEN1                  std      ,X++
                              cmpx     #VRAMEND
                              ble      CLEARSCREEN1
                              clr      DISPROW
                              clr      DISPCOL
                              clr      DISPSTARTADDRESS
                              clr      DISPSTARTADDRESS+1
                              clr      DISPCURSORH
                              clr      DISPCURSORL
                              clr      DISPSTARTADDRESS
                              clr      DISPSTARTADDRESS+1
                              clr      DISPROWOFFSET
                              bsr      UPDATECRTCCURSOR  ; Call subroutine to update R14 & R15 with DISPCURSORH&L
                              bsr      UPDATECRTCSTARTADD  ; Call subroutine to update R12 & R13 with DISPSTARTADDRESS
                              puls a,b,x,pc
                              
                              ; Updates CRTC R14 & R15 with cursor values in DISPCURSORH and DISPCURSORL
UPDATECRTCCURSOR              pshs     a
                              lda      #$0E                ; R14
                              sta      CRTCADDRESS
                              lda      DISPCURSORH
                              sta      CRTCDATAREGISTER
                              lda      #$0F                ; R15 
                              sta      CRTCADDRESS
                              lda      DISPCURSORL
                              sta      CRTCDATAREGISTER                              
                              puls     a,pc

                              ; Updates CRTC R12 & R13 with CRTC start address value in DISPSTARTADDRESS
                              ; This will allow vertical and horizontal scrolling VRAM
UPDATECRTCSTARTADD            pshs     a
                              lda      #$0C                ; R12
                              sta      CRTCADDRESS
                              lda      DISPSTARTADDRESS
                              sta      CRTCDATAREGISTER
                              lda      #$0D                ; R13 
                              sta      CRTCADDRESS
                              lda      DISPSTARTADDRESS+1
                              sta      CRTCDATAREGISTER                              
                              puls     a,pc
                              
; These CRTC R0-R15 settings work with both LCD Monitor and VGA Capture card for 30 lines x 80 Characters at 59/60 Hz
DISPR0TOR1530X80              FCB      $63         ; R0 Horizontal Total 99
                              FCB      $50         ; R1 Horizontal Displayed 80
                              FCB      $53         ; R2 Horizontal from 53 to 55 Sync Position 83
                              FCB      $06         ; R3 Horizontal Sync Width 6
                              FCB      $1F         ; R4 Vertical Total 31
                              FCB      $14         ; R5 Vertical Total Adjust (was 13/$0D)
                              FCB      $1E         ; R6 Vertical Displayed 30
                              FCB      $1F         ; R7 Vertical Sync Position 31
                              FCB      $00         ; R8 Interlace mode - Non Interlaced
                              FCB      $0F         ; R9 Maximum Scan Line Address 16 rows RA0-RA03
                              FCB      $6D         ; R10 Cursor Start - Slow Blink C0 + Line 13 Start Default $6D Cursor off $20
                              FCB      $6F         ; R11 Cursor End - Slow Blink C0 + Line 15 Finish 6F
                              FCB      $00,$00     ; R12,R13 Start Address VRAM offset for hardware scrolling
                              FCB      $00,$00     ; R14,R15 Cursor Address
                                                   ; Pad to orginal Monitor F800 range
BOOTMESSAGE                   FCC  "Nanocomp 6809 V8.00"
                              FCB  $00
                              FCB  $00, $00, $00, $00, $00, $00, $00, $00
                              FCB  $00, $00, $00, $00, $00, $00, $00, $00
                              FCB  $00, $00, $00, $00, $00, $00, $00, $00
                              FCB  $00, $00, $00, $00, $00, $00, $00, $00
                              FCB  $00, $00, $00, $00

                     ORG $F800
; F800 Hex to decimal converter
; G F800 Type L then decimal No then I to display Hex, press I to start again
;        Press P for hex to decimal then I

HEXTODEC             lds      #USERSTACK           

HEXTODEC1            jsr      CLEARDISP
                     lda      #dpRam             
                     tfr      a,dp
                     clra     
                     clrb     
                     sta      <$07
                     std      <$08
                     std      <$0A
                     std      <$0C
                     std      <$0E
HEXTODEC2            jsr      GETKEY

;   P pressed for Hex to Decimal
                     cmpa     #keyP             
                     lbeq     HEXTODEC141

;   L pressed for Decimal to Hex
                     cmpa     #keyL             
                     bne      HEXTODEC2
                     ldx      #$1309
HEXTODEC3            jsr      GETKEY

;   I pressed to display Hex
                     cmpa     #keyI             
                     beq      HEXTODEC4
                     leax     $01,x
                     jsr      HEXCON
                     cmpa     #$09
                     bhi      HEXTODEC1
                     sta      ,x
                     jsr      R7SEG
                     sta      $F0,x 
                     bra      HEXTODEC3

HEXTODEC4            lda      ,x
                     sta      <$09
                     bsr      HEXTODEC13
HEXTODEC5            ldb      #$0A
                     cmpa     #$00
                     beq      HEXTODEC6
                     addb     <$09
                     stb      <$09
                     deca     
                     bra      HEXTODEC5

HEXTODEC6            bsr      HEXTODEC13
HEXTODEC7            cmpa     #$00
                     beq      HEXTODEC8
                     pshs     a
                     ldd      #$64
                     addd     <$08
                     std      <$08
                     puls     a
                     deca     
                     bra      HEXTODEC7

HEXTODEC8            bsr      HEXTODEC13
HEXTODEC9            cmpa     #$00
                     beq      HEXTODEC10
                     pshs     a
                     ldd      #DEC1K
                     addd     <$08
                     std      <$08
                     puls     a
                     deca     
                     bra      HEXTODEC9

HEXTODEC10           bsr      HEXTODEC13
HEXTODEC11           cmpa     #$00
                     beq      HEXTODEC12
                     pshs     a
                     ldd      #DEC10K
                     addd     <$08
                     std      <$08
                     puls     a
                     deca     
                     bra      HEXTODEC11

HEXTODEC12           jsr      CLEARDISP
                     lda      <$07
                     jsr      R7SEG
                     sta      <dbuf+0 
                     lda      <$08
                     jsr      L7SEG
                     sta      <dbuf+1 
                     lda      <$08
                     jsr      R7SEG
                     sta      <dbuf+2 
                     lda      <$09
                     jsr      L7SEG
                     sta      <dbuf+3 
                     lda      <$09
                     jsr      R7SEG
                     sta      <dbuf+4
                     jsr      GETKEY
                     lbra     HEXTODEC1

HEXTODEC13           leax     -$01,x
                     cmpx     #$1309
                     beq      HEXTODEC14
                     lda      ,x
                     rts      

HEXTODEC14           leas     $02,s
                     bra      HEXTODEC12

HEXTODEC141          lda      #$67
                     sta      <dbuf+5
                     jsr      BADDR
                     stx      <$0A
                     ldx      #$130A
                     ldy      #$1307
                     ldb      ,x
                     lsrb     
                     lsrb     
                     lsrb     
                     lsrb     
                     beq      HEXTODEC16
HEXTODEC15           lda      #$40
                     bsr      HEXTODEC23
                     lda      #$96
                     bsr      HEXTODEC24
                     decb     
                     bne      HEXTODEC15
HEXTODEC16           ldb      ,x
                     andb     #$0F
                     beq      HEXTODEC18
HEXTODEC17           lda      #$02
                     bsr      HEXTODEC23
                     lda      #$56
                     bsr      HEXTODEC24
                     decb     
                     bne      HEXTODEC17
HEXTODEC18           ldb      $01,x
                     lsrb     
                     lsrb     
                     lsrb     
                     lsrb     
                     beq      HEXTODEC20
HEXTODEC19           lda      #$16
                     bsr      HEXTODEC24
                     decb     
                     bne      HEXTODEC19
HEXTODEC20           ldb      $01,x
                     andb     #$0F
                     beq      HEXTODEC22
HEXTODEC21           lda      #$01
                     bsr      HEXTODEC24
                     decb     
                     bne      HEXTODEC21
HEXTODEC22           lbra     HEXTODEC12

HEXTODEC23           andcc    #$FE
                     bra      HEXTODEC25

HEXTODEC24           adda     $02,y
                     daa      
                     sta      $02,y
                     lda      #$00
HEXTODEC25           adca     $01,y
                     daa      
                     sta      $01,y
                     lda      #$00
                     adca     ,y
                     daa      
                     sta      ,y
                     rts      

                     FCB  $00, $00, $00, $00, $00, $00, $00, $00, $00, $00 ; removed one 00 for V3 64K Map
; F940 Duckshoot
; G F940 Store 0020 at memory 0000 and 0001 (1000 and 1001 for 6809)
;        Press number 1-6 for duck until none are left.

DUCKSHOOT            lds      #USERSTACK           
                     lda      #dpRam             
                     tfr      a,dp
                     jsr      CLEARDISP
                     lda      #$1F
                     sta      <dbuf+3 
                     sta      <dbuf+5
                     bsr      DUCKSHOOT6
DUCKSHOOT0           jsr      HEXCON
                     cmpa     #$06
                     bhi      DUCKSHOOT2
                     cmpa     #$00
                     beq      DUCKSHOOT2
                     ldx      #$13F9
                     leax     a,x
                     lda      ,x
                     beq      DUCKSHOOT1
                     clr      ,x
                     bra      DUCKSHOOT2

DUCKSHOOT1           lda      #$1F
                     sta      ,x
DUCKSHOOT2           leay     -$01,y
                     beq      DUCKSHOOT3
                     bsr      DUCKSHOOT7
                     bra      DUCKSHOOT2

DUCKSHOOT3           bsr      DUCKSHOOT4
                     bsr      DUCKSHOOT6
                     bra      DUCKSHOOT2

DUCKSHOOT4           ldx      #$13FF
                     lda      ,x
                     sta      <$00F9
                     ldb      #$06
DUCKSHOOT5           lda      ,-x
                     sta      $01,x
                     decb     
                     bne      DUCKSHOOT5
                     rts      

DUCKSHOOT6           ldy      $1000
DUCKSHOOT7           ldx      #dbuf 
                     ldb      #$06
DUCKSHOOT8           lda      ,x+
                     bne      DUCKSHOOT9
                     decb     
                     bne      DUCKSHOOT8
                     jmp      RESET

DUCKSHOOT9           clra     
                     sta      CTRLA
                     sta      CTRLB 
                     sta      PORTA
                     lda      #$0F
                     sta      PORTB 
                     lda      #$04
                     sta      CTRLA
                     sta      CTRLB 
                     ldb      #$FF
DUCKSHOOT10          incb     
                     cmpb     #$04
                     bne      DUCKSHOOT11
                     jsr      DISPRESH
                     leau     -$72,pc ; ($F953)
                     stu      ,s
                     leay     -$01,y
                     bne      DUCKSHOOT7
                     bsr      DUCKSHOOT4
                     bra      DUCKSHOOT6

DUCKSHOOT11          stb      PORTB 
                     lda      PORTA
                     coma     
                     beq      DUCKSHOOT10
                     pshs     b
                     clra     
                     ldb      #$01
DUCKSHOOT12          pshs     a
                     lda      PORTA
                     coma     
                     pshs     b
                     cmpa     ,s+
                     puls     a
                     beq      DUCKSHOOT13
                     inca     
                     aslb     
                     bra      DUCKSHOOT12

DUCKSHOOT13          puls     b
                     aslb     
                     aslb     
                     aslb     
                     aslb     
                     pshs     b
                     adda     ,s+
                     rts      

                     FCB  $00, $00, $00, $00, $00, $00

; FA00 Calculate offset
; G FA00 Displays S enter start address  then I
;        Displays d then enter destination
;        Displays b Enter 2 or 3 for number of bytes length of full instruction
;        Twos complement offset then displayed
;        If outside range displays --, press I for next

CALCOFFSET           lds      #USERSTACK
                     lda      #dpRam             
                     tfr      a,dp
CALCOFFSET1          ldd      #$3D
                     std      <dbuf+4
                     jsr      BADDR
CALCOFFSET2          jsr      GETKEY
                     cmpa     #keyI             
                     bne      CALCOFFSET2
                     stx      <$68
                     lda      #$57
                     sta      <dbuf+5
                     jsr      BADDR
                     stx      <addr
                     jsr      CLEARDISP
                     lda      #$75
                     sta      <dbuf+5
CALCOFFSET3          jsr      KEYHEX
                     cmpa     #$01
                     bls      CALCOFFSET3
                     clr      <$6A
                     sta      <$6B
                     ldd      <addr
                     subd     <$6A
                     subd     <$68
                     std      <$68
                     jsr      L7SEG
                     sta      <dbuf+2 
                     lda      <$68
                     jsr      R7SEG
                     sta      <dbuf+3 
                     lda      <$69
                     jsr      L7SEG
                     sta      <dbuf+4
                     lda      <$69
                     jsr      R7SEG
                     sta      <dbuf+5
                     clrb     
                     stb      <dbuf+0 
                     stb      <dbuf+1 
                     lda      <$6B
                     cmpa     #$02
                     bne      CALCOFFSET4
                     stb      <dbuf+3 
                     stb      <dbuf+2 
                     ldd      <$68
                     rolb     
                     rola     
                     beq      CALCOFFSET4
                     coma     
                     bne      CALCOFFSET5
CALCOFFSET4          jsr      GETKEY
                     cmpa     #keyI             
                     bne      CALCOFFSET4
                     bra      CALCOFFSET1

CALCOFFSET5          ldd      #$0101
                     std      <dbuf+4
                     bra      CALCOFFSET4

                     FCB  $00, $00

; FA80 Mastermind
;        I displayed after thinking of 4 digit number 4 digits 0-7
;        Press I enter guess 2 digits display Correct digits and correct numbers in wrong places (Bulls and Cows)
;        Press I for next number, after 4 bulls press I and displays number of tries

MASTERMIND           lds      #USERSTACK           
                     lda      #dpRam             
                     tfr      a,dp
                     jsr      CLEARDISP
                     lda      #$06
                     sta      <dbuf+5
                     clrb     
                     std      <$10
                     std      <$12
MASTERM1             bsr      MASTERM3
                     beq      MASTERM4
                     ldx      #$1310
                     lda      ,x
                     inca     
                     sta      ,x
                     bita     #$08
                     beq      MASTERM1
MASTERM2             clra     
                     sta      ,x+
                     cmpx     #$1314
                     beq      MASTERM1
                     lda      ,x
                     inca     
                     sta      ,x
                     bita     #$08
                     beq      MASTERM1
                     bra      MASTERM2

MASTERM3             jsr      DISPRESH
                     ldx      #allzeros
                     stx      CTRLA
                     ldx      #$0F
                     stx      PORTA
                     ldx      #$0404
                     stx      CTRLA
                     lda      #$03
                     sta      PORTB 
                     lda      PORTA
                     bita     #$04
                     rts      

MASTERM4             clr      <$0E
MASTERM5             jsr      CLEARDISP
                     ldx      #$1300
MASTERM6             clrb     
MASTERM7             lda      PORTA
                     coma     
                     bne      MASTERM6
                     decb     
                     bne      MASTERM7
MASTERM8             jsr      KEYHEX
                     cmpa     #$07
                     bhi      MASTERM8
                     sta      ,x+
                     jsr      R7SEG
                     sta      $F9,x 
                     cmpx     #$1304
                     bne      MASTERM8
                     lda      #$01
                     adda     <$0E
                     daa      
                     sta      <$0E
                     ldb      #$FE
                     clra     
                     sta      <$0C
                     sta      <$0D
                     ldx      #$1304
MASTERM9             lda      $0C,x
                     sta      $04,x
                     sta      ,x+
                     cmpx     #$1308
                     bne      MASTERM9
                     ldx      #$1300
MASTERM10            lda      ,x
                     cmpa     $04,x
                     beq      MASTERM14
MASTERM11            leax     $01,x
                     cmpx     #$1304
                     bne      MASTERM10
                     ldx      #$1300
MASTERM12            lda      ,x
                     cmpa     <$04
                     beq      MASTERM15
                     cmpa     <keyL
                     beq      MASTERM16
                     cmpa     <$06
                     beq      MASTERM17
                     cmpa     <$07
                     beq      MASTERM18
MASTERM13            leax     $01,x
                     cmpx     #$1304
                     beq      MASTERM20
                     bra      MASTERM12

MASTERM14            inc      <$0C
                     stb      ,x
                     incb     
                     stb      $04,x
                     decb     
                     bra      MASTERM11

MASTERM15            incb     
                     stb      <$04
                     bra      MASTERM19

MASTERM16            incb     
                     stb      <keyL
                     bra      MASTERM19

MASTERM17            incb     
                     stb      <$06
                     bra      MASTERM19

MASTERM18            incb     
                     stb      <$07
MASTERM19            decb     
                     inc      <$0D
                     stb      ,x
                     bra      MASTERM13

MASTERM20            lda      <$0C
                     jsr      R7SEG
                     sta      <dbuf+4
                     lda      <$0D
                     jsr      R7SEG
                     sta      <dbuf+5
MASTERM21            lbsr     MASTERM3
                     bne      MASTERM21
                     lda      <$0C
                     cmpa     #$04
                     lbne     MASTERM5
                     lda      <$0E
                     jsr      L7SEG
                     cmpa     #$7E
                     bne      MASTERM22
                     clra     
MASTERM22            sta      <dbuf+0 
                     lda      <$0E
                     jsr      R7SEG
                     sta      <dbuf+1 
                     ldd      #$7C7E
                     std      <dbuf+2 
                     ldd      #$203D
                     std      <dbuf+4
MASTERM23            clrb     
MASTERM24            lda      PORTA
                     coma     
                     bne      MASTERM23
                     decb     
                     bne      MASTERM24
                     lbra     MASTERM1

                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00 ; Extra 00 for KEYCODE to start at FC00


              
; Monitor Start FC00
; Keypad keycode lookup table to covert key to nibble values, used by KEYHEX and HEXCON
KEYCODE              FCB  key0, key1, key2, key3, key4, key5, key6, key7
                     FCB  key8, key9, keyA, keyB, keyC, keyD, keyE, keyF 

; Seven Segment display lookup table, used to convert 7 Seg value to nibble and nibble to 7 Seg value
; Used by HEXCON, R7SEG and SVNHEX subroutines
SVNSEG               FCB  seg0, seg1, seg2, seg3, seg4, seg5, seg6, seg7
                     FCB  seg8, seg9, segA, segB, segC, segD, segE, segF

; FC20 GETKEY Scans for key and refreshes display until key pressed.
; Then waits to be released and returns key in A
; Key Codes in Hex (see keyxx EQU definitions)

;  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  L SP CN G  M  R  I
; 22 24 02 12 14 00 10 04 01 11 03 13 23 33 21 20 05 15 25 35 31 30 32

GETKEY               pshs     y,dp,b
                     lda      #dpPIA            ; Set Direct Page register to PIA page ($40), PIA code uses Direct addressing 
                     tfr      a,dp

;   Set up I/O port for Port A 0-7 input and Port B 0-3 output
GETKEY1              bsr      DISPRESH          ; Refresh 7 Segment display while waiting for key
                     clra     
                     sta      <CTRLA            ; Set Port A to data direction register b2=0 DDR
                     sta      <CTRLB            ; Set Port B to data direction register
                     sta      <PORTA            ; Set Port A to input bits 0-7
                     lda      #$0F              ; set bits 0-3 for output
                     sta      <PORTB            ; Set Port B to output for bits 0-3
                     lda      #$04              ; Set control register b2=1 Output Register
                     sta      <CTRLA            ; Set Port A control register for Port Output, will be keypad switch input
                     sta      <CTRLB            ; Set Port B control register for Port Output, will be keypad row
                     lda      #$FF              ; *** This should probably set ldb to $FF, bug in original Monitor Code
                                                ; B register is used to strobe keypad output PB0, PB1 to drive 74LS145 decoder (7442 in original)
GETKEY2              incb                       ; Read next keypad row (row will change to 0, key pressed changes from 1 to 0)
                     cmpb     #$04              ; Done rows 0,1,2,3?
                     beq      GETKEY1           ; Scanned all 4 keypad lines, update display and start again
                     stb      <PORTB            ; Output current keypad row to Port B, selected row output will be set to 0 (default to 1)
                     lda      <PORTA            ; Read keypad keys from Port A (note Port A is pulled high, pressed key sets bit to 0)
                     coma                       ; Complement keypad input so pressed key is indicated by 1
                     beq      GETKEY2           ; If all zero, no key pressed, try next keypad row

;   Found a key: decode it
                     stb      tmp2              ; Save the keypad row in B to tmp2 (better name ****)
                     sta      tmp3              ; Save input key to tmp3 (was chksum which maps to same address $13ed)
                     clra                       ; A maintains the bit count of which bit is set
                     ldb      #$01              ; Set b0 as test bit
GETKEY3              cmpb     tmp3              ; Is bit A set?, what happens if two keys pressed, just the lowest?
                     beq      GETKEY4           ; If bit is set calculate key code GETKEY4
                     inca                       ; Next bit count                       
                     aslb                       ; Rotate bit to next positon, fill b0 with 0, when all 0 set Z (eq)
                     beq      GETKEY2           ; When done 0-7 then process next keypad row
                     bra      GETKEY3           ; Check next bit

GETKEY4              ldb      tmp2              ; Get the keypad row
                     aslb                       ; Shift to the top nibble
                     aslb     
                     aslb     
                     aslb     
                     pshs     b                 ; Add B to A via stack
                     adda     ,s+               ; Add value of B on stack to A to calculate keycode

;   Wait for the key to be released
                     pshs     a                 ; Save keycode
                     ldy      #$08              ; Keybounce delay will be 8 (Y) x 256 (B), 2048 loops
GETKEY5              clrb                       ; Initialise B as inner loop counter
GETKEY6              lda      <PORTA            ; Check port to see if key still pressed (bit=0 key pressed)
                     coma                       ; Complement A, non zero (NE) means a key still held down
                     bne      GETKEY5           ; If key still pressed, zero count and check again 
                     decb                       ; Key released, decrement count
                     bne      GETKEY6           ; If still not zero check port again (this will de-bounce keys)
                     leay     -$01,y            ; Counted down B to 0 from 255, decrement Y
                     bne      GETKEY6           ; Check port again, until Y is zero
                     puls     a,b,dp,y,pc       ; (pul? pc=rts) Keycode returned in A 

; FC7B DISPRESH Displays 6 digits from 7FFA (left) to 7FFF (right) 
; Bit Segments, note that A-G seqments in 1981 design are different from modern LED displays (E&F swapped)
;       b3 A
;     -------
; b5 E|     | b1 B
;     | b0 G|
;     -------
; b6 F|     | b2 C
;     |     |
;     -------
;       b4 D  
; 7 Segment display values (see segxx EQU  definitions above)
;  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  -  G  M  P  S  U  X  Y
; 7E 06 5B 1F 27 3D 7D 0E 7F 3F 6F 75 78 57 79 69 01 7C 6E 6B 3D 76 67 37
; Note that due to how the LED is wired segments are on by default
; A 1 value on PIA port will pull segment low turning it off, so values are complemented COM before sending to PIA

DISPRESH             pshs     x,b,a

;   Set output ports for display
                     ldx      #PORTA            ; This routine uses a different approach from GETKEY using index X and offsets
                     clra     
                     sta      $02,x             ; Set CTRLA Port A to data direction register b2=0 DDR
                     sta      $03,x             ; Set CTRLB Port B to data direction register
                     lda      #$7F              ; 
                     sta      ,x                ; Set Port A to output bits 0-6
                     lda      #$0F              ; Set bits 0-3 for output
                     sta      $01,x             ; Set Port B to output for bits 0-3
                     lda      #$04              ; Set control register b2=1 Output Register
                     sta      $02,x             ; Set CTRLA Port A control register for Port Output, will be keypad switch input
                     sta      $03,x             ; Set CTRLB Port B control register for Port Output, will be keypad row

;   Initialise loop over digits
                     ldx      #dbuf             ; Segments are numbered 4..9 to correspond to outputs from the 74LS145/7442 decoder.
                     ldb      #$03
DISP1                incb                       ; Start at 04
                     cmpb     #$0A              ; until done 09
                     bne      DISP2             ; Display next digit

;   Finished
                     puls     a,b,x,pc          ;(pul? pc=rts) Done all 6 segments 04-09


;   Light up the next digit                     ; Changes to reduce Ghosting of 7 Segment displays
DISP2                lda      #$7F              ; This turns off all 7 Segments
                     sta      PORTA             ; Turn off current segments before changing PORTB
                     lda      ,x+               ; Get segment value from Display buffer first
                     coma                       ; Complement 7 Segment values as 1 turns off segment
                     stb      PORTB             ; Select segment on Port B (04-09)            
                     sta      PORTA               ; Save segment bits to Port A

;   Delay loop
                     lda      #$A0              ; Delay loop for 160 iterations
DISP3                deca                       
                     bne      DISP3             ; Delay until A is zero
                     bra      DISP1             ; Process next segment

; Assume this code is before BADDR to keep the subroutine addresses consistent with 6802 code
BADDR1               exg      a,b               ; Swap A and B
                     tfr      d,x               ; Return address in X
                     puls     a,b,pc ;(pul? pc=rts)

; 7CB5 BADDR Build 4 digit hex address from keyboard, refreshing display and return address in X index register
BADDR                pshs     b,a               ; Preserve A and B
                     clra                       ; Clears D (A+B)
                     clrb                       ;
                     std      dbuf              ; Clear left 4 digits in display buffer
                     std      dbuf+2           
                     ldx      #dbuf             ; X is left hand digit of display buffer
                     bsr      HEXIN             ; Get two Hexadecimal keys, and display at X, returns in A
                     pshs     a                 ; Save most significant byte on stack
                     bsr      HEXIN             ; Get two Hexadecimal keys, and display at X, returns in A
                     puls     b                 ; Now get most significant byte from stack
                     bra      BADDR1            ; B=MSB, A=LSB which is wrong way around for D

; 7CCC HEXIN Use KEYHEX to accept two hex key entries and combines two hex digits in one byte in A and updates display at X and X+1
HEXIN                bsr      KEYHEX            ; Get most significant byte (left hand segment)

                     asla                       ; Shift hex digit value to upper nibble
                     asla     
                     asla     
                     asla     
                     pshs     a                 ; Save for return value
                     bsr      L7SEG             ; Convert MSB (Left hand segment) to segment value
                     sta      ,x+               ; Display segment at X and increment to next segment
                     bsr      KEYHEX            ; Get least significant byte
                     adda     ,s+               ; Add to saved most significant byte and remove from stack
                     pshs     a                 ; Save sum of MSB and LSB
                     bsr      R7SEG             ; Convert LSB (right hand segment) to segment
                     sta      ,x+               ; Display segment at X and increment to next segment
                     puls     a,pc              ;(pul? pc=rts) return saved Hex two digit value in A 

; 7CE4 KEYHEX Combines GETKEY and HEXCON
; Gets a single key digit 0-F, converts to 7 Segment, displays at X then returns hex value, return to monitor if invalid key
KEYHEX               lbsr     GETKEY            ; Get a single key

;   Fall-thru
; 7CE7 HEXCON converts key code in A into Hex equivelent for the key and returns in A. 
; If non Hex command key entered returns to monitor
HEXCON               pshs     x,b
                     ldx      #KEYCODE          ; Lookup key in KEYCODE table
                     ldb      #$FF
HEXCON1              incb                       ; B will have hex value of KEYCODE
                     cmpx     #SVNSEG           ; Check if X is at end of KEYCODE table
                     beq      TORESUME2         ; If it is at end without finding key, the resume to monitor **** (may not need toresume2 perhaps to avoid lbeq)
                     cmpa     ,x+               ; Compare KEYCODE at X with current key value
                     bne      HEXCON1           ; If no match then move to next KEYCODE value
                     tfr      b,a               ; Hex digit value returned in A
                     puls     b,x,pc ;(pul? pc=rts)

                     nop      
                     nop      
                     nop      

;   Fall-thru
; 7CFF L7SEG converts left hex digit of A into 7 segment code for display and returns in A
L7SEG                asra                       ; Shift MSB right 4 bits so it is LSB
                     asra     
                     asra     
                     asra     

; 7D03 R7SEG converts right hex digit of A into 7 segment code for display and returns in A
;   Convert low-order 4 bits of A to 7seg
R7SEG                pshs     x                 ; Save X, used as index to lookup 7 segment values
                     ldx      #SVNSEG           ; X is 7 segment lookup table start
                     anda     #$0F              ; mask MSB
R71                  beq      R72               ; A=0 we have found value already
                     leax     $01,x             ; Increment X
                     deca                       ; Decrease A (note that leax A,X would avoid the need for the loop)
                     bra      R71               ; Check next value

R72                  lda      ,x                ; Get 7 segment value for A
                     puls     x,pc              ;(pul? pc=rts) return A and restore X 

; 7D15 SVNHEX Converts 7 segment code in A to a hex value returned in A, default to monitor if code not hex
SVNHEX               pshs     x,b
                     ldx      #SVNSEG           ; Lookup value in A in SVNSEG table
                     tfr      a,b               ; Use B for lookup, return Hex value in A
                     clra                       ; Initialise A
SVNHEX1              cmpb     ,x+               ; Does 7 segment value match?
                     beq      SVNHEX2           ; If match return value in A
TORESUME2            beq      RESUME            ; Wont be zero when landing from previous instruction
                     inca                       ; Next segment value
                     bra      SVNHEX1           ; Lookup next code value in SVNSEG 

SVNHEX2              puls     b,x,pc ;(pul? pc=rts)


;   Prompt for address with 'M' and display memory contents
MEMDISP              clra                       ;
                     sta      <dbuf+4           ; Clear Left Hand Data segment (digit 5)
                     lda      #segM             ; Display M in right hand data segment (digit 6)
                     sta      <dbuf+5
                     bsr      BADDR             ; Gets 4 digit address then displays in Digits 1-4, then returns address in X
MEM0                 lda      ,x                ; Get current memory value at X
                     pshs     a                 ; Save memory value
                     bsr      L7SEG             ; Convert Most Significant Byte / Left Hand digit to segment
                     sta      <dbuf+4           ; Display MSB on left data segment
                     puls     a                 ; Get saved memory value
                     bsr      R7SEG             ; Convert Least Significant Byte / Right hand digit to segment
                     sta      <dbuf+5           ; Display LSB on right data segment

;   Check for I key
MEM1                 lbsr     GETKEY            ; Check for key press
                     cmpa     #keyI             ;  
                     beq      MEM2              ; If I pressed then save and then get next memory value
                     bsr      HEXCON            ; Not I so convert key to hex value in A, returns to monitor if not 0-F
                     ldb      <dbuf+5           ; Move LSB to MSB (move right to left data segment)
                     stb      <dbuf+4
                     bsr      R7SEG             ; Convert A LSB / right hand digit into 7 segment value
                     sta      <dbuf+5           ; Display LSB in right hand data segment
                     bra      MEM1              ; Get next key (hex values will continue to rotate LSB to MSB until I pressed or Abort


;   Store and increment
MEM2                 lda      <dbuf+4           ; Get MSB 7 Segment value
                     bsr      SVNHEX            ; Convert A from 7 segment to Hex MSB value
                     asla                       ; Shift left 4 bits to become MSB nibble     
                     asla     
                     asla     
                     asla     
                     pshs     a                 ; Save MSB value
                     lda      <dbuf+5           ; Get LSB 7 segment value
                     bsr      SVNHEX            ; Convert A from 7 segment to Hex LSB value
                     adda     ,s+               ; Add LSB to MSB nibble
                     tfr      a,b               ; Save A in B
                     sta      ,x                ; Store new value in A at X
                     lda      ,x+               ; Re-read memory value from X then increment X for next byte
                     pshs     b                 ; Push new value onto stack
                     cmpa     ,s+               ; Compare new value and current value at X
                     bne      RESUME            ; If not equal then tried to update invalid RAM (eg ROM etc), return to monitor
                     tfr      x,d               ; X is now next address, copy to D (A=MSB, B=LSB)
                     bsr      L7SEG             ; Convert MSB left digit to 7 segment code
                     sta      <dbuf+0           ; display MSB left digit
                     tfr      x,d               ; Copy X to D again
                     bsr      R7SEG             ; Convert MSB right digit to 7 segment code
                     sta      <dbuf+1           ; display MSB right digit
                     tfr      b,a               ; Copy LSB to A
                     bsr      L7SEG             ; Convert LSB left digit to 7 segment code
                     sta      <dbuf+2           ; display LSB left digit
                     tfr      b,a               ; Copy B to A again
                     lbsr     R7SEG             ; Convert LSB right digit to 7 segment code
                     sta      <dbuf+3           ; display LSB right digit
                     bra      MEM0              ; Get memory value from new X address now on display


;   Reset Handler
RESET               lds      #MONSTACK          ; Load System Stack Pointer with Monitor default  
                    jsr      DISPRESET
NMISR               sts      STACKSTART         ; Save current stack pointer at STACKSTART $13F8 (after NMI or RESET)

;   Set up NMI to point here
                     ldx      #NMISR            ; Save NMI Service Routine address in NMI Vector  
                     stx      NMIVEC
RESUME               lds      #MONSTACK         ; Resume monitor and reinitialise stack pointer   
                     lda      #dpRam            ; Initialise direct page register 
                     tfr      a,dp
                     bsr      CLEARDISP         ; Clear 7 segment display

;   Show '-' at left
                     lda      #segDash          ; Display - on left hand digit 1   
                     sta      <dbuf+0 
                     lbsr     GETKEY            ; Wait for a key

;   If key is 'M'
                     cmpa     #keyM             ; If M Display Memory
                     lbeq     MEMDISP

;   If key is 'R'
                     cmpa     #keyR             ; If R display Registers
                     beq      REGDISP

;   If key is 'CN'
                     cmpa     #keyCN            ; If Continue then pull CPU state from stack and continue 
                     beq      CONT

;   If key is 'L'
                     cmpa     #keyL             ; If L then Load from Serial Port in S-REC
                     lbeq     LOAD

;   If key is 'S' (was P for punch)
                     cmpa     #keyS             ; If S then Save to Serial Port in S-REC
                     lbeq     SAVE

;   If key is 'G'
RESET3               cmpa     #keyG             ; If G then prompt for Address and execute starting at that address
                     bne      RESUME            ; Otherwise wait for another key

;   Prompt and accept start address
GO                   lda      #segG             ; Display G on right hand data segment 6
                     sta      <dbuf+5
                     lbsr     BADDR             ; Get 16 bit address in X and display on segments 1-4
                     ldy      <STACKSTART       ; Get previous saved stack pointer before NMI or SWI
                     stx      $0A,y             ; Overwrite PC saved on stack
                     lda      #$80              ; Set Entire bit in Condition Code register on stack
                     ora      ,y                ; OR with existing CC value
                     sta      ,y                ; Save back CC with Entire bit set
CONT                 lds      <STACKSTART       ; Load stack pointer from saved value
                     rti                        ; Return from interupt, which will load program counter from X value returned from BADDR


;   Clear display
CLEARDISP            pshs     x,b,a
                     clra                       ; Blank segment value
                     ldb      #$06              ; Iterate over 6 digits 
                     ldx      #dbuf 
CLEARDISP1           sta      ,x+               ; Clear display buffer
                     decb                       ; Next segment (1-6)
                     bne      CLEARDISP1        ; Next segment
                     puls     a,b,x,pc ;(pul? pc=rts)


;   Characters for Register Display  C A b d X Y U P S
;   The label REGDISPCHAR2 indicates where values change from 1 byte to 2 bytes at X
REGDISPCHAR          FCB segC, segA, segB, segD
REGDISPCHAR2         FCB segX, segY, segU, segP, segS

REGDISPSWI           lda      #dpRam             ; Initialise direct page register
                     tfr      a,dp
                     sts      <STACKSTART        ; Save stackpointer from running code on SWI
                     lds      #MONSTACK          ; Set stackpointer to monitor default  
REGDISP              bsr      CLEARDISP          ; Clear display
                     ldx      <STACKSTART        ; Get running stack pointer from saved value into X

;   Read characters from REGDISPCHAR
                     leay     -$1B,pc            ; Loads Y with REGDISPCHAR register table start address ($7DEB) 
REGDISP1             lda      ,y+                ; Get register prefix character
                     sta      <dbuf+5            ; Display at segment 6
                     cmpy     #REGDISPSWI        ; Are we on the last register S?       
                     bne      SHOWLEFT           ; Not on S, display bytes
                     ldx      #STACKSTART
SHOWLEFT             cmpy     #REGDISPCHAR2      ; Are we on single byte values still? X is first 2 byte
                     bls      SHOWBYTE           ; Display single byte value
                     lda      ,x                 ; Get most significant byte of two byte value
                     lbsr     L7SEG              ; Get left hand digit segment code
                     sta      <dbuf+0            ; Display at segment 1
                     lda      ,x+                ; Get same byte for register and increment X
                     lbsr     R7SEG              ; Get right hand digit segment code
                     sta      <dbuf+1            ; Display at segment 2
SHOWBYTE             lda      ,x                 ; Get byte value for register from stack
                     lbsr     L7SEG              ; Get left hand segment code
                     sta      <dbuf+2            ; Display at segment 3
                     lda      ,x+                ; Get same byte value for register and increment X
                     lbsr     R7SEG              ; Get right hand segment code
                     sta      <dbuf+3            ; Display at segment 4
                     lbsr     GETKEY             ; Get a key
                     cmpa     #keyI              ; Has I been pressed
                     lbne     RESUME             ; No then return to monitor 
                     cmpy     #REGDISPSWI        ; Are we at end of register table?       
                     bne      REGDISP1           ; No so display next register
                     lbra     RESUME             ; Yes so return to monitor



;   Set up serial port
LOAD                 lda      #CTRLRESET  ; Master Reset bits
                     sta      SERIALCTRL  ; Serial Port Control Master Reset
                     lda      #CTRLDIVIDE64 | CTRLWORD8N1S | CTRLRTSLOW
                     sta      SERIALCTRL  ; Serial Port Divide by 64 (3.6864Mhz Xtal /6 /64 =9600 ), 8 Bits No Parity 1 Stop, /RTS Low - receive data, DCD wired low


RELOAD               bsr      RECVBYTE    ; Get a byte without Hex conversion, should be S1 or S9 combination

;   Wait for $53 = S
                     cmpa     #asciiS    
                     bne      RELOAD      ; Not S then get next byte
                     bsr      RECVBYTE    ; We have S so get next byte, should be S record type 1 or 9

;   Code S1 begins a record
                     cmpa     #ascii1    
                     beq      LOAD2       ; Parse the bytecount and start address, the data and checksum

;   Code S9 means EOF
                     cmpa     #ascii9   
                     bne      RELOAD      ; If not S1 or S9 keep reading otherwise display F for finished

;   Show F in display forever
MESSAGEF             lda      #segF
                     bra      ERRSTOP


;   Get byte count and initialise checksum
LOAD2                bsr      RECVHEXBYTE ; Read the Byte Count for Srec format (note NOT Hex character count)
                     stb      bytecount  
                     lda      bytecount  ; Initialise checksum in A with byte count for new record 
                     dec      bytecount

;   Get two-byte address where to load bytes, accumulating checksum in A
                     bsr      RECVHEXBYTE  ; Get high address byte in B
                     stb      tmpX  
                     pshs     b
                     adda     ,s+          ; add high address to checksum in A
                     dec      bytecount   ; reduce byte count by 1

                     bsr      RECVHEXBYTE  ; Get low address byte in B
                     stb      tmpX+1      ; Save low address byte in tmpX
                     pshs     b
                     adda     ,s+          ; add low address to checksum in A
                     ldx      tmpX        ; Load address in X for use row data bytes storage  
LDLOOP               dec      bytecount   ; reduce byte count by 1

                     beq      LDDONE       ; No more bytes? just checksum left
                     bsr      RECVHEXBYTE  ; Get next hex characters and convert to bytes in B
                     stb      ,x+          ; Save to memory at X
                     pshs     b
                     adda     ,s+          ; Add B to checksum held in A     
                     bra      LDLOOP       ; Next byte


;   Verify checksum
LDDONE               bsr      RECVHEXBYTE  ; Get checksum from end of line in B
                     coma                  ; Since complement not twos complement (NEG) as S19 format checksum would not work

                     pshs     b            ; Compare B to checksum (which is complement of sum)
                     cmpa     ,s+          ; to sum of bytes
                     beq      RELOAD       ; If zero then checksum and complement of sum of bytes are equal, get next row in file
                     lda      #segC         ; Checksum not equal so display C error and stop

;   Fall-thru
ERRSTOP              lbsr     CLEARDISP    ; Clear display then show charcters in A (F Finished, E error in Hex, C Checksum error)
                     sta      dbuf+0  
DEAD                 lbsr     DISPRESH
                     bra      DEAD

;   Receive one byte from serial port in A
RECVBYTE             lda     #STATUSRDRF   ; Mask Receive Data Register Full
RECVBYTE1            bita    SERIALSTATUS  ; Check RDRF, 1 means has data, 0 loop back
                     beq     RECVBYTE1     ; Keep checking until read for more data
                     lda     SERIALDATA    ; Read data from serial port, should clear RDRF
                     rts

;   Receive two ASCII characters, convert to a byte and return in B, invalid Hex display E and stop 
RECVHEXBYTE          pshs    a             ; Save A to stack as will have checksum
                     bsr     RECVBYTE
                     bsr     HEXTOBIN      ; Convert ASCII Hex to Low Nibble
                     lsla                  ; Shift to high nibble
                     lsla
                     lsla
                     lsla
                     tfr     a,b           ; Save high nibble to B

                     bsr     RECVBYTE
                     bsr     HEXTOBIN      ; Convert ASCII Hex to Low Nibble
                     pshs    a             ; Add A to B via stack
                     addb    ,s+               
               
                     puls    a,pc           ; B will have returned two chracter Hex Byte value

; Take ASCII character in A and convert to 4 bit Low nibble in A
HEXTOBIN             cmpa     #'0          ; Compare with ASCII 0
                     bmi      HEXERR       ; Less than 0 then error
                     cmpa     #'9          ; Compare with ASCII 9
                     ble      HEXRTS       ; Less than or equal then numeric, return value
                     cmpa     #'A          ; Compare with ASCII A
                     bmi      HEXERR       ; Less then A then error
                     cmpa     #'F          ; Compare with ASCII F
                     bgt      HEXERR       ; Greater than F then error
                     suba     #7           ; A=$41/65 becomes $3A/58 F=$46/70 becomes $3F/63
HEXRTS               anda     #$0F         ; Mask high nibble to convert from ASCII to binary 4 low bits      
                     rts
               
HEXERR               lda      #segE         ; 7 Segment E
                     bra      ERRSTOP      ; Will display E - error as invalid Hex character


;   Save memory to serial port Prompt S and get start address
SAVE                 ldx      #segS
                     stx      <dbuf+4
                     lbsr     BADDR
                     stx      <ptr

;   Prompt F and get finish address
                     lda      #segF
                     sta      <dbuf+5
                     lbsr     BADDR
                     stx      <addr

;   Set up serial output port - RESET should go in main reset code
                     lda      #CTRLRESET  ; Master Reset bits
                     sta      SERIALCTRL ; Serial Port Control Master Reset
                     lda      #CTRLDIVIDE64 | CTRLWORD8N1S | CTRLRTSLOW
                     sta      SERIALCTRL ; Serial Port Divide by 64 (3.6864Mhz Xtal /6 /64 =9600 ), 8 Bits No Parity 1 Stop, /RTS Low - send data

SAVE1 
;   Send block start
SAVE4                lbsr     SENDSTART
                     lda      #$FF          ; This is effectively -1
                     ldb      <ptr+1
                     pshs     b
                     suba     ,s+           ; Subtract -1 from b gives +1 - not sure why do it this way 

;   Send length code and init checksum
SAVE2                anda     #$0F          ; Do maximum of 16 bytes (0-F), but need to allow for data byte 0 in length calculation
                     adda     #$04          ; Length+2, Checksum+1 data0+1 total 4
                     lbsr     SENDHEXA      ; Send the length in Hex
                     sta      <chksum       ; Start the checksum with the length value

;   Send (half of) address and add to checksum
                     ldx      #ptr          ; Get most significant byte of start address
                     lbsr     SENDBYTEX     ; Send MSB as Hex
                     adda     <chksum       ; Add byte value to checksum
                     sta      <chksum 

;   Send other half of address and add to checksum
                     leax     $01,x         ; Get the least significant byte of the start address
                     lbsr     SENDBYTEX     ; Send LSB as Hex
                     adda     <chksum       ; Add byte value to checksum 
                     sta      <chksum 
                     ldx      <ptr

;   Send a data byte and add to checksum
SAVE3                bsr      SENDBYTEX     ; Send the byte pointed to by X a Hex
                     adda     <chksum       ; Add data byte to checksum
                     sta      <chksum 
                     cmpx     <addr
                     beq      ENDBLOCK      ; Has X reached the Finish Address
                     leax     $01,x         ; Increment X to the next byte
                     stx      <ptr          ; Update ptr to current X value
                     lda      <ptr+1        ; Get the LSB value
                     bita     #$0F          ; Only check the least significant nibble
                     bne      SAVE3         ; Keep sending until 0 (we have done xF byte)

;   Send checksum
ENDBLOCK             lda      <chksum       ; Done block of 16 bytes (other than 1st or last block)
                     coma                   ; Complement checksum (reverse all bits)
                     bsr      SENDHEXA      ; Send checksum as 2 Hex characters
                     lda      #asciiCR      ; Send CR LF to end line
                     lbsr     SEND     
                     lda      #asciiLF    
                     bsr      SEND

;   Test if all finished
                     cmpx     <addr         ; Check X against Finish Address
                     beq      SENDEOF       ; Send S9 End of File if X=Finish Address

;   Test if another full block to send
                     lda      <ptr          ; Compare current byte MSB with finish MSB
                     cmpa     <addr
                     bmi      SAVE4
                     ldb      <ptr+1
                     lda      <addr+1
                     anda     #$F0
                     pshs     b
                     cmpa     ,s+
                     bne      SAVE4

;   Last block may be short
                     bsr      SENDSTART
                     lda      <addr+1
                     bra      SAVE2


;   Send S19 End of file 53 39 30 33 30 30 30 30 42 46 = 'S9030000FC'
SENDEOF              ldy      #SENDEOFMSG
                     bsr      SENDMSG
                     lbra     MESSAGEF 


;   Send S19 file format start line 'S1'
SENDSTART            ldy      #SENDSTARTMSG
                     bsr      SENDMSG
                     rts


;   Transmit byte from (X) and convert from 8 bits to two ASCII HEX characters and send over serial port
SENDBYTEX            lda      ,x       

; Convert byte in A into Hex as two ASCII characters and send over serial port
SENDHEXA
                     pshs     a            ; Save byte value as need to return since used for checksum calc
                     pshs     a            ; Save again so we can mask top and lower nibbles
                     anda     #%11110000   ; Mask High nibble
                     lsra                  ; Shift to Low nibble
                     lsra 
                     lsra
                     lsra

                     adda     #$90          ; LSB to ASCII Hex as per page 7-2 of Leventhal 0-9  $30-$39 A-F $41-$46
                     daa                    ; DAA on adding $90 sets carry which Makes 0-9 3x and A-F be 4x in adca
                     adca     #$40          ; Add $40 makes 0-9 D0-D9  in Decimal 130-139, A-F A0-A5 +$40 + Carry in decimal 141-146 = $41-$46
                     daa                    ; Strips 100 from result to give 30-39 and 41-46
                     bsr      SEND

                     puls     a 
                     anda     #%00001111    ; Mask Low nibble

                     adda     #$90          ; LSB to ASCII Hex as per page 7-2 of Leventhal, same as above
                     daa
                     adca     #$40
                     daa
                     bsr      SEND
                     puls     a
                     rts

;   Transmit byte from A
SEND                 pshs    a
                     nop                   ; Allow for SWI if needed 
                     lda     #STATUSTDRE   ; Mask Transmit Data Register Empty
SEND1                bita    SERIALSTATUS  ; Check TDRE, 1 means ready for more data, 0 loop back
                     beq     SEND1         ; Keep checking until ready for more data
                     nop                   ; Allow for SWI if needed 
                     puls    a             ; 

                     sta     SERIALDATA
                     nop
                     rts                   ; Return


; Send NULL terminated list of BYTES pointed to by Y to serial port
SENDMSG              lda      ,y+
                     beq      SENDMSG2
                     bsr      SEND
;                    nop                 ; Allow SWI to be entered and continue
                     bra      SENDMSG

SENDMSG2             rts

;   Send S19 Start of data S1                         
SENDSTARTMSG         FCB      asciiS, ascii1, asciiNull    

;   Send S19 End of file 53 39 30 33 30 30 30 30 46 43 = 'S9030000FC'
SENDEOFMSG           FCB      asciiS, ascii9, ascii0, ascii3, ascii0, ascii0, ascii0, ascii0, asciiF, asciiC, asciiCR, asciiLF, asciiNull    


                     FCB  $00, $00, $00, $00, $00, $00, $00
;                    FCB  $00, $00, $00, $00, $00, $00, $00
;                    FCB  $00, $00, $00, $00, $00               

; Interupt vector redirect code loads vector from memory to X then jumps to vector in X
; Only NMI Vector is initialised at Reset, others need to be set first
SWI2INTR             ldx      SWI2VEC           
                     jmp      ,x

SWI3INTR             ldx      SWI3VEC           
                     jmp      ,x

FIRQINTR             ldx      FIRQVEC           
                     jmp      ,x

INTRQ                ldx      IRQVEC            
                     jmp      ,x

NMINTR               ldx      NMIVEC            
                     jmp      ,x

                     FCB  $$00 ; Remove 5 $00 to allow for DISPRESH fix and extra 3 for DISPRESET


;   Interrupt vectors point to stubs above which allow vector to be overriden in RAM
;   Reset, NMI and SWI are defined by default, others need to be set first
                     FDB  SWI3INTR     ; SWI3 $FF, $D5     ; <$FFF2>

                     FDB  SWI2INTR     ; SWI2 $FF, $D0     ; <$FFF4>

                     FDB  FIRQINTR     ; FIRQ $FF, $DA     ; <$FFF6>

                     FDB  INTRQ        ; IRQ $FF, $DF      ; <$FFF8>

SWINTR               FDB  REGDISPSWI   ; SWI $FD, $F4      ; <$FFFA>

                     FDB  NMINTR       ; NMI $FF, $E4      ; <$FFFC>

RESETBOOT            FDB  RESET        ; RESET $FD, $89    ; <$FFFE>

