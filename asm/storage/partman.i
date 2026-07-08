    rsreset
PM_DEVICE    rs.l 1
PM_INDEX     rs.w 1
PM_TYPE      rs.b 1
PM_FLAGS     rs.b 1
PM_PSTART    rs.l 1
PM_PSIZE     rs.l 1
PM_READ      rs.l 1
PM_WRITE     rs.l 1
PM_RESERVED  rs.l 2
PM_SIZEOF    rs.b 0

PM_SIZE_SHIFT       equ 5   ; 32 bytes
PM_PART_MAX_COUNT   equ 4
PM_PART_LIST_SIZE   equ PM_SIZEOF*PM_PART_MAX_COUNT