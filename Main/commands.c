#include "commands.h"
#include "io.h"
#include "screen.h"
#include <stddef.h>
#include "fs.h"
#include "util.h"

void help_command(const char *args);
void reboot_command(const char *args);
void echo_command(const char *args);
void ticks_command(const char *args);
void ls_command(const char *args);
void cat_command(const char *args);
void touch_command(const char *args);
void write_command(const char *args);
void remove_command(const char *args);

command_t commands[] = {
    { "help",  "show this help",      help_command },
    { "clear", "clear the screen",    clear_screen },
    { "echo",  "echo text",           echo_command },
    { "ticks", "show timer ticks",    ticks_command },
    // { "reboot",  "reboot system",     reboot_command },
    { "ls",    "list files",          ls_command },
    { "cat",   "show file content",   cat_command },
    { "touch", "create empty file",   touch_command },
    {"write", "Rewrite content of a file", write_command},
    {"rm", "delete a file", remove_command}
};

const int NUM_COMMANDS = sizeof(commands) / sizeof(commands[0]);

void eval_command(char *input)
{
    char cmd_name[MAX_CMD_NAME];
    first_word(input, cmd_name, MAX_CMD_NAME);

    const char *args = jmp_first_word(input);

    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (streq(cmd_name, commands[i].name)) {
            commands[i].func(args);
            return;
        }
    }

    printf("Unknown command: %s\n", cmd_name);
}

void help_command(const char *args)
{
    for(int i = 0; i < NUM_COMMANDS; i++)
    {
        printf("%s", caps(commands[i].name));

        for(int j = 0;j<10-len(commands[i].name);j++)
        {
            printf(" ");
        }

        printf("%s\n",commands[i].help);
    }
}

void echo_command(const char *args)
{
    if(args == NULL || *args == '\0')
    {
        printf("\n");
        return;
    }
    else
        printf("%s\n", args);
}

void reboot_command(const char *args)
{
    asm volatile ("cli");
    asm volatile ("lidt (0)");
    asm volatile ("int3");
}

extern volatile unsigned int timer_ticks;

void ticks_command(const char *args)
{
    printf("Timer ticks since boot: %d\n", (int)timer_ticks);
}

void touch_command(const char *args)
{
    if (!args) {
        printf("Usage: touch <filename>\n");
        return;
    }

    while (*args == ' ')
        args++;

    if (*args == '\0') {
        printf("Usage: touch <filename>\n");
        return;
    }

    fs_file_t *f = fs_create(args);
    if (!f) {
        printf("Cannot create file (FS full?)\n");
        return;
    }

    printf("File '%s' ready (size=%d)\n", f->name, f->size);
}

void ls_command(const char *args)
{
        fs_list_files();
}

void cat_command(const char *args)
{
    if(!args)
        return;

    while (*args == ' ')
        args++;

    if (*args == '\0') {
        printf("Usage: cat <filename>\n");
        return;
    }

    fs_file_t *file = fs_find(args);

    if(!file)
    {
        printf("File not found\n");
        return;
    }

    printf("%s\n",file->data);
}

void write_command(const char *args)
{
    char name[MAX_FILE_NAME];
    char *data;

    // extragem primul cuvânt în bufferul 'name'
    first_word(args, name, MAX_FILE_NAME);

    // restul liniei după primul cuvânt (textul de scris)
    data = jmp_first_word((char *)args);

    printf("%s __ %s\n", name, data);

    fs_write_file(name, data);
}

void remove_command(const char *args)
{
    if(!args || *args == '\0')
    {
        printf("Use: rm + filename\n");
        return;
    }

    fs_remove(args);
}

