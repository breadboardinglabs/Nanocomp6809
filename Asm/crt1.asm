; 
; V1 20/Nov/2022 Using 25.175 Mhz Osc and 64K Memory map CRTC at D400

CRTCAR              EQU  $D400
CRTCDR              EQU  CRTCAR+1

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

CRTEND
                    swi
                    
CRTCTAB             FCB      $63         ; R0 H 62 to 64 Total 98 was 99 (Changed to 98 with 25Mhz as 59Hz not 60Hz)
                    FCB      $50         ; R1 H Displayed 80
                    FCB      $55         ; R2 H from 53 to 55 Sync Position 83
                    FCB      $06         ; R3 H Sync Width 6
                    FCB      $1F         ; R4 V Total 31
                    FCB      $07         ; R5 V Total Adjust (was 13)
                    FCB      $1E         ; R6 V Displayed 25 (was 30)
                    FCB      $1E         ; R7 V Sync Position 29 (was 30)
                    FCB      $00         ; R8 Interlace mode - Non Interlaced
                    FCB      $0F         ; R9 Maximum Scan Line Address 
                    FCB      $00         ; R10 Cursor Start - Slow Blink C0 + Line 13 Start
                    FCB      $00         ; R11 Cursor End - Slow Blink C0 + Line 15 Finish
                    FCB      $00,$00     ; R12,R13 Start Address
                    FCB      $00,$00     ; R14,R15 Cursor Address

;CRTCTAB             FCB      $64         ; R0 H 62 to 64 Total 98 was 99 (Changed to 98 with 25Mhz as 59Hz not 60Hz)
;                    FCB      $50         ; R1 H Displayed 80
;                    FCB      $55         ; R2 H from 53 to 55 Sync Position 83
;                    FCB      $06         ; R3 H Sync Width 6
;                    FCB      $1F         ; R4 V Total 31
;                    FCB      $0D         ; R5 V Total Adjust (was 13)
;                    FCB      $19         ; R6 V Displayed 25 (was 30)
;                    FCB      $1C         ; R7 V Sync Position 29 (was 30)
;                    FCB      $00         ; R8 Interlace mode - Non Interlaced
;                    FCB      $0F         ; R9 Maximum Scan Line Address 
;                    FCB      $6D         ; R10 Cursor Start - Slow Blink C0 + Line 13 Start
;                    FCB      $6F         ; R11 Cursor End - Slow Blink C0 + Line 15 Finish
;                    FCB      $00,$00     ; R12,R13 Start Address
;                    FCB      $00,$00     ; R14,R15 Cursor Address
                    
                    
