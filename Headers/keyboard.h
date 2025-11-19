#ifndef KEYBOARD_H_
#define KEYBOARD_H_

// Inițializează tastatura (deocamdată doar resetează bufferul)
void keyboard_init(void);

// Handler apelat din IRQ1 (din idt.c)
void keyboard_irq_handler(void);

// Citește UN caracter (blocant)
char getchar(void);

// Citește un șir până la Enter, cu limită de lungime
void readline(char *buf, int max_len);

#endif
