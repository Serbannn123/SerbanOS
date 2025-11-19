// disk.c
#include "disk.h"
#include "io.h"
#include "screen.h"   // pentru printf, dacă vrei mesaje

#define ATA_PRIMARY_IO       0x1F0
#define ATA_PRIMARY_CTRL     0x3F6

#define ATA_REG_DATA         (ATA_PRIMARY_IO + 0)
#define ATA_REG_ERROR        (ATA_PRIMARY_IO + 1)
#define ATA_REG_SECCOUNT0    (ATA_PRIMARY_IO + 2)
#define ATA_REG_LBA0         (ATA_PRIMARY_IO + 3)
#define ATA_REG_LBA1         (ATA_PRIMARY_IO + 4)
#define ATA_REG_LBA2         (ATA_PRIMARY_IO + 5)
#define ATA_REG_HDDEVSEL     (ATA_PRIMARY_IO + 6)
#define ATA_REG_COMMAND      (ATA_PRIMARY_IO + 7)
#define ATA_REG_STATUS       (ATA_PRIMARY_IO + 7)

#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF   0x20
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

static unsigned char ata_status(void)
{
    return inb(ATA_REG_STATUS);
}

// așteaptă până când BSY=0 sau expiră
static int ata_wait_not_busy(void)
{
    int timeout = 1000000;
    while (timeout-- > 0) {
        unsigned char s = ata_status();
        if (!(s & ATA_SR_BSY))
            return 1;
    }
    return 0;   // timeout
}

// așteaptă DRQ=1; dacă apare ERR/DF sau expiră -> eșec
static int ata_wait_drq(void)
{
    int timeout = 1000000;
    while (timeout-- > 0) {
        unsigned char s = ata_status();

        if (s & (ATA_SR_ERR | ATA_SR_DF))
            return 0;       // eroare la comandă

        if (s & ATA_SR_DRQ)
            return 1;       // date gata
    }
    return 0;               // timeout
}

int ata_read_sector(uint32_t lba, void *buffer)
{
    unsigned short *buf = (unsigned short *)buffer;

    if (!ata_wait_not_busy()) {
        printf("ATA: timeout waiting for BSY=0\n");
        return 0;
    }

    // selectăm master + LBA
    outb(ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    // 1 sector
    outb(ATA_REG_SECCOUNT0, 1);

    // LBA 0..23
    outb(ATA_REG_LBA0, (unsigned char)(lba & 0xFF));
    outb(ATA_REG_LBA1, (unsigned char)((lba >> 8) & 0xFF));
    outb(ATA_REG_LBA2, (unsigned char)((lba >> 16) & 0xFF));

    // comanda READ SECTORS
    outb(ATA_REG_COMMAND, 0x20);

    if (!ata_wait_drq()) {
        printf("ATA: DRQ not set (ERR/DF or timeout) for LBA %d\n", (int)lba);
        return 0;
    }

    // citim 512 bytes = 256 words
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(ATA_REG_DATA);
    }

    return 1;
}
