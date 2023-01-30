;C:\Users\Dave\Documents\_6809\asm6809-2.12-w64\asm6809.exe -S -l tetrisguipoc.txt tetrisguipoc.asm -o tetrisguipoc.srec
    

                              ORG $0000           ; Tetris Custom Characters start at $F0
PORTB               EQU $D001
CTRLB               EQU $D003

CRTCADDRESS                   EQU  $D400
CRTCDATAREGISTER              EQU  CRTCADDRESS+1
VRAM                          EQU  $8000 ; Start of Video RAM
VRAMEND                       EQU  $BFFF ; End of Video RAM
CGRAM                         EQU  $C000 ; Start of character generator RAM
CGRAMEND                      EQU  $CFFF ; End of character generator RAM
CGROM                         EQU  $E000 ; Start of character generator ROM
ASCII0                        EQU  $30
ASCIISPACE                    EQU  $20
COLSPERROW                    EQU  $50
ROWSPERPAGE                   EQU  $1E
ROWSPERPAGEBCD                EQU  $30
BYTESPERCHAR                  EQU  $02
CLSDEFAULT                    EQU  $F020 ; White on Black, Space

DISPCLS                       EQU  $7FC2 ; This may change with monitor updates (current as of V9)
CLRSCREEN                     EQU  $F6DE  ; Monitor Clear Screen (current as of V9)

TABLECOLOUR                   EQU  $E0   ; Grey on black
TABLEROWBYTES                 EQU  $A0
VRAMTABLE                     EQU  VRAM + 4 * TABLEROWBYTES + 30 * BYTESPERCHAR ; Start table at row 5, 30 cells in (60 bytes)

SIDELEFT           EQU      $F0
SIDERIGHT          EQU      $F1
LOW                EQU      $F2
TOP                EQU      $F3
BACK               EQU      $F4
TOPLEFT            EQU      $F5
TOPRIGHT           EQU      $F6
LOWLEFT            EQU      $F7
LOWRIGHT           EQU      $F8
LEFT               EQU      $F9
RIGHT              EQU      $FA
OUTLINELEFT        EQU      $FB
OUTLINERIGHT       EQU      $FC
BLANK              EQU      $FD


COPYCHARGENROM                ldx      #CGRAM + 16 * 16 * 15 ; Each character is 16 Bytes, 16 Characters per block start at end of block 15
                              ldy      #TETRISSIDELEFT ; 1st Bitmap of character
COPYROM                       ldd      ,y++
                              std      ,x++
                              cmpx     #CGRAMEND
                              ble      COPYROM
                              
                              lda      #CLSDEFAULT ; Change the default CLS combination
                              sta      DISPCLS
                              jsr      CLRSCREEN
                              
                              
; Start at line 5 (+4xA0) Start at 30 Dec x 2, game area is 20 rows by 10 columns (20 characters)
PRINTTABLE                    ldx      #VRAMTABLE
                              ldb      #TABLECOLOUR
                              stb      ,X+
                              lda      #TOPLEFT
                              sta      ,X+
                              lda      #TOP ; Need 20 TOP Characters then TOPRIGHT
TOPLINE                       stb      ,X+
                              sta      ,X+
                              cmpx     #VRAMTABLE+42 ; 2 + 4x10
                              bne      TOPLINE
                              stb      ,X+
                              lda      #TOPRIGHT
                              sta      ,X+
                              ldx      #VRAMTABLE + TABLEROWBYTES
TABLESIDES                    lda      #SIDELEFT
                              stb      ,X+
                              sta      ,X+
                              leax     40,X
                              lda      #SIDERIGHT
                              stb      ,X+
                              sta      ,X+
                              leax     TABLEROWBYTES-44,X
                              cmpx     #VRAMTABLE + TABLEROWBYTES * 20
                              ble      TABLESIDES

                              ;swi
                              stb      ,X+
                              lda      #LOWLEFT
                              sta      ,X+
                              lda      #LOW ; Need 20 LOW Characters then LOWRIGHT
LOWLINE                       stb      ,X+
                              sta      ,X+
                              cmpx     #VRAMTABLE + TABLEROWBYTES * 21 + 40  ; 2 + 4x10
                              
                              ble      LOWLINE
                              stb      ,X+
                              lda      #LOWRIGHT
                              sta      ,X+

                              ;swi
                              ; Output Ghost tetronimo at the bottom
                              lda      #$13
                              sta      CURROW
                              lda      #$04
                              sta      CURCOL
                             
                              jsr      ROWCOLTOX  ; Get address of colour byte in X
                              lda      #$F0
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              
                              clra                      ; Init PIA for checking Display Enable
                              sta      CTRLB            ; Set Port B to data direction register
                              lda      #$0F              ; set bits 0-3 for output, 4-7 for input
                              sta      PORTB            ; Set Port B to output for bits 0-3
                              lda      #$04              ; Set control register b2=1 Output Register
                              sta      CTRLB            ; Set Port B control register for Port Output, will be keypad row

STARTDROP                     clr      CURROW
                              lda      #$04
                              sta      CURCOL
                              jsr      CLEARTET
DROPLOOP                      inc      CURROW
                              jsr      DISPTET
                              
                              ldy      #$4000
DELAYLOOP                     leay     -1,y
                              BNE      DELAYLOOP
                              jsr      CLEARTET
                              lda      #$13
                              cmpa     CURROW
                              bne      DROPLOOP

                              ; Output Ghost tetronimo at the bottom
                              lda      #$13
                              sta      CURROW
                              lda      #$04
                              sta      CURCOL
                             
                              jsr      ROWCOLTOX  ; Get address of colour byte in X
                              lda      #$F0
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              ldb      #OUTLINELEFT
                              std      ,X++
                              ldb      #OUTLINERIGHT
                              std      ,X++
                              
                              jmp      STARTDROP
                            
                              
END                           swi


WAITFORVSDE         pshs     a
TESTDE              lda      #$80          ; Mask PB7 on Port B of PIA
                    bita     PORTB        ; Test PB7 for Display Enable, when 1 then screen blanked
                    beq      TESTDE
                    nop
                    nop
                    nop                    
                    bita     PORTB        ; Test PB7 again
                    beq      TESTDE       ; If 0 then only HS or end of VS, so continue to wait for DE=1 for more than 6ms
                    puls     a,pc

                              
DISPTET                       jsr      ROWCOLTOX  ; Get address of colour byte in X using CURROW and CURCOL
                              jsr      WAITFORVSDE
                              lda      #$70
                              ldb      #LEFT
                              std      ,X++
                              ldb      #RIGHT
                              std      ,X++
                              ldb      #LEFT
                              std      ,X++
                              ldb      #RIGHT
                              std      ,X++
                              ldb      #LEFT
                              std      ,X++
                              ldb      #RIGHT
                              std      ,X++
                              ldb      #LEFT
                              std      ,X++
                              ldb      #RIGHT
                              std      ,X++
                              rts

                              ; swi
                               


                              swi

CLEARTET                      jsr      ROWCOLTOX  ; Get address of colour byte in X using CURROW and CURCOL
                              jsr      WAITFORVSDE
                              lda      #$F0
                              ldb      #BLANK
                              std      ,X++
                              std      ,X++
                              std      ,X++
                              std      ,X++
                              std      ,X++
                              std      ,X++
                              std      ,X++
                              std      ,X++
                              rts
                              
; This lookup table can be used to calculate address for (CURCOL, CURROW), Use column in A ROL 1 bit (x2) 
ROWCOLTOX                     LDX      #COLADDRESS
                              LDA      CURROW
                              ROLA
                              LEAX     [A,X]
                              LDB      CURCOL
                              ROLB
                              LEAX      B,X
                              RTS
                              
; Pre-caclculated index table for start of each row to avoid multiplication during execution
COLADDRESS                    FDB      #VRAMTABLE + 2 + TABLEROWBYTES
COLADDRESS1                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 2
COLADDRESS2                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 3
COLADDRESS3                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 4
COLADDRESS4                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 5
COLADDRESS5                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 6
COLADDRESS6                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 7
COLADDRESS7                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 8
COLADDRESS8                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 9
COLADDRESS9                   FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 10
COLADDRESS10                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 11
COLADDRESS11                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 12
COLADDRESS12                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 13
COLADDRESS13                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 14
COLADDRESS14                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 15
COLADDRESS15                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 16
COLADDRESS16                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 17
COLADDRESS17                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 18
COLADDRESS18                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 19
COLADDRESS19                  FDB      #VRAMTABLE + 2 + TABLEROWBYTES * 20


; Character Generator redefined Tetris characters from $F0
TETRISSIDELEFT     FCB      $1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E,$1E         ; Side vertical bar 4 wide, 1 in left
TETRISSIDERIGHT    FCB      $78,$78,$78,$78,$78,$78,$78,$78,$78,$78,$78,$78,$78,$78,$78,$78         ; Side vertical bar 4 wide, 1 in right
TETRISLOW          FCB      $00,$FF,$FF,$FF,$FF,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00         ; Low bar 4 deep 1 down
TETRISTOP          FCB      $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$FF,$FF,$FF,$FF,$00         ; Top bar 4 deep 1 up
TETRISBACK         FCB      $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF         ; Background
TETRISTOPLEFT      FCB      $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$07,$0F,$1F,$1F,$1E         ; Top Right Corner
TETRISTOPRIGHT     FCB      $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$E0,$F0,$F8,$F8,$78         ; Top Left Corner
TETRISLOWLEFT      FCB      $1E,$1F,$1F,$0F,$07,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00         ; Lower Left Corner
TETRISLOWRIGHT     FCB      $78,$F8,$F8,$F0,$E0,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00         ; Lower Right Corner
TETRISLEFT         FCB      $00,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$7F,$00         ; Tetris Square Left
TETRISRIGHT        FCB      $00,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$FE,$00         ; Tetris Square Right
TETRISOUTLINELEFT  FCB      $00,$7F,$40,$40,$40,$40,$40,$40,$40,$40,$40,$40,$40,$40,$7F,$00         ; Tetris Square Outline Left
TETRISOUTLINERIGHT FCB      $00,$FE,$02,$02,$02,$02,$02,$02,$02,$02,$02,$02,$02,$02,$FE,$00         ; Tetris Square Outline Right
TETRISBLANK0       FCB      $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00         ; Blank
TETRISBLANK1       FCB      $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00         ; Blank
TETRISBLANK2       FCB      $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00         ; Blank

CURROW            RMB      1;
CURCOL            RMB      1;
