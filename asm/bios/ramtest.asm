RAM_TEST_START_ADDRESS equ $000400
RAM_TEST_END_ADDRESS   equ $020000

RamTest:
    lea RAM_TEST_START_ADDRESS,a2
    move.w #$700,d7
RamTestNextPage:
    bsr RamTestPrintProgress
    bsr RamTestOnePage    
    bne.s RamTestFail
    move.w #$800,d7
    cmp.l #RAM_TEST_END_ADDRESS,a2
    bne.s RamTestNextPage   
RamTestDone:    
    lea RamOkMessage(pc),a1
    bsr ConPuts
    rts

RamTestFail:    
    lea RamNokMessage(pc),a1
    bsr ConPuts
    rts

RamTestPrintProgress:
    move.b #'.',d0
    bsr ConPutc
    rts
    
RamTestOnePage:
    inline
.1    
    moveq #0,d2
    bsr.s TestAscending    
    bne.s RamTestOnePageFail
    moveq #$ff,d2
    bsr.s TestAscending
    bne.s RamTestOnePageFail
    lea.l 4(a2),a2
    subq.w #1,d7
    bne.s .1
RamTestOnePageFail:
    rts
    
TestAscending:
    move.b d2,(a2)
    move.b d2,1(a2)
    move.b d2,2(a2)
    move.b d2,3(a2)
    move.b (a2),d0
    cmp.b d0,d2
    bne.s RamTestFail
    move.b 1(a2),d0
    cmp.b d0,d2
    bne.s RamTestFail
    move.b 2(a2),d0
    cmp.b d0,d2
    bne.s RamTestFail
    move.b 3(a2),d0
    cmp.b d0,d2
TestAscendingFail:    
    rts    
RamOkMessage:
    dc.b "OK!",13,10,0
RamNokMessage:
    dc.b "Failed!",13,10,0    

    include console.asm
