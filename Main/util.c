#include "util.h"

int streq(const char *s,const char *p)
{
    while(*s != '\0' && *p != '\0')
    {
        if(*s++ != *p++)
            return 0;
    }

    if(*s =='\0' && *p == '\0')
        return 1;

    return 0;
}

int strleq(char *s, char *p)
{
    for(int i = 0;i < len (p);i++)
    {
        if(s[i] != p[i])
            return 0;
    }

    return 1;
}

int len(char *s)
{
    int i = 0;
    while(s[i] != '\0')
    {
        i++;
    }

    return i;
}

char *jmp_first_word(char *s)
{
    while(*s != ' ' && *s !='\0')
    {
        s++;
    }
    if(*s != '\0')
        s++;

    return s;
}

void first_word(const char *src, char *dest, int dest_size)
{
    int i = 0;
    while (*src == ' ') src++;

    while (*src != ' ' && *src != '\0' && i < dest_size - 1) {
        dest[i++] = *src++;
    }

    dest[i] = '\0';
}

char *caps(char *s)
{
    int i = 0;

    while(*(s+i) != '\0')
    {
        if(*(s+i) >= 'a' && *(s+i) <= 'z')
        {
            *(s+i) = *(s+i) - 32;
        }
        i++;
    }

    return s;
}

void strcpy(const char *s,char *d, int len)
{
    int i = 0;

    for(i;i<len && s[i] != '\0';i++)
    {
        d[i] = s[i];
    }

    d[i] = '\0';
    return;
}
