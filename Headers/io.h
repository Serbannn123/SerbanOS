#ifndef IO_H
#define IO_H

#ifndef __INTELLISENSE__   // VS Code nu va intra aici
static inline void outb(unsigned short port, unsigned char val)
{
    __asm__ __volatile__ (
        "outb %0, %1"
        :
        : "a"(val), "Nd"(port)
    );
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char ret;
    __asm__ __volatile__ (
        "inb %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}

static inline unsigned short inw(unsigned short port)
{
    unsigned short ret;
    __asm__ __volatile__ (
        "inw %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );
    return ret;
}

static inline void outw(unsigned short port, unsigned short val)
{
    __asm__ __volatile__ (
        "outw %0, %1"
        :
        : "a"(val), "Nd"(port)
    );
}
#else
// Pentru IntelliSense doar declarații goale, fără asm
static inline void          outb(unsigned short port, unsigned char val);
static inline unsigned char inb(unsigned short port);
static inline void          outw(unsigned short port, unsigned short val);
#endif

#endif
