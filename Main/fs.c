#include "fs.h"
#include "screen.h"
#include "util.h"
#include "disk.h"

#define FS_TABLE_LBA 100
#define FS_MAX_ONDISK_FILES 8

#define FS_DATA_BASE_LBA   101 

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

            strcpy(files[i].name, table[i].name, MAX_FILE_NAME - 1);
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

    // 🔥 NOU: salvăm tabela pe disc la FS_TABLE_LBA
    fs_sync_table();
}


fs_file_t *fs_create(const char *name)
{
    fs_file_t *found = 0;

    if (!name || name[0] == '\0')
        return 0;

    // verificare daca fisierul exista deja
    found = fs_find(name);
    if (found)
    {
        print("File already exist!\n");
        return 0;
    }

    for (int i = 0; i < MAX_FILE_NUMBER; i++)
    {
        if (!files[i].used)
        {
            files[i].used = 1;

            // primele FS_MAX_ONDISK_FILES sunt persistente pe disc
            if (i < FS_MAX_ONDISK_FILES)
            {
                files[i].on_disk   = 1;
                files[i].lba_start = FS_DATA_BASE_LBA + i;  // sectorul pentru acest slot
            }
            else
            {
                files[i].on_disk   = 0;  // RAM-only
                files[i].lba_start = 0;
            }

            strcpy(name, files[i].name, MAX_FILE_NAME);
            files[i].data[0] = '\0';
            files[i].size    = 0;

            // 🔥 NOU: dacă fișierul e pe disc, actualizăm imediat tabela
            if (files[i].on_disk)
            {
                fs_sync_table();
            }

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
    if (!file || !data)
        return 0;

    int n = len((char *)data);
    if (n > MAX_FILE_SIZE)
        n = MAX_FILE_SIZE;

    // copiem in bufferul din RAM
    strcpy(data, file->data, n);
    file->size = n;

    // daca fisierul e pe disc, scriem si sectorul
    if (file->on_disk && file->lba_start != 0)
    {
        unsigned char sector[512];

        // umplem sectorul cu 0
        for (int i = 0; i < 512; i++)
            sector[i] = 0;

        // copiem continutul text in sector
        for (int i = 0; i < n && i < 511; i++)
            sector[i] = (unsigned char)file->data[i];

        sector[n] = '\0';

        if (!ata_write_sector(file->lba_start, sector))
        {
            print("FS: failed to write file data to disk.\n");
        }
    }

    // actualizam tabela de pe disc (dimensiune, lba, nume, used)
    fs_sync_table();

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

    if (!file)
    {
        printf("File not found\n");
        return;
    }

    fs_write(file, data);
}

void fs_remove(const char *name)
{
    fs_file_t *file = fs_find(name);

    if (!file)
    {
        printf("File not found!\n");
        return;
    }

    // daca e pe disc, optional curatam sectorul
    if (file->on_disk && file->lba_start != 0)
    {
        unsigned char sector[512];
        for (int i = 0; i < 512; i++)
            sector[i] = 0;

        ata_write_sector(file->lba_start, sector);
    }

    file->name[0] = '\0';
    file->data[0] = '\0';
    file->used     = 0;
    file->size     = 0;
    file->on_disk  = 0;
    file->lba_start = 0;

    // actualizam tabela FS pe disc
    fs_sync_table();

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

void fs_sync_table(void)
{
    unsigned char sector[512];

    // curățăm sectorul
    for (int i = 0; i < 512; i++) {
        sector[i] = 0;
    }

    disk_fs_entry_t *table = (disk_fs_entry_t *)sector;

    // copiem din RAM (files[]) în tabela de pe disc
    for (int i = 0; i < FS_MAX_ONDISK_FILES && i < MAX_FILE_NUMBER; i++) {
        if (files[i].used) {
            table[i].used      = 1;
            table[i].size      = files[i].size;
            table[i].lba_start = files[i].lba_start;

            // nume: din RAM în structura de pe disc
            // util.h: void strcpy(const char *s, char *d, int len);
            strcpy(files[i].name, table[i].name, 31);
            table[i].name[30] = '\0';   // siguranță (max ~30 caractere)
        } else {
            table[i].used      = 0;
            table[i].name[0]   = '\0';
            table[i].size      = 0;
            table[i].lba_start = 0;
        }
    }

    if (!ata_write_sector(FS_TABLE_LBA, sector)) {
        print("FS: failed to write table to disk.\n");
    } else {
        print("FS: table synced to disk.\n");
    }
}







