ExecuteCd:
    lea CdMsg,a1
    jsr CONPUTS(a6)
    rts
CdMsg:
    dc.b "CD command",13,10,0
    even