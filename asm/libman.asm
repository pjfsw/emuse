LMInit:
    rts
    
;____________________________________________________________
;
; Open Library
; 
; Input:  D0 library identifier
;         D1 version
; Ouptut: D0 pointer to library base or -1 if unsuccessful
;____________________________________________________________
LMOpenLibrary:
    cmp.l #DOS_LIB_ID,d0
    bne.s .fail
    cmp.l #1,d1
    bhi.s .fail
    move.l #JT_DOS_LIB_BASE,d0
    rts
.fail:
    moveq #-1,d0
    rts

;____________________________________________________________
;
; Close library 
; Input:  D0: 32-bit Library identifier
; Output: D0: 0=OK
;____________________________________________________________
LMCloseLibrary:
    moveq #0,d0
    rts

LMAddLibrary:
    moveq #-1,d0
    rts
    