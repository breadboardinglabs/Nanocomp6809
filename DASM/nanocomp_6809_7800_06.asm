PORTA                         EQU $4000
PORTB                         EQU $4001
CTRLA                         EQU $4002
CTRLB                         EQU $4003
DEC10K                        EQU $2710
DEC1K                         EQU $03E8
keyCN                         EQU $25
keyG                          EQU $35
keyI                          EQU $32
keyL                          EQU $05
keyM                          EQU $31
keyP                          EQU $15
keyR                          EQU $30
REGDISPCHAR2                  EQU  $7DEF
chksum                        EQU  $13ED
allzeros                      EQU  $0000
               ORG $13D0
usrsp                         RMB 1  ;
               ORG $13E5
;org needs to be 13E4 to have separate chksum			   
;chksum                        RMB 1  ;
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
tmpX                          RMB 2  ;
stackstart                    RMB 2  ;
dbuf                          RMB 6  ;

                     ORG $7800
HEXTODEC             lds      #$1390
HEXTODEC1            jsr      CLEARDISP
                     lda      #$13
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
                     cmpa     #$15
                     lbeq     HEXTODEC141

;   L pressed for Decimal to Hex
                     cmpa     #$05
                     bne      HEXTODEC2
                     ldx      #$1309
HEXTODEC3            jsr      GETKEY

;   I pressed to display Hex
                     cmpa     #$32
                     beq      HEXTODEC4
                     leax     $01,x
                     jsr      HEXCON
                     cmpa     #$09
                     bhi      HEXTODEC1
                     sta      ,x
                     jsr      R7SEG
                     sta      $00F0,x
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

                     FCB  $00, $00, $00, $00, $00, $00, $00, $00, $00, $00

DUCKSHOOT            lds      #$1390
                     lda      #$13
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
                     leau     -$72,pc ; ($7953)
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

CALCOFFSET           lds      #$1390
                     lda      #$13
                     tfr      a,dp
CALCOFFSET1          ldd      #$3D
                     std      <dbuf+4
                     jsr      BADDR
CALCOFFSET2          jsr      GETKEY
                     cmpa     #$32
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
                     cmpa     #$32
                     bne      CALCOFFSET4
                     bra      CALCOFFSET1

CALCOFFSET5          ldd      #$0101
                     std      <dbuf+4
                     bra      CALCOFFSET4

                     FCB  $00, $00

MASTERMIND           lds      #$1390
                     lda      #$13
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
                     sta      $00F9,x
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
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00

KEYCODE              FCB  $22, $24, $02, $12, $14, $00, $10, $04
                     FCB  $01, $11, $03, $13, $23, $33, $21, $20

SVNSEG               FCB  $7E, $06, $5B, $1F, $27, $3D, $7D, $0E
                     FCB  $7F, $3F, $6F, $75, $78, $57, $79, $69

GETKEY               pshs     y,dp,b
                     lda      #$40
                     tfr      a,dp

;   Set up I/O port
GETKEY0              bsr      DISPRESH
                     clra     
                     sta      <CTRLA
                     sta      <CTRLB 
                     sta      <PORTA
                     lda      #$0F
                     sta      <PORTB 
                     lda      #$04
                     sta      <CTRLA
                     sta      <CTRLB 
                     lda      #$FF
GETKEY2              incb     
                     cmpb     #$04
                     beq      GETKEY0
GETKEY1              stb      <PORTB 
                     lda      <PORTA
                     coma     
                     beq      GETKEY2

;   Found a key - decode it
                     stb      $13EC
                     sta      chksum 
                     clra     
                     ldb      #$01
GETKEY4              cmpb     chksum 
                     beq      GETKEY3
                     inca     
                     aslb     
                     beq      GETKEY2
                     bra      GETKEY4

GETKEY3              ldb      $13EC
                     aslb     
                     aslb     
                     aslb     
                     aslb     
                     pshs     b
                     adda     ,s+

;   Wait for the key to be released
                     pshs     a
                     ldy      #$08
GETKEY6              clrb     
GETKEY5              lda      <PORTA
                     coma     
                     bne      GETKEY6
                     decb     
                     bne      GETKEY5
                     leay     -$01,y
                     bne      GETKEY5
                     puls     a,b,dp,y,pc ;(pul? pc=rts)

DISPRESH             pshs     x,b,a

;   Set output ports for display
                     ldx      #PORTA
                     clra     
                     sta      $02,x
                     sta      $03,x
                     lda      #$7F
                     sta      ,x
                     lda      #$0F
                     sta      $01,x
                     lda      #$04
                     sta      $02,x
                     sta      $03,x

;   Initialise loop over digits
                     ldx      #dbuf                   ;They are numbered 4..9 to correspond to outputs from the 7442 decoder.
                     ldb      #$03
DISP2                incb     
                     cmpb     #$0A
                     bne      DISP3

;   Finished
                     puls     a,b,x,pc ;(pul? pc=rts)


;   Light up the next digit
DISP3                stb      PORTB 
                     lda      ,x+
                     coma     
                     sta      PORTA

;   Delay loop
                     lda      #$A0
DISP1                deca     
                     bne      DISP1
                     bra      DISP2

BADDR1               exg      a,b
                     tfr      d,x
                     puls     a,b,pc ;(pul? pc=rts)


;   Input 2 byte address into X; result also at [addr]
BADDR                pshs     b,a
                     clra     
                     clrb     
                     std      dbuf 
                     std      $13FC
                     ldx      #dbuf 
                     bsr      HEXIN
                     pshs     a
                     bsr      HEXIN
                     puls     b
                     bra      BADDR1


;   Input 2 hex digits into A and update display at X
HEXIN                bsr      KEYHEX
                     asla     
                     asla     
                     asla     
                     asla     
                     pshs     a
                     bsr      L7SEG
                     sta      ,x+
                     bsr      KEYHEX
                     adda     ,s+
                     pshs     a
                     bsr      R7SEG
                     sta      ,x+
                     puls     a,pc ;(pul? pc=rts)

KEYHEX               lbsr     GETKEY

;   Fall-thru
HEXCON               pshs     x,b
                     ldx      #KEYCODE
                     ldb      #$FF
HEXCON1              incb     
                     cmpx     #SVNSEG
                     beq      TORESUME2
                     cmpa     ,x+
                     bne      HEXCON1
                     tfr      b,a
                     puls     b,x,pc ;(pul? pc=rts)

                     nop      
                     nop      
                     nop      

;   Fall-thru
L7SEG                asra     
                     asra     
                     asra     
                     asra     

;   Convert low-order 4 bits of A to 7seg
R7SEG                pshs     x
                     ldx      #SVNSEG
                     anda     #$0F
R71                  beq      R72
                     leax     $01,x
                     deca     
                     bra      R71

R72                  lda      ,x
                     puls     x,pc ;(pul? pc=rts)


;   Convert 7seg to hex
SVNHEX               pshs     x,b
                     ldx      #SVNSEG
                     tfr      a,b
                     clra     
SVNHEX1              cmpb     ,x+
                     beq      SVNHEX2
TORESUME2            beq      RESUME
                     inca     
                     bra      SVNHEX1

SVNHEX2              puls     b,x,pc ;(pul? pc=rts)


;   Prompt for address with 'M' and display memory contents
MEMDISP              clra     
                     sta      <dbuf+4
                     lda      #$6E
                     sta      <dbuf+5
                     bsr      BADDR
MEM0                 lda      ,x
                     pshs     a
                     bsr      L7SEG
                     sta      <dbuf+4
                     puls     a
                     bsr      R7SEG
                     sta      <dbuf+5

;   Check for I key
MEM1                 lbsr     GETKEY
                     cmpa     #$32
                     beq      MEM2
                     bsr      HEXCON
                     ldb      <dbuf+5
                     stb      <dbuf+4
                     bsr      R7SEG
                     sta      <dbuf+5
                     bra      MEM1


;   Store and increment
MEM2                 lda      <dbuf+4
                     bsr      SVNHEX
                     asla     
                     asla     
                     asla     
                     asla     
                     pshs     a
                     lda      <dbuf+5
                     bsr      SVNHEX
                     adda     ,s+
                     tfr      a,b
                     sta      ,x
                     lda      ,x+
                     pshs     b
                     cmpa     ,s+
                     bne      RESUME
                     tfr      x,d
                     bsr      L7SEG
                     sta      <dbuf+0 
                     tfr      x,d
                     bsr      R7SEG
                     sta      <dbuf+1 
                     tfr      b,a
                     bsr      L7SEG
                     sta      <dbuf+2 
                     tfr      b,a
                     lbsr     R7SEG
                     sta      <dbuf+3 
                     bra      MEM0


;   Reset Handler
RESET                lds      #usrsp
NMISR                sts      stackstart

;   Set up NMI to point here
                     ldx      #NMISR
                     stx      NMIVEC
RESUME               lds      #usrsp
                     lda      #$13
                     tfr      a,dp
                     bsr      CLEARDISP

;   Show '-' at left
                     lda      #$01
                     sta      <dbuf+0 
                     lbsr     GETKEY

;   If key is 'M'
                     cmpa     #$31
                     lbeq     MEMDISP

;   If key is 'R'
                     cmpa     #$30
                     beq      REGDISP

;   If key is 'CN'
                     cmpa     #$25
                     beq      CONT

;   If key is 'CL'
                     cmpa     #$05
                     lbeq     LOAD

;   If key is 'P'
                     cmpa     #$15
                     lbeq     SAVE

;   If key is 'G'
reset3               cmpa     #$35
                     bne      RESUME

;   Prompt and accept start address
go                   lda      #$7C
                     sta      <dbuf+5
                     lbsr     BADDR
                     ldy      <stackstart
                     stx      $0A,y
                     lda      #$80
                     ora      ,y
                     sta      ,y
CONT                 lds      <stackstart
                     rti      


;   Clear display
CLEARDISP            pshs     x,b,a
                     clra     
                     ldb      #$06
                     ldx      #dbuf 
CLEARDISP1           sta      ,x+
                     decb     
                     bne      CLEARDISP1
                     puls     a,b,x,pc ;(pul? pc=rts)


;   Characters for Reg Display  C b A b d H Y U P S 
REGDISPCHAR          FCB  $78, $6F, $75, $57, $67, $37, $76, $6B, $3D

REGCHAREND           lda      #$13
                     tfr      a,dp
                     sts      <stackstart
                     lds      #usrsp
REGDISP              bsr      CLEARDISP
                     ldx      <stackstart

;   Read characters from REGDISPCHAR
                     leay     -$1B,pc ; ($7DEB)
REGDISP1             lda      ,y+
                     sta      <dbuf+5
                     cmpy     #REGCHAREND 
                     bne      SHOWLEFT
                     ldx      #stackstart
SHOWLEFT             cmpy     #REGDISPCHAR2
                     bls      SHOWBYTE
                     lda      ,x
                     lbsr     L7SEG
                     sta      <dbuf+0 
                     lda      ,x+
                     lbsr     R7SEG
                     sta      <dbuf+1 
SHOWBYTE             lda      ,x
                     lbsr     L7SEG
                     sta      <dbuf+2 
                     lda      ,x+
                     lbsr     R7SEG
                     sta      <dbuf+3 
                     lbsr     GETKEY
                     cmpa     #$32
                     lbne     RESUME
                     cmpy     #REGCHAREND 
                     bne      REGDISP1
                     lbra     RESUME

RECVBIT              pshs     b,a
RETRY                ldb      #$06

;   Listen for a high signal on PORTB[7]
RECV1                lda      PORTB 
                     asla     
                     bcc      RECV1

;   Wait a moment
RECV2                decb     
                     bne      RECV2

;   Check the signal is still high
                     lda      PORTB 
                     asla     
                     bcc      RETRY

;   Time until the signal is low
                     ldb      #$24
RECV3                decb     
                     lda      PORTB 
                     asla     
                     bcs      RECV3

;   Show a bit pattern in the display
                     lda      #$EF
                     tstb     
                     bmi      RECV4
                     asra     
RECV4                sta      PORTA

;   Set CC with the result - positive for a 1
                     tstb     
                     puls     a,b,pc ;(pul? pc=rts)


;   Receive a byte and return in B
RECVBYTE             pshs     a

;   Wait for start bit
RBYTE1               bsr      RECVBIT
                     bpl      RBYTE1

;   Prepare to gather 8 bits
                     lda      #$08

;   Loop once per bit, LSB first
RBYTE3               bsr      RECVBIT
                     andcc    #$FE
                     bmi      RBYTE2
                     orcc     #$01
RBYTE2               rorb     
                     deca     
                     bne      RBYTE3
                     puls     a,pc ;(pul? pc=rts)


;   Set up I/O Ports
LOAD                 ldd      #allzeros
                     std      CTRLA
                     ldd      #$FF7F
                     std      PORTA
                     ldd      #$0404
                     std      CTRLA
                     ldd      #$FF04
                     std      PORTA
RELOAD               bsr      RECVBYTE

;   Wait for $53 = S
                     cmpb     #$53
                     bne      RELOAD
                     bsr      RECVBYTE

;   Code S1 begins a record
                     cmpb     #$31
                     beq      LOAD2

;   Code SJ means EOF
                     cmpb     #$4A
                     bne      RELOAD

;   Show F in display forever
MESSAGEF             lda      #$69
                     bra      ERRSTOP


;   Get count, initialise checksum
LOAD2                bsr      RECVBYTE
                     stb      <tmp2 
                     lda      <tmp2 

;   Get two-byte address, accumulating checksum in A
                     bsr      RECVBYTE
                     stb      <tmpX
                     pshs     b
                     adda     ,s+
                     dec      <tmp2 
                     bsr      RECVBYTE
                     stb      <$00F7
                     pshs     b
                     adda     ,s+
                     ldx      <tmpX
LDLOOP               dec      <tmp2 
                     beq      LDDONE
                     bsr      RECVBYTE
                     stb      ,x+
                     pshs     b
                     adda     ,s+
                     bra      LDLOOP


;   Verify checksum
LDDONE               bsr      RECVBYTE
                     pshs     b
                     adda     ,s+
                     beq      RELOAD
                     lda      #$78

;   Fall-thru
ERRSTOP              lbsr     CLEARDISP
                     sta      <dbuf+0 
DEAD                 lbsr     DISPRESH
                     bra      DEAD


;   Save to tape Prompt S and get start address
SAVE                 ldx      #$3D
                     stx      <dbuf+4
                     lbsr     BADDR
                     stx      <ptr

;   Prompt F and get finish address
                     lda      #$69
                     sta      <dbuf+5
                     lbsr     BADDR
                     stx      <addr

;   Set up output port
                     clra     
                     sta      CTRLB 
                     deca     
                     sta      PORTB 
                     lda      #$04
                     sta      CTRLB 
                     clra     
                     sta      PORTB 

;   Send two bytes of all ones
SAVE1                lda      #$FF
                     bsr      SEND
                     bsr      SEND

;   Send block start
SAVE4                bsr      SENDSTART
                     lda      #$FF
                     ldb      <ptr+1
                     pshs     b
                     suba     ,s+

;   Send length code and init checksum
SAVE2                anda     #$0F
                     adda     #$03
                     bsr      SEND
                     sta      <chksum 

;   Send (half of) address and add to checksum
                     ldx      #ptr
                     bsr      SENDBYTE
                     adda     <chksum 
                     sta      <chksum 

;   Send other half of address and add to checksum
                     leax     $01,x
                     bsr      SENDBYTE
                     adda     <chksum 
                     sta      <chksum 
                     ldx      <ptr

;   Send a data byte and add to checksum
SAVE3                bsr      SENDBYTE
                     adda     <chksum 
                     sta      <chksum 
                     cmpx     <addr
                     beq      ENDBLOCK
                     leax     $01,x
                     stx      <ptr
                     lda      <ptr+1
                     bita     #$0F
                     bne      SAVE3

;   Send checksum
ENDBLOCK             lda      <chksum 
                     nega     
                     bsr      SEND

;   Test if all finished
                     cmpx     <addr
                     beq      SENDEOF

;   Test if another full block to send
                     lda      <ptr
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


;   Send eof code 534a = 'SJ'
SENDEOF              lda      #$53
                     bsr      SEND
                     lda      #$4A
                     bsr      SEND
                     lbra     MESSAGEF


;   Send magic code 'S1'
SENDSTART            lda      #$53
                     bsr      SEND
                     lda      #$31
                     bra      SEND


;   Transmit byte from (X)
SENDBYTE             lda      ,x

;   Transmit byte from A
SEND                 pshs     b,a

;   Total of 10 bits to send, including start and stop
                     ldb      #$0A
                     stb      <count

;   Set up start bit
                     andcc    #$FE

;   Next bit in C flag - set B to delay count
NEXTBIT              ldb      #$90
                     bcc      SAVE8
                     ldb      #$24
SAVE8                pshs     b

;   Set bit in output port
                     ldb      #$40
                     stb      PORTB 
                     ldb      ,s
DELAY1               decb     
                     bne      DELAY1

;   Clear bit in output port
                     stb      PORTB 

;   Same delay loop again
                     puls     b
DELAY2               decb     
                     bne      DELAY2

;   Rotate a 1 bit into A, so stop bit will be a 1
                     orcc     #$01
                     rora     
                     dec      <count
                     bne      NEXTBIT
                     puls     a,b,pc ;(pul? pc=rts)

                     FCB  $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
                     FCB  $00

                     ldx      SWI2VEC
                     jmp      ,x

                     ldx      SWI3VEC
                     jmp      ,x

                     ldx      FIRQVEC
                     jmp      ,x

                     ldx      IRQVEC
                     jmp      ,x

                     ldx      NMIVEC
                     jmp      ,x

                     FCB  $00, $00, $00, $00, $00, $00, $00, $00, $00


;   Interrupt vectors
SWI3INTR             FCB  $7F, $D5

SWI2INTR             FCB  $7F, $D0

FIRQINTR             FCB  $7F, $DA

INTRQ                FCB  $7F, $DF

SWINTR               FCB  $7D, $F4

NMINTR               FCB  $7F, $E4

RESETBOOT            FCB  $7D, $89

