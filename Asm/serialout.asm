; C:\Users\Dave\OneDrive\Documents\_6809\asm6809-2.12-w64\asm6809.exe -B -l serialout.txt serialout.asm -o serialout.bin
; C:\Users\Dave\OneDrive\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l serialout.txt serialout.asm -o serialout.srec

; Configure MC6850 ACIA for sending data at 9600 Baud 8 Bits, N parity 1 Stop Bit

SERIALCTRL          EQU  $00           ; Use DP $50   $5000
SERIALSTATUS        EQU  $00           ; Use DP $50   $5000
SERIALDATA          EQU  $01           ; Use DP $50   $5001

CTRLRESET           EQU  %00000011       ; CR1, CR0 1,1 for Master Reset
CTRLDIVIDE16        EQU  %00000001       ; CR1, CR0 0,1 for divide by 16
CTRLDIVIDE64        EQU  %00000010       ; CR1, CR0 1,0 for divide by 64
CTRLWORD8N1S        EQU  %00010100       ; CR4, CR3, CR2 1, 0, 1 for 8 Bits, 1 Stop No Parity
CTRLRTSLOW          EQU  %00000000       ; CR6, CR5, 0,0 RTS low IRQ disabled, requests data
CTRLRTSHIGH         EQU  %01000000       ; CR6, CR5, 1,0 RTS high IRQ disabled, blocks receiving data
CTRLIRQENABLED      EQU  %10000000       ; CR7, 1 IRQ enabled

STATUSRDRF          EQU  %00000001       ; Receive Data Register Full Bit 0
STATUSTDRE          EQU  %00000010       ; Transmit Data Register Empty Bit 1
STATUSDCD           EQU  %00000100       ; Data Carrier Detect Bit 2
STATUSCTS           EQU  %00001000       ; Clear To Send Bit 3
STATUSFE            EQU  %00010000       ; Framing Error Bit 4
STATUSOVRN          EQU  %00100000       ; Receiver Overrun Bit 5
STATUSPE            EQU  %01000000       ; Parity Error Bit 6
STATUSIRQREQ        EQU  %10000000       ; Interupt Request /IRQ Bit 7

EOS                 EQU  $00      end of string

                    ORG $1000
SerialStart         lds      #$1200      ;System Stack init
                    lda      #$50             
                    tfr      a,dp        ; Set Direct Page Register to Serial Port Base Address       
                    lda      #CTRLRESET  ; Master Reset bits
                    sta      <SERIALCTRL ; Serial Port Control Master Reset
					
             
InitSerial          lda      #CTRLDIVIDE64 | CTRLWORD8N1S | CTRLRTSLOW
                    sta      <SERIALCTRL ; Serial Port Divide by 64 (3.6864Mhz Xtal /6 /64 =9600 ), 8 Bits No Parity 1 Stop, /RTS Low - send data

                    ldx      #message    ; Point X to message to send NULL delimited
send                lda      ,x+
                    beq      SerialEnd
                    jsr      send_char
                    jmp      send

SerialEnd           swi                  ; Software Interupt
                    jmp     SerialStart  ; If Continue from SWI


send_char           pshs    a
                    lda     #STATUSTDRE   ; Mask Transmit Data Register Empty
send_char1          bita    <SERIALSTATUS ; Check TDRE, 1 means ready for more data, 0 loop back
                    beq     send_char1    ; Keep checking until read for more data
                    puls    a             ; 

                    sta     <SERIALDATA
                    rts                   ; Return

message             FCC     "Hello, world!"
                    FCB     EOS


