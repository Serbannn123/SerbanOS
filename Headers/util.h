#ifndef UTIL_H_
#define UTIL_H_

int streq(const char *s,const char *p);
char *jmp_first_word(char *s);
int len(char *s);
int strleq(char *s, char *p);
int len(char *s);
void first_word(const char *src, char *dest, int dest_size);
void strcpy(const char *s,char *d, int len);

#endif