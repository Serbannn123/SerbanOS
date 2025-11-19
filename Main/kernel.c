#include "screen.h"
#include "idt.h"
#include "keyboard.h"
#include "io.h"
#include "commands.h"
#include "fs.h"
#include "util.h"

void test_ata_rw(void)
{
    unsigned char writebuf[512];
    unsigned char readbuf[512];

    // umplem bufferul de scris cu un pattern cunoscut
    for (int i = 0; i < 512; i++) {
        writebuf[i] = (unsigned char)(i & 0xFF);  // 0,1,2,3,...,255,0,1,...
    }

    printf("ATA: writing test sector...\n");
    if (!ata_write_sector(50, writebuf)) {   // LBA 50 -> zona safe
        printf("ATA: write failed!\n");
        return;
    }

    // curățăm readbuf ca să fim siguri că nu e deja ok din întâmplare
    for (int i = 0; i < 512; i++) {
        readbuf[i] = 0;
    }

    printf("ATA: reading test sector...\n");
    if (!ata_read_sector(50, readbuf)) {
        printf("ATA: read failed!\n");
        return;
    }

    // verificăm câțiva bytes
    printf("ATA: first 8 bytes read back: ");
    for (int i = 0; i < 8; i++) {
        printf("%x ", (unsigned int)readbuf[i]);
    }
    printf("\n");
}

// kernel.c
void kmain(void)
{
    // 3. Clear screen
    clear_screen();

    idt_install();
    printf("IDT installed\n");

    pic_remap();
    printf("PIC remapped\n");

    fs_init();

    keyboard_init();
    printf("Keyboard init done\n");

    asm volatile("sti");   // enable interrupts

    // Test tastatură
    unsigned char  buf[512];

    test_ata_rw();

    // buclă infinită
    for (;;) {
        printf("SerbanOS > ");
        readline(buf, sizeof(buf));
        eval_command(buf);
    }
}
