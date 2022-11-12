; C:\Users\Dave\OneDrive\Documents\_6809\asm6809-2.12-w64\asm6809.exe -B -l hello.txt hello.asm -o hello.bin
; C:\Users\Dave\OneDrive\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l hello.txt hello.asm -o hello.bin

                    ORG $1000
start               ldx      #message
                    ldy      #$13FA
loop                lda ,x+
                    sta ,y+
                    beq end
                    bra loop
end                 jsr >$7c7b ; DISPRESH Not changed with 2.1 Display fix
                    bra end
message             FCB     $67,$79,$70,$70,$7E,$00 ; HELLO
;message             FCB     $6E,$78,$7D,$3F,$7F,$3F ; NC6809
               


