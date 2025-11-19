[org 0x7C00]
[bits 16]

start:
    cli

    ; segmente de bază
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    sti

    mov [boot_drive], dl      ; BIOS pune drive-ul de boot în DL

    call clear_screen

    mov si, msg_loading
    call print_string

    ; încarcă kernelul de pe disc la adresa fizică 0x00010000
    call load_kernel

    ; trecem în protected mode
    call enter_protected_mode

.hang:
    jmp .hang


; -----------------------
; load_kernel: citește N sectoare în 0x10000
; N = KERNEL_SECTORS (definit din Makefile)
; -----------------------

%ifndef KERNEL_SECTORS
    ; fallback dacă nu e definit (de ex. build manual)
    %define KERNEL_SECTORS 1
%endif

SECTORS_TO_READ equ KERNEL_SECTORS

load_kernel:
    mov ax, 0x1000          ; 0x1000:0000 = 0x00010000 fizic
    mov es, ax
    xor bx, bx              ; offset în buffer

    mov ch, 0               ; cilindru 0
    mov dh, 0               ; head 0
    mov dl, [boot_drive]    ; drive-ul de boot

    mov cl, 2               ; începem de la sectorul 2 (1 = bootloader)
    mov si, SECTORS_TO_READ ; contor sectoare de citit

.read_loop:
    mov ah, 0x02            ; citire 1 sector
    mov al, 1
    int 0x13
    jc disk_error           ; dacă e eroare la ORICE sector => mesaj

    add bx, 512             ; mutăm bufferul cu 512 bytes mai departe
    inc cl                  ; sectorul următor
    dec si
    jnz .read_loop

    ret

disk_error:
    mov si, msg_disk_error
    call print_string
.death:
    jmp .death


; -----------------------
; Intrare în Protected Mode
; -----------------------
enter_protected_mode:
    cli

    ; încarcă GDT
    lgdt [gdt_descriptor]

    ; setează PE bit în CR0
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    ; far jump în cod 32-bit (selector 0x08)
    jmp 0x08:protected_mode_start


; -----------------------
; De aici suntem în 32-bit
; -----------------------
[bits 32]
protected_mode_start:
    mov ax, 0x10        ; data segment selector (a 3-a intrare din GDT)
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov esp, 0x90000    ; un stack ok mai sus în memorie

    ; kernelul e la adresa fizică 0x00010000
    jmp 0x00010000      ; intrăm în kernel (entry point-ul lui)


; -----------------------
; FUNCȚII 16-bit (folosite înainte de PM)
; (codul de mai sus e 32-bit, dar datele de mai jos sunt doar date)
; -----------------------
[bits 16]

print_string:
    mov al, [si]
    cmp al, 0
    je .done

    cmp al, 0x0A
    je .newline

    mov ah, 0x0E
    int 0x10

    inc si
    jmp print_string

.done:
    ret

.newline:
    mov ah, 0x0E
    mov al, 0x0D
    int 0x10
    mov al, 0x0A
    int 0x10
    inc si
    jmp print_string


clear_screen:
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    ret


; -----------------------
; GDT (Global Descriptor Table)
; -----------------------

gdt_start:
    ; descriptor nul
    dq 0x0000000000000000

    ; code segment: base 0, limit 4GB, flags 0x9A, granularity 4KB
    dq 0x00CF9A000000FFFF

    ; data segment: base 0, limit 4GB, flags 0x92, granularity 4KB
    dq 0x00CF92000000FFFF

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


; -----------------------
; DATE
; -----------------------
boot_drive      db 0
msg_loading     db "Loading 32-bit C kernel...", 0x0A, 0
msg_disk_error  db "Disk read error!", 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
