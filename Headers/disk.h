#ifndef DISK_H_
#define DISK_H_

#include <stdint.h>

int ata_read_sector(uint32_t lba, void *buffer);
int ata_write_sector(uint32_t lba, const void *buffer);

#endif
