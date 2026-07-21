#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_le32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

int inject_file(uint8_t *sd_image, const char *filename) {
    const size_t data_offset = 0x00383000;
    const size_t dir_offset = 0x0013F300;
    const size_t cluster_size = 0x10000; /* 128 sectors × 512 bytes */

    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror(filename);
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }

    long length = ftell(file);

    if (length < 0) {
        fclose(file);
        return -1;
    }

    rewind(file);

    /*
     * Clearing is not required for FAT correctness because the file size
     * controls how many bytes are read, but it makes dumps less confusing.
     */
    memset(sd_image + data_offset, 0, cluster_size);

    if (fread(sd_image + data_offset, 1, (size_t)length, file) != (size_t)length) {
        fprintf(stderr, "Failed to read complete input file\n");
        fclose(file);
        return -1;
    }

    fclose(file);

    /* FAT directory entry file-size field: bytes 28..31, little-endian. */
    write_le32(sd_image + dir_offset + 28, (uint32_t)length);

    printf("Injected %s: %ld bytes at image offset 0x%08zX\n", filename, length, data_offset);

    return 0;
}