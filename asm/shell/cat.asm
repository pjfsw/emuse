ExecuteCat:
    lea CatMsg,a1
    jsr CONPUTS(a6)
    rts
CatMsg:
    dc.b "Cat command",13,10,0
    even