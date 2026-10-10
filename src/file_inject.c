#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdint.h>
#include <stddef.h>

static uint16_t read_le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_le32(const uint8_t *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint64_t fat16_cluster_offset(const uint8_t *image,
                                     uint16_t cluster)
{
    /* MBR: first partition starts at offset 446 */
    const uint8_t *partition = image + 446;

    uint32_t partition_lba = read_le32(partition + 8);

    /* FAT16 boot sector */
    const uint8_t *bpb = image + (uint64_t)partition_lba * 512;

    uint16_t bytes_per_sector = read_le16(bpb + 11);
    uint8_t sectors_per_cluster = bpb[13];
    uint16_t reserved_sectors = read_le16(bpb + 14);
    uint8_t number_of_fats = bpb[16];
    uint16_t root_entries = read_le16(bpb + 17);
    uint16_t sectors_per_fat = read_le16(bpb + 22);

    /* Root directory occupies a fixed number of sectors */
    uint32_t root_sectors =
        ((uint32_t)root_entries * 32 + bytes_per_sector - 1)
        / bytes_per_sector;

    /* First data sector, relative to partition */
    uint32_t data_sector =
        reserved_sectors +
        number_of_fats * sectors_per_fat +
        root_sectors;

    /* FAT cluster numbers start at 2 */
    uint64_t sector =
        (uint64_t)partition_lba +
        data_sector +
        (uint64_t)(cluster - 2) * sectors_per_cluster;

    return sector * bytes_per_sector;
}

static int make_fat_name(const char *filename, char out[11])
{
    const char *base = strrchr(filename, '/');
    base = base ? base + 1 : filename;

    memset(out, ' ', 11);

    size_t i = 0;
    while (*base && *base != '.') {
        if (i >= 8) return -1;
        out[i++] = toupper((unsigned char)*base++);
    }

    if (*base == '.') {
        base++;
        i = 8;

        while (*base) {
            if (i >= 11) return -1;
            out[i++] = toupper((unsigned char)*base++);
        }
    }

    return 0;
}

static int64_t find_file_entry(const uint8_t *image,
                               size_t image_size,
                               const char *filename)
{
    char fat_name[11];

    if (make_fat_name(filename, fat_name) != 0)
        return -1;

    for (size_t offset = 0; offset <= image_size - 32; offset += 32) {
        const uint8_t *entry = image + offset;

        if (memcmp(entry, fat_name, 11) != 0)
            continue;

        /* Reject deleted entries, LFN entries and directories */
        if (entry[0] == 0xE5 ||
            (entry[11] & 0x0F) == 0x0F ||
            (entry[11] & 0x18))
            continue;

        return (int64_t)offset;
    }

    return -1;
}

static void write_le32(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16);
    p[3] = (uint8_t)(value >> 24);
}

int inject_file(uint8_t *sd_image, const char *filename) {
    const size_t cluster_size = 0x10000; /* 128 sectors × 512 bytes */
    int64_t dir_offset = find_file_entry(sd_image, 32*1024*1024, filename);
    if (dir_offset < 0) {
        return -1;
    }
    uint16_t cluster = read_le16(sd_image + dir_offset + 26);
    uint64_t data_offset = fat16_cluster_offset(sd_image, cluster);

    printf("Found %s dir offset at %08x, data offset at %08x\n", filename, dir_offset, data_offset);

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