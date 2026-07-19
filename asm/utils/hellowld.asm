    incdir "../lib"
    include "rootlib.i"
    
    move.l $4.w,a6
    lea msg(pc),a1
    jsr CONPUTS(a6)
    moveq #0,d0
    rts
msg:
    dc.b "Hello world!",13,10,0
