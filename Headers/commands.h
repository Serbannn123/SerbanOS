#ifndef COMMANDS_H_
#define COMMANDS_H_

#define MAX_CMD_NAME 32

typedef void (*command_func_t)(const char *args);

typedef struct {
    const char *name;
    const char *help;
    command_func_t func;
} command_t;

extern command_t commands[];
extern const int NUM_COMMANDS;

void eval_command(char *input);

#endif