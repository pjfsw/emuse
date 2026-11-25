;____________________________________________________________
;
; Kernel entry point
;
; Must be the first object in the link, so that _start is at
; the beginning of the first code hunk (where the loader jumps).
;
; Input:  A1: Pointer to process startup block (PSB)
; Output: D0: Return code (kernel does not normally return)
;____________________________________________________________
    section "CODE",code

    xdef _start
    xref _main

_start:
    jmp _main
