; C:\Users\Dave\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l crt8.txt crt8.asm -o crt8.srec
; Updated 03 Feb 2023 for testing 'snow' updating video RAM

PORTB               EQU $D001
CTRLB               EQU $D003

CRTCAR              EQU  $D400
CRTCDR              EQU  CRTCAR+1
VRAM                EQU  $8000
VRAMTOP             EQU  $BFFF ; All Video RAM so nothing between frames during VS
ASCII0              EQU  $30
ASCIISPACE          EQU  $20
COLSPERROW          EQU  $50
ROWSPERPAGE         EQU  $1E
ROWSPERPAGEBCD      EQU  $30
BYTESPERCHAR        EQU  $02
CHARCOLOUR          EQU  %11110010 ; White foreground, blue background

NCCLEARSCREEN       EQU  $F6DE
NCCHROUT            EQU  $F653
NCDISABLECRTCCURSOR EQU  $F74D

DISPROW             EQU  $7FC0                  
DISPCOL             EQU  $7FC1

EOS                 EQU  $00      end of string

                    ORG  $0000
                    
CRTStart            ;jsr      NCCLEARSCREEN
CLRSCREEN           ldx      #VRAM
                    lda      #ASCIISPACE
                    ldb      #CHARCOLOUR
CLEARSCREEN1        lda      #ASCIISPACE
                    stb      ,X+
                    sta      ,X+         ; Could Use D and X++
                    lda      #$DB   ; Solid 8 x 16
                    stb      ,X+
                    sta      ,X+         ; Could Use D and X++
                    cmpx     #VRAMTOP
                    ble      CLEARSCREEN1

                    jsr      NCDISABLECRTCCURSOR

TESTLOOP            lda      #$1D              ; 29 Lines down
                    sta      DISPROW           ; Store Row
                    lda      #$4F              ; 79 Characters across
                    sta      DISPCOL           ; Set Port B control register for Port Output, will be keypad row
                    lda      #$F2
                    ldb      #$DB            ; DB solid
                    jsr      NCCHROUT
;WAITLOOP            ldy      #1
;                    leay     -1,y
;                    bne      WAITLOOP
                    lda      #$1D              ; 29 Lines down
                    sta      DISPROW           ; Store Row
                    lda      #$4F              ; 79 Characters across
                    sta      DISPCOL           ; Set Port B control register for Port Output, will be keypad row
                    lda      #$F2
                    ldb      #$20              ; B1 Checkered
                    jsr      NCCHROUT
                    jmp      TESTLOOP
                    
                    
WAITFORVSDE         pshs     a
TESTDE              lda      #$80          ; Mask PB7 on Port B of PIA
                    bita     PORTB        ; Test PB7 for Display Enable, when 1 then screen blanked
                    beq      TESTDE       
                    bita     PORTB        ; Test PB7 again
                    beq      TESTDE       ; If 0 then only HS or end of VS, so continue to wait for DE=1 for more than 6ms
                    puls     a,pc
                    
                    
                    
