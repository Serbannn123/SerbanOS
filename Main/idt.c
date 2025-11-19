#include"idt.h"
#include"io.h"
#include"screen.h"
#include"keyboard.h"
#include "util.h"

static struct idt_entry idt[256];
static struct idt_ptr idtp;

extern void idt_load(struct idt_ptr *ptr);
extern void isr0_stub(void);
extern void irq0_stub(void);
extern void irq1_stub(void);

// extern void isr0_stub();
// extern void isr1_stub();
// extern void isr31_stub();

// extern void idt_load(struct idt_ptr *ptr);
// extern void irq1_stub(void);

extern void (*isr_stub_table[])(void);

static void idt_set_gate(int n, unsigned int handler_addr, unsigned short selector, unsigned char type_attr)
{
    idt[n].offset_low = handler_addr & 0xFFFF;
    idt[n].selector = selector;
    idt[n].zero = 0;
    idt[n].type_attr = type_attr;
    idt[n].offset_high = (handler_addr >> 16) & 0xFFFF;
}

void idt_install(void)
{
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (unsigned int)&idt;

    // Golim toate intrările
    for (int i = 0; i < 256; i++)
    {
        idt[i].offset_low  = 0;
        idt[i].selector    = 0;
        idt[i].zero        = 0;
        idt[i].type_attr   = 0;
        idt[i].offset_high = 0;
    }

    // 0–31: excepțiile CPU, setate generic în isr_install()
    isr_install();

    // IRQ1 (tastatură) pe intrarea 0x21
    // idt_set_gate(0x21, (unsigned int)irq1_stub, 0x08, 0x8E);

    idt_set_gate(0,    (unsigned int)isr0_stub, 0x08, 0x8E);   // excepția 0
    idt_set_gate(0x20, (unsigned int)irq0_stub, 0x08, 0x8E);   // TIMER IRQ0
    idt_set_gate(0x21, (unsigned int)irq1_stub, 0x08, 0x8E);   // Tastatură IRQ1

    // Încărcăm IDT-ul
    idt_load(&idtp);
}

volatile unsigned int timer_ticks = 0;

void irq0_handler_c(void)
{
    timer_ticks++;
}

void irq1_handler_c(void)
{
    keyboard_irq_handler();
}

static const char *exception_messages[] = {
    "Division by zero",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 FPU error",
    "Alignment check",
    "Machine check",
    "SIMD FP exception",
    "Virtualization exception",
    "Security exception",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved"
};

void isr_handler(uint32_t int_no)
{
    printf("\n*** CPU EXCEPTION %d ***\n", int_no);
    printf("%s\n", exception_messages[int_no]);
    printf("System halted.\n");

    while (1) {
        __asm__ __volatile__("hlt");
    }
}

void isr_install(void)
{
    // Avem în ASM un tabel cu adresele isr0_stub ... isr31_stub
    // declarat ca: extern void (*isr_stub_table[])(void);

    for (int i = 0; i < 32; i++)
    {
        unsigned int handler_addr = (unsigned int)isr_stub_table[i];
        idt_set_gate(i, handler_addr, 0x08, 0x8E);
        // 0x08 = selector segment cod în GDT
        // 0x8E = interrupt gate, present, ring 0
    }
}