    include hardware.i

Uploader:
    lea Uploader_StartMsg(pc),a1
    bsr ConPuts
    rts
Uploader_StartMsg:
    dc.b 13,10,"Paste S-record data into terminal now.",13,10
    dc.b "Press <ESC> to abort: ",0
    even

    include console.asm