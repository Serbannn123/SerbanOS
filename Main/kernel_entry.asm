[bits 32]

global _start
global idt_load
global isr0_stub
global irq0_stub
global irq1_stub

extern kmain
extern isr_handler          ; void isr_handler(uint32_t int_no);
extern irq0_handler_c       ; void irq0_handler_c(void);
extern irq1_handler_c       ; void irq1_handler_c(void);

; ==========================
; Intrarea în kernel
; ==========================
_start:
    call kmain

.hang:
    hlt
    jmp .hang


; ==========================
; void idt_load(struct idt_ptr* ptr)
; C pune pe stivă: [ret] [ptr]
; ==========================
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; ==========================
; IRQ0 (timer)
; vector: 0x20 -> irq0_stub
; ==========================
irq0_stub:
    pusha

    call irq0_handler_c      ; handler-ul în C

    ; EOI către PIC master (pentru IRQ0)
    mov al, 0x20
    out 0x20, al

    popa
    iret

; ==========================
; IRQ1 (tastatura)
; vector: 0x21 -> irq1_stub
; ==========================
irq1_stub:
    pusha

    call irq1_handler_c      ; handler-ul tău din C (keyboard.c)

    ; EOI către PIC master
    mov al, 0x20
    out 0x20, al

    popa
    iret


; ==========================
; Macro pentru excepții fără error code
; ==========================
%macro ISR_NOERR 1
global isr%1_stub
isr%1_stub:
    pusha
    push dword %1          ; numărul excepției (int_no)
    call isr_handler       ; handler comun în C
    add esp, 4             ; scoatem int_no de pe stivă
    popa
    iret
%endmacro

; Excepțiile 0–31
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_NOERR 8
ISR_NOERR 9
ISR_NOERR 10
ISR_NOERR 11
ISR_NOERR 12
ISR_NOERR 13
ISR_NOERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; Tabel cu pointeri la toate stub-urile
global isr_stub_table
isr_stub_table:
    dd isr0_stub
    dd isr1_stub
    dd isr2_stub
    dd isr3_stub
    dd isr4_stub
    dd isr5_stub
    dd isr6_stub
    dd isr7_stub
    dd isr8_stub
    dd isr9_stub
    dd isr10_stub
    dd isr11_stub
    dd isr12_stub
    dd isr13_stub
    dd isr14_stub
    dd isr15_stub
    dd isr16_stub
    dd isr17_stub
    dd isr18_stub
    dd isr19_stub
    dd isr20_stub
    dd isr21_stub
    dd isr22_stub
    dd isr23_stub
    dd isr24_stub
    dd isr25_stub
    dd isr26_stub
    dd isr27_stub
    dd isr28_stub
    dd isr29_stub
    dd isr30_stub
    dd isr31_stub
