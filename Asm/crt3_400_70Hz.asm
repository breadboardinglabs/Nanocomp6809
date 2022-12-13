; C:\Users\Dave\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l crt3_400_70Hz.txt crt3_400_70Hz.asm -o crt3_400_70Hz.srec
; V3 16/Jan/2021 Using 25.175 Mhz Osc not 25Mhz crystal, Better display of character set

CRTCAR              EQU  $D400
CRTCDR              EQU  CRTCAR+1
VRAM                EQU  $8000
ASCII0              EQU  $30
ASCIISPACE          EQU  $20
COLSPERROW          EQU  $50
ROWSPERPAGE         EQU  $19
ROWSPERPAGEBCD      EQU  $30
BYTESPERCHAR        EQU  $01
CHARCOLOUR          EQU  %00001111 ; Black background, white foreground
CRTLINE1            RMB  2 ; CRTC Address where 1st Line starts,  
ENDVRAM             RMB  2

EOS                 EQU  $00      end of string

                    ORG  $0000
                    
CRTStart            lds      #$7FD0      ;System Stack init
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
                    ;ldb      #CHARCOLOUR
CLEARSCREEN1        ;stb      ,X+
                    sta      ,X+         ; Could Use D and X++
                    cmpx     #VRAM + COLSPERROW * BYTESPERCHAR * ROWSPERPAGE
                    ble      CLEARSCREEN1
                    
TOPLINE             ldx      #VRAM
                    lda      #$01
TOPLINE1            lbsr      PRINTDIGIT
                    adda      #$01
                    daa
                    cmpx      #VRAM + COLSPERROW * BYTESPERCHAR ;Only 1 byte per character currently 
                    ble       TOPLINE1
; Print 10's under 1st line

SECLINE             ldx      #VRAM + COLSPERROW * BYTESPERCHAR + 9 ; Start at 10th Column
                    lda      #$1
SECLINE1            bsr      PRINTDIGIT ; This will increase X by 2
                    leax     9,X
                    adda     #$01
                    daa
                    cmpx     #VRAM + 2 * COLSPERROW * BYTESPERCHAR ; 2 Bytes per character 0/even is character 1/odd is colour
                    ble      SECLINE1


LOWLINE             ldx      #VRAM + ROWSPERPAGE * COLSPERROW * BYTESPERCHAR - COLSPERROW * BYTESPERCHAR 
                    lda      #$01
LOWLINE1            bsr      PRINTDIGIT
                    adda      #$01
                    daa
                    cmpx      #VRAM + COLSPERROW * BYTESPERCHAR * ROWSPERPAGE; 2 Bytes per character 0/even is character 1/odd is colour
                    ble       LOWLINE1

LINENO              lda       #$02
                    ldx       #VRAM + COLSPERROW * BYTESPERCHAR
LINENO1             bsr       PRINTNUM
                    leax      76,X  ; Move to end of line
                    bsr       PRINTNUM ; X should be start of next line after this
                    adda      #$01
                    daa
                    cmpa      #ROWSPERPAGEBCD
                    bne       LINENO1
                    ;SWI
                    ; +7 (4 char), 16 x 4 char= 64=128 bytes + 25
                    ; Add 32 for end of line (16 char)
CHARDUMP            clra      ; Display two digit character code, character, 1 spaces for 0-255, start at Col 4 (add 7)
                    ldx       #VRAM - 16 + 7 + 2 * COLSPERROW * BYTESPERCHAR
CHARDUMP1           clrb
                    leax      16,X
CHARDUMP2           bsr       PRINTHEX
                    ;leax      2,X
                    sta       ,X+
                    leax      1,X
                    inca
                    incb
                    cmpb      #$10
                    bne       CHARDUMP2
                    tsta
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
                    sta      ,X+
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
                    sta     ,X+
               
                    puls     a 
                    anda     #%00001111    ; Mask Low nibble

                    adda     #$90          ; LSB to ASCII Hex as per page 7-2 of Leventhal
                    daa
                    adca     #$40
                    daa
                    sta     ,X+
                    puls     a
                    rts
                                         ; 640 x 400 25 lines x 80 Columns 70 Hz
CRTCTAB             FCB      $63         ; R0 H 63 99 
                    FCB      $50         ; R1 H Displayed 80
                    FCB      $52         ; R2 H 53 Sync Position 83 (52 for LCD)
                    FCB      $0c         ; R3 H Sync Width 12
                    FCB      $1B         ; R4 V Total 27
                    FCB      $01         ; R5 V Total Adjust (was 13) (01 for LCD)
                    FCB      $19         ; R6 V Displayed 25 (was 30)
                    FCB      $1A         ; R7 V Sync Position 29 (was 30)
                    FCB      $00         ; R8 Interlace mode - Non Interlaced
                    FCB      $0F         ; R9 Maximum Scan Line Address 
                    FCB      $00         ; R10 Cursor Start - Slow Blink C0 + Line 13 Start
                    FCB      $00         ; R11 Cursor End - Slow Blink C0 + Line 15 Finish
                    FCB      $00,$00     ; R12,R13 Start Address
                    FCB      $00,$00     ; R14,R15 Cursor Address
                    
                    
