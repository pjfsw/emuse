BootLoader:
    lea BootLoader_Loading(pc),a1
    bsr ConPuts
    rts
BootLoader_Loading:
    dc.b 13,10,13,10,"Loading DOS",13,10,0
    even    

    include console.asm