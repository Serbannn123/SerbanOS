#ifndef FS_H_
#define FS_H_

#define MAX_FILE_NUMBER 16
#define MAX_FILE_NAME 32
#define MAX_FILE_SIZE 512

#include <stdint.h>   // adaugă asta sus în fs.h

typedef struct 
{
    int used;
    int on_disk;                 // 0 = doar în RAM, 1 = are date pe disc
    uint32_t lba_start;          // de la ce sector pornește conținutul pe disc
    char name[MAX_FILE_NAME];
    char data[MAX_FILE_SIZE];
    int size;
} fs_file_t;

void fs_init(void);
fs_file_t *fs_find(const char *name);
fs_file_t *fs_create(const char *name);
int fs_write(fs_file_t *file, const char *data);
void fs_list_files(void);
void fs_write_file(const char *name, const char *data);
void fs_remove(const char *name);
int fs_load_content(fs_file_t *file);   // nou, îl folosim la cat

#endif