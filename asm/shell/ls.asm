ExecuteLs:
    lea LsMsg,a1
    jsr CONPUTS(a6)
    rts
LsMsg:
    dc.b "Ls command",13,10,0
    even