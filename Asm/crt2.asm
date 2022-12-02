; C:\Users\Dave\OneDrive\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l crt2.txt crt2.asm -o crt2.srec
; V2 31/dec/2021 Using 25.175 Mhz Osc not 25Mhz crystal

CRTCAR              EQU  $6000
CRTCDR              EQU  CRTCAR+1
VRAM                EQU  $2000
ASCII0              EQU  $30
ASCIISPACE          EQU  $20
COLSPERROW          EQU  $50
ROWSPERPAGE         EQU  $19
ROWSPERPAGEBCD      EQU  $25
BYTESPERCHAR        EQU  $02
CHARCOLOUR          EQU  %00001111 ; Black background, white foreground

EOS                 EQU  $00      end of string

                    ORG  $1000
                    
CRTStart            lds      #$1200      ;System Stack init
                    clrb
                    ldx      #CRTCTAB
CRTCLOOP            stb      CRTCAR      ; Save Rn in CRTC Address Register        
                    lda      ,X+        ; Get Rn value from CRTC Table single + for +1, ++ is +2 and wrong!
                    sta      CRTCDR      ; Save Rn data value in CRTC Data Register
                    incb
                    cmpb     #$10         ; Have we processed all register values
                    bne      CRTCLOOP

CLRSCREEN           ldx      #VRAM
                    lda      #ASCIISPACE
                    ldb      #CHARCOLOUR
CLEARSCREEN1        stb      ,X+
                    incb
                    sta      ,X+         ; Could Use D and X++
                    cmpx     #VRAM + COLSPERROW * BYTESPERCHAR * ROWSPERPAGE
                    ble      CLEARSCREEN1
                    
TOPLINE             ldx      #VRAM+1
                    lda      #$01
TOPLINE1            bsr      PRINTDIGIT
                    adda      #$01
                    daa
                    cmpx      #VRAM + COLSPERROW * BYTESPERCHAR ; 2 Bytes per character 0/even is character 1/odd is colour
                    ble       TOPLINE1
; Print 10's under 1st line

SECLINE             ldx      #VRAM + 1+  COLSPERROW * BYTESPERCHAR + 18 ; Start at 10th Column
                    lda      #$1
SECLINE1            bsr      PRINTDIGIT ; This will increase X by 2
                    leax     18,X
                    adda     #$01
                    daa
                    cmpx     #VRAM + 2 * COLSPERROW * BYTESPERCHAR ; 2 Bytes per character 0/even is character 1/odd is colour
                    ble      SECLINE1


LOWLINE             ldx      #VRAM + 1 + ROWSPERPAGE * COLSPERROW * BYTESPERCHAR - COLSPERROW * BYTESPERCHAR 
                    lda      #$01
LOWLINE1            bsr      PRINTDIGIT
                    adda      #$01
                    daa
                    cmpx      #VRAM + COLSPERROW * BYTESPERCHAR * ROWSPERPAGE; 2 Bytes per character 0/even is character 1/odd is colour
                    ble       LOWLINE1

LINENO              lda       #$02
                    ldx       #VRAM + 1 + COLSPERROW * BYTESPERCHAR
LINENO1             bsr       PRINTNUM
                    leax      152,X  ; Move to end of line
                    bsr       PRINTNUM ; X should be start of next line after this
                    adda      #$01
                    daa
                    cmpa      #ROWSPERPAGEBCD
                    bne       LINENO1

CHARDUMP            clra      ; Display two digit character code 2 spaces, character, 3 spaces for 0-255, start at Col 2
                    ldx       #VRAM + 7 + 4 * COLSPERROW * BYTESPERCHAR
CHARDUMP1           bsr       PRINTHEX
                    ;leax      2,X
                    sta       ,X++
                    leax      2,X
                    inca
;                    cmpa      #7
                    bne       CHARDUMP1

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
                    
CRTCTAB             FCB      $64         ; R0 H 62 to 64 Total 98 was 99 (Changed to 98 with 25Mhz as 59Hz not 60Hz)
                    FCB      $50         ; R1 H Displayed 80
                    FCB      $55         ; R2 H from 53 to 55 Sync Position 83
                    FCB      $06         ; R3 H Sync Width 6
                    FCB      $1F         ; R4 V Total 31
                    FCB      $0D         ; R5 V Total Adjust (was 13)
                    FCB      $19         ; R6 V Displayed 25 (was 30)
                    FCB      $1C         ; R7 V Sync Position 29 (was 30)
                    FCB      $00         ; R8 Interlace mode - Non Interlaced
                    FCB      $0F         ; R9 Maximum Scan Line Address 
                    FCB      $6D         ; R10 Cursor Start - Slow Blink C0 + Line 13 Start
                    FCB      $6F         ; R11 Cursor End - Slow Blink C0 + Line 15 Finish
                    FCB      $00,$00     ; R12,R13 Start Address
                    FCB      $00,$00     ; R14,R15 Cursor Address
                    
                    
