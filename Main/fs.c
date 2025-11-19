#include "fs.h"
#include "screen.h"
#include "util.h"
#include "disk.h"

#define FS_TABLE_LBA 0
#define FS_MAX_ONDISK_FILES 8

fs_file_t files[MAX_FILE_NUMBER];

typedef struct 
{
    unsigned char used;
    char name[31];      // nume pe disc, max 31 char
    uint32_t size;  
    uint32_t lba_start; // sectorul de început al datelor
} disk_fs_entry_t;

void fs_init(void)
{
    // 1) Reset structuri din RAM
    for (int i = 0; i < MAX_FILE_NUMBER; i++) {
        files[i].used      = 0;
        files[i].on_disk   = 0;
        files[i].lba_start = 0;
        files[i].name[0]   = '\0';
        files[i].data[0]   = '\0';
        files[i].size      = 0;
    }

    // 2) Citim sectorul cu tabela de pe disc
    unsigned char sector[512];

    if (!ata_read_sector(FS_TABLE_LBA, sector)) {
        print("FS: disk read failed, using RAM defaults.\n");
        goto ram_defaults;
    }

    // interpretăm sectorul ca un vector de disk_fs_entry_t
    disk_fs_entry_t *table = (disk_fs_entry_t *)sector;

    int any_used = 0;

    for (int i = 0; i < FS_MAX_ONDISK_FILES && i < MAX_FILE_NUMBER; i++) {
        if (table[i].used) {
            any_used = 1;

            files[i].used      = 1;
            files[i].on_disk   = 1;
            files[i].lba_start = table[i].lba_start;

            strcpy(table[i].name, files[i].name, MAX_FILE_NAME - 1);
            files[i].name[MAX_FILE_NAME - 1] = '\0';

            files[i].size = table[i].size;
            files[i].data[0] = '\0';
        }
    }

    if (any_used) {
        print("FS: file table loaded from disk.\n");
        return;
    }

ram_defaults:
    print("FS: no table on disk, using RAM defaults.\n");

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
            files[i].used      = 1;
            files[i].on_disk   = 0;       // fișier nou: doar în RAM
            files[i].lba_start = 0;
            strcpy(name, files[i].name, MAX_FILE_NAME);
            files[i].data[0]   = '\0';
            files[i].size      = 0;
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
    file->size = 0;

    printf("%s deleted!\n", name);
}

int fs_load_content(fs_file_t *file)
{
    if (!file)
        return 0;

    // Dacă nu e marcat ca "on_disk", presupunem că data[] e deja bună
    if (!file->on_disk)
        return 1;

    // Deocamdată: presupunem 1 sector / fișier, text, null-terminated
    if (file->lba_start == 0) {
        // nimic valid pe disc
        return 0;
    }

    unsigned char buf[512];
    ata_read_sector(file->lba_start, buf);

    // Copiem în bufferul din RAM, max MAX_FILE_SIZE-1 și până la '\0'
    int i = 0;
    while (i < MAX_FILE_SIZE - 1 && buf[i] != '\0') {
        file->data[i] = buf[i];
        i++;
    }

    file->data[i] = '\0';
    file->size    = i;

    // dacă vrei, după prima citire îl poți trata ca RAM-only
    file->on_disk = 0;

    return 1;
}