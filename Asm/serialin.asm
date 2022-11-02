; C:\Users\Dave\OneDrive\Documents\_6809\asm6809-2.12-w64\asm6809.exe -B -l serialin.txt serialin.asm -o serialin.bin
; C:\Users\Dave\OneDrive\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l serialin.txt serialin.asm -o serialin.srec
SERIALCTRL          EQU  $00           ; Use DP $50   $5000
SERIALSTATUS        EQU  $00           ; Use DP $50   $5000
SERIALDATA          EQU  $01           ; Use DP $50   $5001

CTRLRESET           EQU  %00000011       ; CR1, CR0 1,1 for Master Reset
CTRLDIVIDE16        EQU  %00000001       ; CR1, CR0 0,1 for divide by 16 1,0 for divide by 64
CTRLDIVIDE64        EQU  %00000010       ; CR1, CR0 1,0 for divide by 64
CTRLWORD8N1S        EQU  %00010100       ; CR4, CR3, CR2 1, 0, 1 for 8 Bits, 1 Stop No Parity
CTRLRTSLOW          EQU  %00000000       ; CR6, CR5, 0,0 RTS low IRQ disabled, requests data
CTRLRTSHIGH         EQU  %01000000       ; CR6, CR5, 1,0 RTS high IRQ disabled, prevents receiving data
CTRLIRQENABLED      EQU  %10000000       ; CR7, 1 IRQ enabled

STATUSRDRF          EQU  %00000001       ; Receive Data Register Full Bit 0
STATUSTDRE          EQU  %00000010       ; Transmit Data Register Empty Bit 1
STATUSDCD           EQU  %00000100       ; Data Carrier Detect Bit 2
STATUSCTS           EQU  %00001000       ; Clear To Send Bit 3
STATUSFE            EQU  %00010000       ; Framing Error Bit 4
STATUSOVRN          EQU  %00100000       ; Receiver Overrun Bit 5
STATUSPE            EQU  %01000000       ; Parity Error Bit 6
STATUSIRQREQ        EQU  %10000000       ; Interupt Request /IRQ Bit 7

CODESTART           EQU  $1000
CODELENGTH          EQU  $3C

EOS                 EQU  $00      end of string

                    ORG $1200
                                         ; Read Serial Port and load at $1000 for upto $3B bytes (length of Serial Out code)					
SerialInStart       lds      #$1300      ; System Stack init
                    lda      #$50             
                    tfr      a,dp        ; Set Direct Page Register to Serial Port Base Address       
                    lda      #CTRLRESET  ; Master Reset bits
                    sta      <SERIALCTRL ; Serial Port Control Master Reset

             
InitSerial          lda      #CTRLDIVIDE64 | CTRLWORD8N1S | CTRLRTSLOW
                    sta      <SERIALCTRL ; ; Serial Port Divide by 64 (3.6864Mhz Xtal /6 /64 =9600 ), 8 Bits No Parity 1 Stop, /RTS Low - receive data, DCD wired low

                    ldx      #CODESTART  ; Point X to start address where to load file data from serial port
                    ldb      #CODELENGTH ; We will decrement B until 0

receive             jsr      receive_char             
                    sta      ,x+
                    decb
                    beq      SerialInEnd
                    jmp      receive

SerialInEnd         swi                  ; Software Interupt
                    jmp     SerialInStart  ; If Continue from SWI


receive_char        lda     #STATUSRDRF   ; Mask Receive Data Register Full
receive_char1       bita    <SERIALSTATUS ; Check RDRF, 1 means has data, 0 loop back
                    beq     receive_char1 ; Keep checking until read for more data
                    lda     <SERIALDATA   ; Read data from serial port, should clear RDRF
                    rts                   ; Return 
