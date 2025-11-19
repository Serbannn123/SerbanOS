//Definim IDT - ul
#ifndef IDT_H_
#define IDT_H_

#include <stdint.h>

struct idt_entry
{
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_high;
}__attribute__((packed));

struct idt_ptr
{
    unsigned short limit;
    unsigned int base;
}__attribute__((packed));

void idt_install(void);
void isr_install(void);
void isr_handler(uint32_t int_no);

#endif
