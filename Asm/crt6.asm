; C:\Users\Dave\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l crt6.txt crt6.asm -o crt6.srec
; Updated 04 Jan 2023 for Dual 32K(8K used) VRAM

PORTB               EQU $D001
CTRLB               EQU $D003

CRTCAR              EQU  $D400
CRTCDR              EQU  CRTCAR+1
VRAM                EQU  $8000
VRAMTOP             EQU  $92C0 ; (30x80x2)
ASCII0              EQU  $30
ASCIISPACE          EQU  $20
COLSPERROW          EQU  $50
ROWSPERPAGE         EQU  $1E
ROWSPERPAGEBCD      EQU  $30
BYTESPERCHAR        EQU  $02
CHARCOLOUR          EQU  %11110010 ; White foreground, blue background
CRTLINE1            RMB  2 ; CRTC Address where 1st Line starts,  
ENDVRAM             RMB  2

EOS                 EQU  $00      end of string

                    ORG  $1000
                    
CRTStart            lds      #$7FD0      ;System Stack init
                    clra
                    sta      CTRLB            ; Set Port B to data direction register
                    lda      #$0F              ; set bits 0-3 for output, 4-7 for input
                    sta      PORTB            ; Set Port B to output for bits 0-3
                    lda      #$04              ; Set control register b2=1 Output Register
                    sta      CTRLB            ; Set Port B control register for Port Output, will be keypad row

CLRSCREEN           ldx      #VRAM
                    lda      #ASCIISPACE
                    ldb      #CHARCOLOUR
CLEARSCREEN1        lbsr     WAITFORVSDE
                    ldy      #80
CLEARSCREEN2        stb      ,X+
                    sta      ,X+         ; Could Use D and X++
                    leay     -1,Y
                    bne      CLEARSCREEN2
                    cmpx     #VRAMTOP
                    ble      CLEARSCREEN1
                    
TOPLINE             ldx      #VRAM+1
                    lda      #$01
TOPLINE1            lbsr      PRINTDIGIT
                    adda      #$01
                    daa
                    cmpx      #VRAM + COLSPERROW * BYTESPERCHAR 
                    ble       TOPLINE1
; Print 10's under 1st line

SECLINE             ldx      #VRAM + 1 + COLSPERROW * BYTESPERCHAR + 9 * BYTESPERCHAR ; Start at 10th Column
                    lda      #$1
SECLINE1            lbsr      PRINTDIGIT ; This will increase X by 2
                    leax     9*BYTESPERCHAR,X
                    adda     #$01
                    daa
                    cmpx     #VRAM + 2 * COLSPERROW * BYTESPERCHAR ; 2 Bytes per character 0/even is character 1/odd is colour
                    ble      SECLINE1


LOWLINE             ldx      #VRAM + 1 + ROWSPERPAGE * COLSPERROW * BYTESPERCHAR - COLSPERROW * BYTESPERCHAR 
                    lda      #$01
LOWLINE1            bsr      PRINTDIGIT
                    adda     #$01
                    daa
                    cmpx     #VRAM + COLSPERROW * BYTESPERCHAR * ROWSPERPAGE; 2 Bytes per character 0/even is character 1/odd is colour
                    ble      LOWLINE1

LOWSECLINE          ldx      #VRAM + 1 + ROWSPERPAGE * COLSPERROW * BYTESPERCHAR - 2 * COLSPERROW * BYTESPERCHAR + 9 * BYTESPERCHAR ; Start at 10th Column
                    lda      #$1
LOWSECLINE1         bsr      PRINTDIGIT ; This will increase X by 2
                    leax     9*BYTESPERCHAR,X
                    adda     #$01
                    daa
                    cmpx     #VRAM + ROWSPERPAGE * COLSPERROW * BYTESPERCHAR - COLSPERROW * BYTESPERCHAR
                    ble      LOWSECLINE1


LINENO              lda      #$02
                    ldx      #VRAM + 1 + COLSPERROW * BYTESPERCHAR
LINENO1             bsr      PRINTNUM
                    leax     76*BYTESPERCHAR,X  ; Move to end of line
                    bsr      PRINTNUM ; X should be start of next line after this
                    adda     #$01
                    daa
                    cmpa     #ROWSPERPAGEBCD
                    bne      LINENO1
                    ;SWI
                    ; +7 (4 char), 16 x 4 char= 64=128 bytes + 25
                    ; Add 32 for end of line (16 char)
CHARDUMP            clra      ; Display two digit character code, character, 1 spaces for 0-255, start at Col 4 (add 7)
                    ldx       #VRAM - 32 + 7 + 2 * COLSPERROW * BYTESPERCHAR
CHARDUMP1           clrb
                    leax      16*BYTESPERCHAR,X
CHARDUMP2           bsr       PRINTHEX
                    sta       ,X++
                    leax      1*BYTESPERCHAR,X
                    inca
                    incb
                    cmpb      #$10
                    bne       CHARDUMP2
                    tsta
                    bne       CHARDUMP1

                    ; +7 (4 char), 16 x 4 char= 64=128 bytes + 25
                    ; Add 32 for end of line (16 char)
COLDUMP             clra      ; Display two digit character code, character, 1 spaces for 0-255, start at Col 4 (add 7)
                    ldx       #VRAM - 32 + 7 + 20 * COLSPERROW * BYTESPERCHAR
                    ldb       #$20
                    leax      32,X
COLDUMP1            bsr       PRINTHEX
;COLDUMP1            leax      4,X
                    bsr      WAITFORVSDE
                    sta       -1,X
                    stb       ,X
                    leax      4,X
                    inca
                    cmpa      #$10
                    bne       COLDUMP1

CRTEND
                    swi
                    
; 12345678901234567890
; 02       1         2 ..... 02

; Take BCD two digit number in A and print at X
PRINTNUM            pshs     a           ; Save A twice
                    pshs     a
                    anda     #%11110000  ; Mask LSB
                    lsra                 ; Rotate MSB down to LSB
                    lsra
                    lsra
                    lsra
                    bsr PRINTDIGIT
                    puls     a           ; Restore A
                    anda     #%00001111  ; Mask MSB
                    bsr PRINTDIGIT
                    puls     a,pc

; Take Lower 4 bits in A and print ASCII digit at X and increment X
PRINTDIGIT          pshs     a
                    anda     #%00001111  ; Strip any MSB
                    adda     #ASCII0     ; Add 48 decimal, 0=$30, 1=$31
                    bsr      WAITFORVSDE
                    sta      ,X++
                    puls     a,pc

PRINTHEX            pshs     a            ; Save byte value as need to return since used for checksum calc
                    pshs     a            ; Save again so we can mask top and lower nibbles
                    anda     #%11110000   ; Mask High nibble
                    lsra                  ; Shift to Low nibble
                    lsra 
                    lsra
                    lsra
               
                    adda     #$90          ; LSB to ASCII Hex as per page 7-2 of Leventhal
                    daa                    ; 0-9  $30-$39 A-F $41-$46
                    adca     #$40
                    daa
                    bsr      WAITFORVSDE
                    sta     ,X++
               
                    puls     a 
                    anda     #%00001111    ; Mask Low nibble

                    adda     #$90          ; LSB to ASCII Hex as per page 7-2 of Leventhal
                    daa
                    adca     #$40
                    daa
                    sta     ,X++
                    puls     a
                    rts
                    
WAITFORVSDE         pshs     a
TESTDE              lda      #$80          ; Mask PB7 on Port B of PIA
                    bita     PORTB        ; Test PB7 for Display Enable, when 1 then screen blanked
                    beq      TESTDE       
                    bita     PORTB        ; Test PB7 again
                    beq      TESTDE       ; If 0 then only HS or end of VS, so continue to wait for DE=1 for more than 6ms
                    puls     a,pc
                    
CRTCTAB             FCB      $63         ; R0 H 62 to 64 Total 99
                    FCB      $50         ; R1 H Displayed 80
                    FCB      $53         ; R2 H from 53 to 55 Sync Position 83
                    FCB      $06         ; R3 H Sync Width 6
                    FCB      $1F         ; R4 V Total 31
                    FCB      $14         ; R5 V Total Adjust (was 13/$0D)
                    FCB      $1E         ; R6 V Displayed 30
                    FCB      $1F         ; R7 V Sync Position 31
                    FCB      $00         ; R8 Interlace mode - Non Interlaced
                    FCB      $0F         ; R9 Maximum Scan Line Address 
                    FCB      $6D         ; R10 Cursor Start - Slow Blink C0 + Line 13 Start was 6D Cursor off $20
                    FCB      $6F         ; R11 Cursor End - Slow Blink C0 + Line 15 Finish 6F
                    FCB      $00,$00     ; R12,R13 Start Address
                    FCB      $00,$00     ; R14,R15 Cursor Address
                    
                    
