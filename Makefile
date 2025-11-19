# ===== Paths =====
SRCDIR   := Main
INCDIR   := Headers
BUILDDIR := Build
RELDIR   := Relevant_files

# ===== Tools =====
CC      = gcc
LD      = ld
AS      = nasm
OBJCOPY = objcopy

CFLAGS  = -m32 -ffreestanding -Wall -Wextra -O2 -I$(INCDIR)
LDFLAGS = -m elf_i386 -T $(INCDIR)/linker.ld -nostdlib

# ===== Objects =====
OBJS = \
  $(BUILDDIR)/kernel_entry.o \
  $(BUILDDIR)/kernel.o \
  $(BUILDDIR)/screen.o \
  $(BUILDDIR)/idt.o \
  $(BUILDDIR)/keyboard.o \
  $(BUILDDIR)/commands.o \
  $(BUILDDIR)/fs.o \
  $(BUILDDIR)/util.o \
  $(BUILDDIR)/disk.o

# Fișiere relevante (cod) care vor fi copiate în Relevant_files
RELEVANT_SRCS = \
  $(SRCDIR)/* \
  $(INCDIR)/* \
  Makefile

# Fișierul mare cu TOT codul concatenat
BUNDLE_FILE := $(RELDIR)/all_sources.txt

# Target implicit
all: dirs $(BUILDDIR)/os-image.bin copy_relevant

# ===== Creare directoare =====
dirs:
	mkdir -p $(BUILDDIR) $(RELDIR)

# ===== Build image: bootloader (512B) + kernel =====
$(BUILDDIR)/os-image.bin: $(BUILDDIR)/boot.bin $(BUILDDIR)/kernel.bin
	cat $(BUILDDIR)/boot.bin $(BUILDDIR)/kernel.bin > $(BUILDDIR)/os-image.bin
	truncate -s %512 $(BUILDDIR)/os-image.bin

# ===== Bootloader raw bin =====
# Calculăm automat numărul de sectoare al kernelului și îl dăm la NASM
$(BUILDDIR)/boot.bin: $(SRCDIR)/boot.asm $(BUILDDIR)/kernel.bin
	@KERNEL_SECTORS=$$((($$(stat -c%s $(BUILDDIR)/kernel.bin) + 511) / 512)); \
	echo "KERNEL_SECTORS = $$KERNEL_SECTORS"; \
	$(AS) -f bin $(SRCDIR)/boot.asm -o $(BUILDDIR)/boot.bin -DKERNEL_SECTORS=$$KERNEL_SECTORS

# ===== Kernel raw bin =====
$(BUILDDIR)/kernel.bin: $(BUILDDIR)/kernel.elf
	$(OBJCOPY) -O binary $(BUILDDIR)/kernel.elf $(BUILDDIR)/kernel.bin

# ===== Link kernel ELF =====
$(BUILDDIR)/kernel.elf: $(OBJS) $(INCDIR)/linker.ld
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/kernel.elf $(OBJS)

# ===== Object files =====
$(BUILDDIR)/kernel_entry.o: $(SRCDIR)/kernel_entry.asm
	$(AS) -f elf32 $(SRCDIR)/kernel_entry.asm -o $(BUILDDIR)/kernel_entry.o

$(BUILDDIR)/kernel.o: $(SRCDIR)/kernel.c $(INCDIR)/screen.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/kernel.c -o $(BUILDDIR)/kernel.o

$(BUILDDIR)/screen.o: $(SRCDIR)/screen.c $(INCDIR)/screen.h $(INCDIR)/io.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/screen.c -o $(BUILDDIR)/screen.o

$(BUILDDIR)/idt.o: $(SRCDIR)/idt.c $(INCDIR)/idt.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/idt.c -o $(BUILDDIR)/idt.o

$(BUILDDIR)/keyboard.o: $(SRCDIR)/keyboard.c $(INCDIR)/keyboard.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/keyboard.c -o $(BUILDDIR)/keyboard.o

$(BUILDDIR)/commands.o: $(SRCDIR)/commands.c $(INCDIR)/commands.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/commands.c -o $(BUILDDIR)/commands.o

$(BUILDDIR)/fs.o: $(SRCDIR)/fs.c $(INCDIR)/fs.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/fs.c -o $(BUILDDIR)/fs.o

$(BUILDDIR)/util.o: $(SRCDIR)/util.c $(INCDIR)/util.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/util.c -o $(BUILDDIR)/util.o

$(BUILDDIR)/disk.o: $(SRCDIR)/disk.c $(INCDIR)/disk.h $(INCDIR)/io.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/disk.c -o $(BUILDDIR)/disk.o

# ===== Copiere fișiere relevante =====
copy_relevant:
	mkdir -p $(RELDIR)
	cp $(RELEVANT_SRCS) $(RELDIR)/

# ===== Concatenează toate fișierele relevante cu delimitare =====
bundle_relevant: copy_relevant
	@echo "Generare $(BUNDLE_FILE)..."
	@rm -f $(BUNDLE_FILE)
	@for f in $(RELEVANT_SRCS); do \
	  name=$$(basename $$f); \
	  echo "============================" >> $(BUNDLE_FILE); \
	  echo "=== $$name" >> $(BUNDLE_FILE); \
	  echo "============================" >> $(BUNDLE_FILE); \
	  cat $$f >> $(BUNDLE_FILE); \
	  echo >> $(BUNDLE_FILE); \
	  echo >> $(BUNDLE_FILE); \
	done
	@echo "Creat: $(BUNDLE_FILE)"

# ===== Rulează QEMU cu resurse limitate =====
run: $(BUILDDIR)/os-image.bin bundle_relevant
	qemu-system-i386 \
	  -drive format=raw,file=$(BUILDDIR)/os-image.bin \
	  -m 16M \
	  -smp 1 \
	  -serial mon:stdio 

# ===== Curățare =====
clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/*.bin $(BUILDDIR)/*.elf $(BUILDDIR)/os-image.bin

.PHONY: all dirs copy_relevant bundle_relevant run clean
