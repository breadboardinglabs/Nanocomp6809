; C:\Users\Dave\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l crt11.txt crt11.asm -o crt11.srec
; cd Documents\_Breadboarding\VideoController\v11_640x480_Graphics
; Updated 01 Mar 2023 for testing 640 x 480 2 colour graphics

PORTB               EQU $D001
CTRLB               EQU $D003

CRTCAR              EQU  $D400
CRTCDR              EQU  CRTCAR+1
PAGEREG             EQU  $D480
COLOURLATCH         EQU  $D500
VRAM                EQU  $8000
VRAMTOP             EQU  $BFFF ; All Video RAM so nothing between frames during VS
VRAMPAGE3TOP        EQU  $9600 ; Top of visible Page 3 VRAM

ASCII0              EQU  $30
ASCIISPACE          EQU  $20
GRAPHICON           EQU  $FF
GRAPHICOFF          EQU  $00

COLSPERROW          EQU  $50
ROWSPERPAGE         EQU  $1E
ROWSPERPAGEBCD      EQU  $30
BYTESPERCHAR        EQU  $01
ROWSPERCHAR         EQU  $04
BYTESPERROW         EQU  $140   ; 320 Bytes for chracter 'row' of 4 rows

CHARCOLOUR          EQU  %11110010 ; White foreground, blue background
GRAPHICSCOLOUR      EQU  %11110000 ; White foreground, black background
GRAPHICMODE1        EQU  %00001000 ;

NCCLEARSCREEN       EQU  $F6DE
NCCHROUT            EQU  $F653
NCDISABLECRTCCURSOR EQU  $F74D

DISPROW             EQU  $7FC0                  
DISPCOL             EQU  $7FC1

EOS                 EQU  $00      end of string

                    ORG  $0000
                    
CRTStart            clrb
                    ldx      #CRTCTAB
CRTCLOOP            stb      CRTCAR      ; Save Rn in CRTC Address Register        
                    lda      ,X+        ; Get Rn value from CRTC Table single + for +1, ++ is +2 and wrong!
                    sta      CRTCDR      ; Save Rn data value in CRTC Data Register
                    incb
                    cmpb     #$10         ; Have we processed all register values
                    bne      CRTCLOOP

                    lda      #GRAPHICMODE1 ; Set Graphics on first, colour reg won't get databus value without GM enabled
                    sta      PAGEREG
                    lda      #GRAPHICSCOLOUR
                    ldb      #GRAPHICSCOLOUR
                    
                    sta      COLOURLATCH      ; Set Foreground and Background Colour for graphics
CLRSCREEN           lda      #GRAPHICMODE1
                    sta      PAGEREG
                    ldx      #VRAM
                    lda      #$AA
                    ldb      #$55
CLEARSCREEN1        std      ,X++
                    cmpx     #VRAMTOP
                    ble      CLEARSCREEN1
                    lda      #GRAPHICMODE1 + 1
                    sta      PAGEREG
                    ldx      #VRAM
                    lda      #$AA
                    ldb      #$AA
CLEARSCREEN2        std      ,X++
                    cmpx     #VRAMTOP
                    ble      CLEARSCREEN2
                    lda      #GRAPHICMODE1 + 2
                    sta      PAGEREG
                    ldx      #VRAM
                    lda      #$55
                    ldb      #$FF
CLEARSCREEN3        std      ,X++
                    cmpx     #VRAMPAGE3TOP
                    ble      CLEARSCREEN3
                    lda      #GRAPHICMODE1    ; Set back to Page 0
                    sta      PAGEREG

                    jsr      NCDISABLECRTCCURSOR
                    
                    swi
                    ldx      #VRAM
                    lda      #01
TESTLOOP            inca
                    sta      ,X+         ; Could Use D and X++
                    cmpx     #VRAMTOP
                    ble      TESTLOOP
                    swi
                    jmp      TESTLOOP
                    
                    
WAITFORVSDE         pshs     a
TESTDE              lda      #$80          ; Mask PB7 on Port B of PIA
                    bita     PORTB        ; Test PB7 for Display Enable, when 1 then screen blanked
                    beq      TESTDE       
                    bita     PORTB        ; Test PB7 again
                    beq      TESTDE       ; If 0 then only HS or end of VS, so continue to wait for DE=1 for more than 6ms
                    puls     a,pc
                    sta      $7FB9            ; Override CRTC R9 Maximum Scan Line Address 4 rows RA0-RA1

                                         ; Graphics mode 2 640x480 2 Colour            
CRTCTAB             FCB      $63         ; R0 H 62 to 64 Total 98 was 99 (Changed to 98 with 25Mhz as 59Hz not 60Hz)
                    FCB      $50         ; R1 H Displayed 80
                    FCB      $56         ; R2 H from was 53 Sync Position 86
                    FCB      $06         ; R3 H Sync Width 6
                    FCB      $40         ; R4 V Total 64
                    FCB      $05         ; R5 V Total Adjust (was 14)
                    FCB      $3C         ; R6 V Displayed 60 (was 30)
                    FCB      $3D         ; R7 V Sync Position 61
                    FCB      $00         ; R8 Interlace mode - Non Interlaced
                    FCB      $07         ; R9 Maximum Scan Line Address 
                    FCB      $00         ; R10 Cursor Start - Slow Blink C0 + Line 13 Start
                    FCB      $00         ; R11 Cursor End - Slow Blink C0 + Line 15 Finish
                    FCB      $00,$00     ; R12,R13 Start Address
                    FCB      $00,$00     ; R14,R15 Cursor Address               
