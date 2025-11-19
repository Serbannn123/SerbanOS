#include<stdarg.h>

#include "screen.h"
#include "io.h"
#include "keyboard.h"
#include "util.h"

char *video = (char *)0xB8000;

int cursor_row = 0, cursor_col = 0;

void move_cursor(int row, int col)
{
    unsigned short pos = row * VGA_WIDTH + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void clear_screen()
{
    int index;

    for(int i = 0;i < VGA_HEIGHT;i++)
    {
        for(int j = 0;j < VGA_WIDTH;j++)
        {
            index = i*VGA_WIDTH + j;
            video[index * 2] = ' ';
            video[index * 2 + 1] = WHITE;
        }
    }

    cursor_row = 0;
    cursor_col = 0;
}

void putc(const char c)
{
    int index;

    if(c == '\n')
    {
        cursor_row++;
        cursor_col = 0;

        if (cursor_row >= VGA_HEIGHT)
        {
            scroll();
        }

        move_cursor(cursor_row, cursor_col);
        return;
    }

    if(c == '\b' && cursor_col > 0)
    {
        cursor_col--;
        move_cursor(cursor_row,cursor_col);
        return;
    }
    
    if(cursor_col >= VGA_WIDTH)
    {
        cursor_col = 0;
        cursor_row++;
    }

    if(cursor_row >= VGA_HEIGHT)
    {
        clear_screen();
        cursor_col = 0;
        cursor_row = 0;
    }

    if(cursor_col<VGA_WIDTH && cursor_row < VGA_HEIGHT)
    {
        index = cursor_row*VGA_WIDTH + cursor_col;

        video[index * 2] = c;
        video[index * 2 + 1] = WHITE;

        cursor_col++;
    }

    
    move_cursor(cursor_row,cursor_col);
}

void print(const char *s)
{
    while(*s)
    {
        putc(*s);
        s++;
    }
}

void print_dec(int n)
{
    short int c[16] = {0}, i = 0;

    if (n == 0)
    {
        putc('0');
        return;
    }

    if(n<0)
    {
        putc('-');
        n = -n;
    }

    while(n)
    {
        c[i++] = n%10;
        n=n/10;
    }

    for(i = i - 1;i>=0;i--)
    {
        putc('0' + c[i]); 
    }
    
    return;
}

void print_hex(unsigned int n)
{
    char hexdigit[] = "0123456789ABCDEF", buffer[] = "00000000";
    int i = 0;
    if(n == 0)
    {
        print("0x0");
        return;
    }

    while(n)
    {
        buffer[i++] = hexdigit[n%16];
        n = n/16;
    }

    print("0x");
    i = 7;

    while(i+1)
    {
        
        putc(buffer[i--]);
    }

    return;
}

void printf(const char *s, ...)
{
    va_list args;
    va_start(args, s);

    while(*s)
    {
        if(*s == '%')
        {
            s++;
            switch (*s)
            {
            case 'd':
                int val1 = va_arg(args, int);
                print_dec(val1);
                break;
            
            case 'x':
                unsigned int val = va_arg(args, unsigned int);
                print_hex(val);
                break;
                
            case 'c':
                const char c = va_arg(args, int);
                putc(c);
                break;

            case 's':
                const char *p = va_arg(args,const char*);
                print(p);
                break;

            case '%':
                putc('%');
                break;
            
            default:
                putc('%');
                putc(*s);
                break;
            }
        }
        else
        {
            putc(*s);
        }

        s++;
    }

    va_end(args);

    return;
}

void scroll()
{
    int from,to;
    if(cursor_row < VGA_HEIGHT)
    {
        return;
    }

    for(int i = 0;i < VGA_HEIGHT - 1;i++) //Scrol printre linii (De sus in jos),
    {
        for(int j = 0;j < VGA_WIDTH;j++)
        {
            from = (i+1) * VGA_WIDTH + j;
            to = i * VGA_WIDTH +j;

            video[to * 2] = video[from * 2];
            video[to * 2 + 1] = video[from * 2 + 1];
        }
    }

    for(int j = 0;j < VGA_WIDTH;j++)
    {
        from = (VGA_HEIGHT - 1) * VGA_WIDTH + j;

        video[from * 2] = ' ';
        video[from * 2 + 1] = WHITE;
    }

    cursor_row = VGA_HEIGHT - 1;
    cursor_col = 0;

    move_cursor(cursor_row, cursor_col);
}

void isr0_handler(void)
{
    printf("\n\n[EXPECTION] Divide by zero\n");
    panic("Divide by zero exception");
}

__attribute__((noreturn))
void panic(const char *msg)
{
    printf("\n\n*** KERNEL PANIC ***\n");
    printf("%s\n", msg);
    printf("System halted.\n");

    while (1)
    {
        __asm__ __volatile__("hlt");
    }
}

// void outb(uint16_t port, uint8_t value)
// {
//     __asm__ __volatile__ (
//         "outb %0, %1"
//         :
//         : "a"(value), "Nd"(port)
//     );
// }

void pic_remap()
{
    // 1) Trimitem comanda de initializare (ICW1) la ambele PIC-uri
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // 2) Setam vectorul de inceput pentru fiecare PIC (ICW2)
    outb(0x21, 0x20);  // IRQ0..7  -> IDT 0x20..0x27
    outb(0xA1, 0x28);  // IRQ8..15 -> IDT 0x28..0x2F

    // 3) Legatura master–slave (ICW3)
    outb(0x21, 0x04);  // slave pe IRQ2
    outb(0xA1, 0x02);

    // 4) ICW4 – 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // 🔴 AICI schimbăm masca
    // Pentru test: activăm DOAR IRQ1 (tastatura)
    // 1111 1101b = 0xFD -> doar bitul 1 = 0 (IRQ1 enabled)
    outb(0x21, 0xFC);  // master: timer (IRQ0) + tastatură (IRQ1)
    outb(0xA1, 0xFF);  // slave: toate mascate
}




//functii pt comenzi
