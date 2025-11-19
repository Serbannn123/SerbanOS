#include "screen.h"
#include "idt.h"
#include "keyboard.h"
#include "io.h"
#include "commands.h"
#include "fs.h"
#include "util.h"

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
    unsigned char  buf[128];

    ata_read_sector(0, buf);  // LBA 0: MBR/boot sector

    printf("First bytes of LBA0: %x %x %x %x\n",
           buf[0], buf[1], buf[2], buf[3]);

    // buclă infinită
    for (;;) {
        printf("SerbanOS > ");
        readline(buf, sizeof(buf));
        eval_command(buf);
    }
}
