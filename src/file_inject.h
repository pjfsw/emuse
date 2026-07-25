#pragma once

#include <stdint.h>

int inject_file(uint8_t *sd_image, const char *filename, uint32_t data_offset, uint32_t dir_offset);