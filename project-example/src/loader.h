#pragma once

#include <libcd.h>

#define CD_SECTOR_SIZE 2048
#define len_to_sectors(size) ((size + CD_SECTOR_SIZE -1) / CD_SECTOR_SIZE)

extern u_long load_buffer[(1 << 16)/sizeof(u_long)];

u_long load_file(char * filename);
