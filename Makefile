BUILD_DIR = ./build
NASM = nasm
CC = gcc
AS = as
LD = ld
RM = rm -rf

LIB      = -I ./kernel -I ./lib -I ./device -I ./task
CFLAGS   = -mcmodel=large -fno-builtin -m64 -fno-stack-protector $(LIB)
LDFLAGS  = -b elf64-x86-64 -z muldefs -T kernel/kernel.lds
OBJS     =  $(BUILD_DIR)/head.o $(BUILD_DIR)/main.o $(BUILD_DIR)/printk.o \
	   		$(BUILD_DIR)/init.o $(BUILD_DIR)/screen.o $(BUILD_DIR)/trap.o \
			$(BUILD_DIR)/entry.o $(BUILD_DIR)/memory.o


.PHONY: clean build disk bochs qemu default

compile: $(OBJS)

link: compile
	$(LD) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/system

copy: link
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $(BUILD_DIR)/system $(BUILD_DIR)/kernel.bin



$(BUILD_DIR)/boot.bin: boot/boot.asm
	$(NASM) $< -o $@

$(BUILD_DIR)/loader.bin: boot/loader.asm
	$(NASM) $< -o $@


$(BUILD_DIR)/head.o: kernel/head.S kernel/linkage.h
	gcc -E kernel/head.S > $(BUILD_DIR)/head.s
	$(AS) --64 $(BUILD_DIR)/head.s -o $@ -a=$(BUILD_DIR)/1.lst

$(BUILD_DIR)/entry.o: kernel/entry.S
	gcc -E kernel/entry.S > $(BUILD_DIR)/entry.s
	$(AS) --64 build/entry.s -o $@


$(BUILD_DIR)/main.o: kernel/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/printk.o: lib/printk.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/screen.o: device/screen.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/init.o:kernel/init.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/trap.o:kernel/trap.c kernel/trap.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o:kernel/memory.c kernel/memory.h
	$(CC) $(CFLAGS) -c $< -o $@



clean:
	$(RM) $(RMFLAGS) $(BUILD_DIR)/* kernel/head.s tools/boot.img

disk: copy $(BUILD_DIR)/loader.bin $(BUILD_DIR)/boot.bin
	cp ./a.img tools/boot.img
	dd if=$(BUILD_DIR)/boot.bin of=tools/boot.img bs=512 count=1 conv=notrunc
	sudo mount tools/boot.img /media/tf -t vfat -o loop
	sudo cp $(BUILD_DIR)/loader.bin /media/tf
	sudo cp $(BUILD_DIR)/kernel.bin /media/tf
	sudo sync
	sudo umount /media/tf

bochs: clean compile link disk
	./bochs/bin/bochs -f tools/bochsrc

qemu: clean compile link disk
	qemu-system-x86_64 \
	-m 2048 \
	-drive file=./tools/boot.img,format=raw,if=floppy \
	-boot a \
	-cpu qemu64 \
	-enable-kvm \
	-smp 1 \
	-no-acpi \
	-vga std \
	-serial stdio \
	-parallel none \
	-net none \
	-display gtk \
	-name "QemuKernelDebug"

default: compile link 
