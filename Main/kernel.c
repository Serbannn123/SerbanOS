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
    char buf[128];

    // buclă infinită
    for (;;) {
        printf("SerbanOS > ");
        readline(buf, sizeof(buf));
        eval_command(buf);
    }
}
