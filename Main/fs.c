#include "fs.h"
#include "screen.h"
#include "util.h"

fs_file_t files[MAX_FILE_NUMBER];

void fs_init(void)
{
    for (int i = 0; i < MAX_FILE_NUMBER; i++) {
        files[i].used = 0;
        files[i].name[0] = '\0';
        files[i].data[0] = '\0';
        files[i].size = 0;
    }

    // Exemplu: creăm câteva fișiere default folosind chiar API-ul nostru
    fs_file_t *f1 = fs_create("readme.txt");
    if (f1) {
        fs_write(f1,
            "SerbanOS RAM FS\n"
            "Acesta este un fisier creat la boot.\n"
        );
    }

    fs_file_t *f2 = fs_create("help.txt");
    if (f2) {
        fs_write(f2,
            "Comenzi filesystem:\n"
            "  ls          - listeaza fisierele\n"
            "  cat <nume>  - afiseaza continutul\n"
            "  touch <nume>- creeaza fisier gol\n"
        );
    }
}

fs_file_t *fs_create(const char *name)
{
    fs_file_t *found = 0;

    if(!name || name[0] == '\0')
        return 0;

    //verificare daca fisierul exista
    found = fs_find(name);

    if(found)
    {
        print("File already exist!\n");
        return 0;
    }

    for(int i = 0;i < MAX_FILE_NUMBER;i++)
    {
        if(!files[i].used)
        {
            files[i].used = 1;
            strcpy(name,files[i].name,MAX_FILE_NAME);
            files[i].data[0]  = '\0';
            files[i].size = 0;
            return &files[i];
        }
    }

    return 0;
}

fs_file_t *fs_find(const char *name)
{
    if (!name)
        return 0;

    for(int i = 0;i < MAX_FILE_NUMBER;i++)
    {
        if(files[i].used && streq(files[i].name, name))
        {
            return &files[i];
        }
    }

    return 0;
}

int fs_write(fs_file_t *file, const char *data)
{
    if(!file || !data)
    {
        return 0;
    }

    int n = len((char *) data);
    if(n > MAX_FILE_SIZE)
    {
        n = MAX_FILE_SIZE;
    }

    strcpy(data,file->data,n);

    file->size = n;

    return n;
}

void fs_list_files(void)
{
    for (int i = 0; i < MAX_FILE_NUMBER; i++) {
        if (files[i].used) {
            printf("%s  (%d bytes)\n", files[i].name, files[i].size);
        }
    }
}

void fs_write_file(const char *name, const char *data)
{
    fs_file_t *file = fs_find(name);

    if (!file) {
        printf("File not found\n");
        return;
    }

    int n = len((char *)data);
    if (n > MAX_FILE_SIZE)
        n = MAX_FILE_SIZE;

    strcpy(data, file->data, n);
    file->data[n] = '\0';
    file->size = n;
}

void fs_remove(const char *name)
{
    fs_file_t *file = 0;

    file = fs_find(name);

    if(!file)
    {
        printf("File not found!\n");
        return;
    }

    file->name[0] = '\0';
    file->data[0] = '\0';
    file->used = 0;
    file->used = 0;

    printf("%s deleted!\n", name);
}
