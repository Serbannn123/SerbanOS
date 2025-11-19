#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#define VGA_HEIGHT 25
#define VGA_WIDTH 80
#define WHITE 0x07

#include <stdint.h>

void clear_screen();
void putc(const char c);
void move_cursor(int row, int col);
void print(const char *p);
void print_dec(int n);
void print_hex(unsigned int n);
void printf(const char *s, ...);
void scroll();
void isr0_handler(void);
void panic(const char *msg);
// void outb(uint16_t port, uint8_t value);
// uint8_t inb(uint16_t port);
void pic_remap();



#endif