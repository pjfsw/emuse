/*
 * hunkreloc.c
 *
 * Minimal Amiga HUNK executable loader/dumper.
 *
 * Supported:
 *   HUNK_HEADER
 *   HUNK_CODE
 *   HUNK_DATA
 *   HUNK_BSS
 *   HUNK_RELOC32SHORT (0x3f7)
 *   HUNK_END
 *
 * Usage:
 *   ./hunkreloc program.exe 0x20000
 *
 * Build:
 *   cc -std=c11 -Wall -Wextra -Wpedantic -O2 \
 *      -o hunkreloc hunkreloc.c
 */

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    HUNK_UNIT         = 0x03E7,
    HUNK_NAME         = 0x03E8,
    HUNK_CODE         = 0x03E9,
    HUNK_DATA         = 0x03EA,
    HUNK_BSS          = 0x03EB,
    HUNK_RELOC32      = 0x03EC,
    HUNK_RELOC16      = 0x03ED,
    HUNK_RELOC8       = 0x03EE,
    HUNK_EXT          = 0x03EF,
    HUNK_SYMBOL       = 0x03F0,
    HUNK_DEBUG        = 0x03F1,
    HUNK_END          = 0x03F2,
    HUNK_HEADER       = 0x03F3,
    HUNK_OVERLAY      = 0x03F5,
    HUNK_BREAK        = 0x03F6,
    HUNK_RELOC32SHORT = 0x03F7
};

#define HUNK_TYPE_MASK 0x3FFFFFFFu
#define HUNK_SIZE_MASK 0x3FFFFFFFu

typedef enum
{
    MEM_HUNK_UNKNOWN,
    MEM_HUNK_CODE,
    MEM_HUNK_DATA,
    MEM_HUNK_BSS
} HunkType;

typedef struct
{
    HunkType type;

    uint32_t declared_longs;
    uint32_t allocated_size;
    uint32_t content_size;

    uint32_t load_address;
    uint8_t *data;
} Hunk;

typedef struct
{
    uint8_t *data;
    size_t size;
    size_t position;
} Reader;

static void fatal(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void fatal_at(const Reader *reader, const char *message)
{
    fprintf(stderr,
            "error at file offset 0x%zx: %s\n",
            reader->position,
            message);
    exit(EXIT_FAILURE);
}

static uint16_t read_be16(Reader *reader)
{
    if (reader->position + 2 > reader->size)
        fatal_at(reader, "unexpected end of file reading word");

    uint16_t value =
        ((uint16_t)reader->data[reader->position] << 8) |
        ((uint16_t)reader->data[reader->position + 1]);

    reader->position += 2;
    return value;
}

static uint32_t read_be32(Reader *reader)
{
    if (reader->position + 4 > reader->size)
        fatal_at(reader, "unexpected end of file reading longword");

    uint32_t value =
        ((uint32_t)reader->data[reader->position] << 24) |
        ((uint32_t)reader->data[reader->position + 1] << 16) |
        ((uint32_t)reader->data[reader->position + 2] << 8) |
        ((uint32_t)reader->data[reader->position + 3]);

    reader->position += 4;
    return value;
}

static uint32_t load_be32(const uint8_t *data)
{
    return
        ((uint32_t)data[0] << 24) |
        ((uint32_t)data[1] << 16) |
        ((uint32_t)data[2] << 8) |
        ((uint32_t)data[3]);
}

static void store_be32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value >> 24);
    data[1] = (uint8_t)(value >> 16);
    data[2] = (uint8_t)(value >> 8);
    data[3] = (uint8_t)value;
}

static uint8_t *read_entire_file(const char *filename, size_t *size_out)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "cannot open '%s': %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fseek(file, 0, SEEK_END) != 0)
        fatal("failed to seek to end of input file");

    long length = ftell(file);
    if (length < 0)
        fatal("failed to determine input file size");

    if (fseek(file, 0, SEEK_SET) != 0)
        fatal("failed to rewind input file");

    uint8_t *data = malloc((size_t)length);
    if (data == NULL && length != 0)
        fatal("out of memory reading input file");

    if (fread(data, 1, (size_t)length, file) != (size_t)length)
        fatal("failed to read input file");

    fclose(file);

    *size_out = (size_t)length;
    return data;
}

static uint32_t parse_u32(const char *text)
{
    char *end = NULL;

    errno = 0;
    unsigned long value = strtoul(text, &end, 0);

    if (errno != 0 || end == text || *end != '\0' || value > UINT32_MAX)
    {
        fprintf(stderr, "invalid 32-bit address: '%s'\n", text);
        exit(EXIT_FAILURE);
    }

    return (uint32_t)value;
}

static const char *hunk_type_name(HunkType type)
{
    switch (type)
    {
        case MEM_HUNK_CODE:
            return "CODE";

        case MEM_HUNK_DATA:
            return "DATA";

        case MEM_HUNK_BSS:
            return "BSS";

        default:
            return "UNKNOWN";
    }
}

static void skip_resident_names(Reader *reader)
{
    for (;;)
    {
        uint32_t length_longs = read_be32(reader);

        if (length_longs == 0)
            return;

        uint64_t bytes = (uint64_t)length_longs * 4;

        if (bytes > SIZE_MAX ||
            reader->position + (size_t)bytes > reader->size)
        {
            fatal_at(reader, "invalid resident-name list");
        }

        reader->position += (size_t)bytes;
    }
}

static void read_hunk_contents(Reader *reader,
                               Hunk *hunk,
                               HunkType type)
{
    uint32_t content_longs = read_be32(reader) & HUNK_SIZE_MASK;
    uint64_t content_bytes64 = (uint64_t)content_longs * 4;

    if (content_bytes64 > UINT32_MAX)
        fatal_at(reader, "hunk content is too large");

    uint32_t content_bytes = (uint32_t)content_bytes64;

    if (content_bytes > hunk->allocated_size)
        fatal_at(reader, "hunk contents exceed header allocation size");

    hunk->type = type;
    hunk->content_size = content_bytes;

    if (type == MEM_HUNK_BSS)
    {
        /*
         * HUNK_BSS contains no payload. calloc() has already cleared it.
         */
        return;
    }

    if (reader->position + content_bytes > reader->size)
        fatal_at(reader, "truncated CODE or DATA hunk");

    memcpy(hunk->data,
           reader->data + reader->position,
           content_bytes);

    reader->position += content_bytes;
}

static void apply_reloc32short(Reader *reader,
                               Hunk *hunks,
                               uint32_t hunk_count,
                               uint32_t current_hunk)
{
    Hunk *source = &hunks[current_hunk];

    printf("\nRelocations for hunk %" PRIu32
           " (%s at $%08" PRIX32 "):\n",
           current_hunk,
           hunk_type_name(source->type),
           source->load_address);

    size_t relocation_start = reader->position;

    for (;;)
    {
        uint16_t count = read_be16(reader);

        if (count == 0)
            break;

        uint16_t target_index = read_be16(reader);

        if (target_index >= hunk_count)
            fatal_at(reader, "relocation target hunk is out of range");

        Hunk *target = &hunks[target_index];

        for (uint16_t i = 0; i < count; ++i)
        {
            uint16_t offset = read_be16(reader);

            if ((uint32_t)offset + 4 > source->allocated_size)
                fatal_at(reader, "relocation offset is outside source hunk");

            uint8_t *location = source->data + offset;

            uint32_t old_value = load_be32(location);
            uint32_t new_value = old_value + target->load_address;

            store_be32(location, new_value);

            printf("  hunk %" PRIu32 " + $%04" PRIX16
                   "  [$%08" PRIX32 "]"
                   "  -> target hunk %" PRIu16
                   " base $%08" PRIX32
                   "  : $%08" PRIX32 " -> $%08" PRIX32 "\n",
                   current_hunk,
                   offset,
                   source->load_address + offset,
                   target_index,
                   target->load_address,
                   old_value,
                   new_value);
        }
    }

    /*
     * HUNK_RELOC32SHORT is padded to a longword boundary.
     *
     * Alignment is relative to the file itself, which is equivalent to
     * rounding the current file position up to a multiple of four.
     */
    if ((reader->position & 2u) != 0)
    {
        uint16_t padding = read_be16(reader);

        if (padding != 0)
        {
            fprintf(stderr,
                    "warning: nonzero relocation padding word $%04" PRIX16
                    " at file offset 0x%zx\n",
                    padding,
                    reader->position - 2);
        }
    }

    printf("  relocation block bytes: %zu\n",
           reader->position - relocation_start);
}

static void print_hunk_table(const Hunk *hunks, uint32_t hunk_count)
{
    printf("\nLoaded hunks:\n");

    for (uint32_t i = 0; i < hunk_count; ++i)
    {
        const Hunk *hunk = &hunks[i];

        printf("  hunk %" PRIu32
               ": %-4s"
               " address=$%08" PRIX32
               " size=$%08" PRIX32
               " content=$%08" PRIX32 "\n",
               i,
               hunk_type_name(hunk->type),
               hunk->load_address,
               hunk->allocated_size,
               hunk->content_size);
    }
}

static void print_hex_dump(const Hunk *hunks,
                           uint32_t hunk_count,
                           uint32_t load_base)
{
    uint64_t total_size64 = 0;

    for (uint32_t i = 0; i < hunk_count; ++i)
        total_size64 += hunks[i].allocated_size;

    if (total_size64 > SIZE_MAX)
        fatal("loaded image is too large to dump");

    size_t total_size = (size_t)total_size64;
    uint8_t *image = calloc(total_size, 1);

    if (image == NULL && total_size != 0)
        fatal("out of memory constructing dump image");

    size_t position = 0;

    for (uint32_t i = 0; i < hunk_count; ++i)
    {
        memcpy(image + position,
               hunks[i].data,
               hunks[i].allocated_size);

        position += hunks[i].allocated_size;
    }

    printf("\nExpected memory dump:\n");

    for (size_t offset = 0; offset < total_size; offset += 16)
    {
        size_t line_size = total_size - offset;

        if (line_size > 16)
            line_size = 16;

        printf("%08" PRIX32 ": ", load_base + (uint32_t)offset);

        for (size_t i = 0; i < 16; ++i)
        {
            if (i < line_size)
                printf("%02X ", image[offset + i]);
            else
                printf("   ");
        }

        printf(" |");

        for (size_t i = 0; i < line_size; ++i)
        {
            uint8_t c = image[offset + i];
            putchar(c >= 32 && c <= 126 ? (char)c : '.');
        }

        for (size_t i = line_size; i < 16; ++i)
            putchar(' ');

        printf("|\n");
    }

    free(image);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr,
                "usage: %s <hunk-executable> <load-address>\n"
                "example: %s blinker.exe 0x20000\n",
                argv[0],
                argv[0]);
        return EXIT_FAILURE;
    }

    size_t file_size = 0;
    uint8_t *file_data = read_entire_file(argv[1], &file_size);
    uint32_t load_base = parse_u32(argv[2]);

    Reader reader = {
        .data = file_data,
        .size = file_size,
        .position = 0
    };

    uint32_t type = read_be32(&reader) & HUNK_TYPE_MASK;

    if (type != HUNK_HEADER)
        fatal_at(&reader, "file does not begin with HUNK_HEADER");

    skip_resident_names(&reader);

    uint32_t table_size = read_be32(&reader);
    uint32_t first_hunk = read_be32(&reader);
    uint32_t last_hunk = read_be32(&reader);

    if (last_hunk < first_hunk)
        fatal_at(&reader, "invalid first/last hunk range");

    uint32_t hunk_count = last_hunk - first_hunk + 1;

    if (table_size < hunk_count)
        fatal_at(&reader, "hunk size table is smaller than hunk range");

    /*
     * Executables normally use first_hunk == 0. Supporting a nonzero
     * first hunk would require translating file hunk numbers.
     */
    if (first_hunk != 0)
        fatal_at(&reader, "nonzero first hunk is not supported");

    Hunk *hunks = calloc(hunk_count, sizeof(*hunks));

    if (hunks == NULL)
        fatal("out of memory allocating hunk table");

    uint32_t next_address = load_base;

    for (uint32_t i = 0; i < table_size; ++i)
    {
        uint32_t size_word = read_be32(&reader);
        uint32_t size_longs = size_word & HUNK_SIZE_MASK;

        if (i >= hunk_count)
            continue;

        uint64_t size_bytes64 = (uint64_t)size_longs * 4;

        if (size_bytes64 > UINT32_MAX)
            fatal_at(&reader, "hunk allocation is too large");

        uint32_t size_bytes = (uint32_t)size_bytes64;

        if (UINT32_MAX - next_address < size_bytes)
            fatal_at(&reader, "loaded address range overflows 32 bits");

        hunks[i].declared_longs = size_longs;
        hunks[i].allocated_size = size_bytes;
        hunks[i].load_address = next_address;
        hunks[i].type = MEM_HUNK_UNKNOWN;
        hunks[i].data = calloc(size_bytes, 1);

        if (hunks[i].data == NULL && size_bytes != 0)
            fatal("out of memory allocating hunk");

        next_address += size_bytes;
    }

    uint32_t current_hunk = 0;
    bool have_current_hunk = false;

    while (reader.position < reader.size)
    {
        size_t type_offset = reader.position;
        uint32_t raw_type = read_be32(&reader);
        uint32_t hunk_type = raw_type & HUNK_TYPE_MASK;

        switch (hunk_type)
        {
            case HUNK_CODE:
                if (current_hunk >= hunk_count)
                    fatal_at(&reader, "too many loadable hunks");

                read_hunk_contents(&reader,
                                   &hunks[current_hunk],
                                   MEM_HUNK_CODE);

                printf("Read HUNK_CODE for hunk %" PRIu32 "\n",
                       current_hunk);

                have_current_hunk = true;
                break;

            case HUNK_DATA:
                if (current_hunk >= hunk_count)
                    fatal_at(&reader, "too many loadable hunks");

                read_hunk_contents(&reader,
                                   &hunks[current_hunk],
                                   MEM_HUNK_DATA);

                printf("Read HUNK_DATA for hunk %" PRIu32 "\n",
                       current_hunk);

                have_current_hunk = true;
                break;

            case HUNK_BSS:
                if (current_hunk >= hunk_count)
                    fatal_at(&reader, "too many loadable hunks");

                read_hunk_contents(&reader,
                                   &hunks[current_hunk],
                                   MEM_HUNK_BSS);

                printf("Read HUNK_BSS for hunk %" PRIu32 "\n",
                       current_hunk);

                have_current_hunk = true;
                break;

            case HUNK_RELOC32SHORT:
                if (!have_current_hunk || current_hunk >= hunk_count)
                    fatal_at(&reader,
                             "HUNK_RELOC32SHORT without a current hunk");

                apply_reloc32short(&reader,
                                   hunks,
                                   hunk_count,
                                   current_hunk);
                break;

            case HUNK_END:
                if (!have_current_hunk)
                    fatal_at(&reader, "HUNK_END without a current hunk");

                ++current_hunk;
                have_current_hunk = false;
                break;

            default:
                fprintf(stderr,
                        "unsupported hunk type $%08" PRIX32
                        " at file offset 0x%zx\n",
                        hunk_type,
                        type_offset);
                return EXIT_FAILURE;
        }
    }

    if (current_hunk != hunk_count)
    {
        fprintf(stderr,
                "warning: parsed %" PRIu32 " of %" PRIu32 " hunks\n",
                current_hunk,
                hunk_count);
    }

    print_hunk_table(hunks, hunk_count);
    print_hex_dump(hunks, hunk_count, load_base);

    for (uint32_t i = 0; i < hunk_count; ++i)
        free(hunks[i].data);

    free(hunks);
    free(file_data);

    return EXIT_SUCCESS;
}