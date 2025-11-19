#include "keyboard.h"
#include "io.h"
#include "screen.h"

#define KB_BUF_SIZE 128

static char kb_buffer[KB_BUF_SIZE];
static int kb_head = 0;  // scriem aici
static int kb_tail = 0;  // citim de aici

// ======================
// Ring buffer intern
// ======================

static void kb_push(char c)
{
    int next = (kb_head + 1) % KB_BUF_SIZE;

    // dacă următoarea poziție e tail => buffer plin, ignorăm caracterul
    if (next == kb_tail) {
        return;
    }

    kb_buffer[kb_head] = c;
    kb_head = next;
}

static char kb_pop(void)
{
    // dacă bufferul e gol, blocăm CPU până vine ceva (hlt + IRQ)
    while (kb_head == kb_tail) {
        __asm__ __volatile__("hlt");
    }

    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF_SIZE;
    return c;
}

// ======================
// API public
// ======================

void keyboard_init(void)
{
    kb_head = 0;
    kb_tail = 0;
}

char getchar(void)
{
    return kb_pop();
}

void readline(char *buf, int max_len)
{
    int i = 0;

    while (1)
    {
        char c = getchar();

        if (c == '\n') {
            putc('\n');
            buf[i] = 0;          // terminator de șir
            return;
        }
        else if (c == '\b' || c == 0x7F) {
            // backspace
            if (i > 0) {
                i--;
                // ștergem vizual ultimul caracter
                putc('\b');
                putc(' ');
                putc('\b');
            }
        }
        else {
            if (i < max_len - 1) {
                buf[i++] = c;
                putc(c);         // echo pe ecran
            }
            // dacă e plin buffer-ul de user, ignorăm caracterele extra
        }
    }
}

// ======================
// Handler de IRQ1
// ======================

void keyboard_irq_handler(void)
{
    unsigned char scancode = inb(0x60);

    // break code (tasta ridicată)? ignorăm
    if (scancode & 0x80) {
        return;
    }

    // Tabel simplu scancode -> ASCII (layout US, fără Shift)
    static const char scancode_to_ascii[128] = {
        0,  27, '1','2','3','4','5','6',
        '7','8','9','0','-','=', '\b',   // 0x0E = backspace
        '\t',                            // 0x0F = tab
        'q','w','e','r','t','y','u','i',
        'o','p','[',']','\n',            // 0x1C = Enter
        0,                               // 0x1D = Ctrl
        'a','s','d','f','g','h','j','k',
        'l',';','\'','`',
        0,                               // LShift
        '\\',
        'z','x','c','v','b','n','m',',',
        '.','/',
        0,                               // RShift
        '*',
        0,                               // Alt
        ' ',                             // Space
        // restul rămân 0
    };

    char c = 0;
    if (scancode < 128) {
        c = scancode_to_ascii[scancode];
    }

    if (c != 0) {
        kb_push(c);   // DOAR punem în buffer, nu afișăm aici
    }

    // EOI îl dai deja în irq1_stub (asm), deci aici nu mai faci nimic
}
