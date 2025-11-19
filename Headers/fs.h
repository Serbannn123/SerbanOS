#ifndef FS_H_
#define FS_H_

#define MAX_FILE_NUMBER 16
#define MAX_FILE_NAME 32
#define MAX_FILE_SIZE 512

typedef struct 
{
    int used;
    char name[MAX_FILE_NAME];
    char data[MAX_FILE_SIZE];
    int size;
}fs_file_t;

void fs_init(void);
fs_file_t *fs_find(const char *name);
fs_file_t *fs_create(const char *name);
int fs_write(fs_file_t *file, const char *data);
void fs_list_files(void);
void fs_write_file(const char *name, const char *data);
void fs_remove(const char *name);
#endif