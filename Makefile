SHELL = /bin/sh
BUILD_DIR = build
MAKE = make
NASM = nasm
AS = as
CC = gcc
LD = ld
AR = ar
RM = rm -rf



LIB      = -I kernel -I lib -I device -I task -I fs -I task
CFLAGS   = -mcmodel=large -march=x86-64 -fno-builtin -m64 -fno-stack-protector -w -fno-pic -fno-pie -fdiagnostics-color=always $(LIB)
LDFLAGS  = -b elf64-x86-64 -z muldefs -T kernel/kernel.lds
OBJS     =  $(BUILD_DIR)/head.o $(BUILD_DIR)/main.o $(BUILD_DIR)/printk.o \
			$(BUILD_DIR)/init.o $(BUILD_DIR)/screen.o $(BUILD_DIR)/string.o \
			$(BUILD_DIR)/trap.o $(BUILD_DIR)/entry.o $(BUILD_DIR)/memory.o \
			$(BUILD_DIR)/interrupt.o $(BUILD_DIR)/cpu.o $(BUILD_DIR)/task.o \
			$(BUILD_DIR)/APIC.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/mouse.o \
			$(BUILD_DIR)/disk.o $(BUILD_DIR)/SMP.o $(BUILD_DIR)/APU_boot.o \
			$(BUILD_DIR)/time.o $(BUILD_DIR)/HPET.o $(BUILD_DIR)/softirq.o \
			$(BUILD_DIR)/timer.o $(BUILD_DIR)/schedule.o $(BUILD_DIR)/fat32.o \
			$(BUILD_DIR)/log.o

PIC := PIC_APIC

.PHONY: clean build disk bochs qemu default

compile: $(OBJS)

link: compile
	@$(LD) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/system

copy: link
	@objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $(BUILD_DIR)/system $(BUILD_DIR)/kernel.bin



$(BUILD_DIR)/boot.bin: boot/boot.asm
	$(NASM) $< -o $@

$(BUILD_DIR)/loader.bin: boot/loader.asm
	$(NASM) $< -o $@




$(BUILD_DIR)/head.o: kernel/head.S kernel/linkage.h
	@gcc -E kernel/head.S > $(BUILD_DIR)/head.s
	@$(AS)  $(BUILD_DIR)/head.s -o $@ -a=$(BUILD_DIR)/1.lst

$(BUILD_DIR)/entry.o: kernel/entry.S
	@gcc -E kernel/entry.S > $(BUILD_DIR)/entry.s
	@$(AS)  build/entry.s -o $@

$(BUILD_DIR)/APU_boot.o: kernel/APU_boot.S
	@gcc -E kernel/APU_boot.S > $(BUILD_DIR)/APU_boot.s
	@$(AS)  build/APU_boot.s -o $@




$(BUILD_DIR)/main.o: kernel/main.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/printk.o: lib/printk.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/screen.o: device/screen.c
	@@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/init.o:kernel/init.c
	@$(CC) $(CFLAGS) -c $< -o $@ -D$(PIC)

$(BUILD_DIR)/trap.o:kernel/trap.c kernel/trap.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o:kernel/memory.c kernel/memory.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/interrupt.o:kernel/interrupt.c kernel/interrupt.h
	@$(CC) $(CFLAGS) -c $< -o $@ -D$(PIC)

$(BUILD_DIR)/cpu.o:kernel/cpu.c kernel/cpu.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/task.o:task/task.c task/task.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/schedule.o:task/schedule.c task/schedule.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/string.o:lib/string.c lib/string.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/APIC.o:device/APIC.c device/APIC.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o:device/keyboard.c device/keyboard.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mouse.o:device/mouse.c device/mouse.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/disk.o:device/disk.c device/disk.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/time.o:device/time.c device/time.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/timer.o:device/timer.c device/timer.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/HPET.o:device/HPET.c device/HPET.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/SMP.o:kernel/SMP.c kernel/SMP.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/softirq.o:kernel/softirq.c kernel/softirq.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/fat32.o:fs/fat32.c fs/fat32.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/log.o:lib/log.c lib/log.h
	@$(CC) $(CFLAGS) -c $< -o $@




clean:
	$(RM) $(RMFLAGS) kernel/head.s tools/boot.img build/* tools/*.lock

disk: copy $(BUILD_DIR)/loader.bin $(BUILD_DIR)/boot.bin
	cp ./disk/hard.img tools/boot.img
	dd if=$(BUILD_DIR)/boot.bin of=tools/boot.img bs=512 count=1 conv=notrunc
	sudo mount tools/boot.img ./disk -t vfat -o loop
	sudo cp $(BUILD_DIR)/loader.bin ./disk
	sudo cp $(BUILD_DIR)/kernel.bin ./disk
	sudo sync
	sudo umount ./disk

bochs:clean compile link disk
	@read -p "请输入平台类型(Intel/AMD): " platform; \
	case "$$platform" in \
	 "AMD"|"amd"|"a" ) echo "Hello,AMD"; \
	           bochs -f tools/bochsrc_AMD;; \
	 "Intel"|"intel"|"i" ) echo "Hello,Intel"; \
	           bochs -f tools/bochsrc;; \
	 * ) echo "无效的输入";; \
	esac


qemu:clean compile link disk
	echo $(CFLAGS)
	qemu-system-x86_64 \
	-m 4G \
	-rtc base=localtime \
	-boot c \
	-drive file=./tools/boot.img,readonly=off,format=raw  \
	-cpu Haswell -smp cores=1,threads=2 \
	-machine acpi=off \
	-vga std \
	-serial stdio \
	-net none \
	-d int \
	-display gtk,gl=on \
	-s -S \
	-name "ToyOS Development Platform for x86_64"

default: compile link
	@echo "构建成功"
	@echo $(MAKE_VERSION)
