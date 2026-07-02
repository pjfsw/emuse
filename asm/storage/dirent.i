FILEATTR_DIR     EQU $01

    rsreset
DIRENT_NAME      rs.b 8
DIRENT_EXT       rs.b 3
DIRENT_ZERO      rs.b 1
DIRENT_ATTR      rs.b 1
DIRENT_PAD       rs.b 1
DIRENT_FSIZE     rs.l 1
DIRENT_SECTOR    rs.l 1
DIRENT_RESERVED  rs.b 8
DIRENT_STR_SIZE  rs.b 0
