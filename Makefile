SHELL = /bin/sh
BUILD_DIR_BOOT 	 = build/boot
BUILD_DIR_KERNEL = build/kernel
BUILD_DIR_USER   = build/user
MAKE = make
NASM = nasm
AS = as
CC = gcc
LD = ld
AR = ar
RM = rm -rf



LIB      = -I kernel -I lib -I device -I task -I fs -I task -I posix -I user
CFLAGS   = -g -nostdlib -mcmodel=large -march=x86-64 -fno-builtin -m64 -fno-stack-protector -w -fno-pic -fno-pie -fdiagnostics-color=always $(LIB)
LDFLAGS  = -b elf64-x86-64 -z muldefs -T kernel/kernel.lds
OBJS     =   	$(BUILD_DIR_KERNEL)/main.o 	$(BUILD_DIR_KERNEL)/printk.o  \
		$(BUILD_DIR_KERNEL)/init.o      $(BUILD_DIR_KERNEL)/screen.o  \
		$(BUILD_DIR_KERNEL)/string.o 	$(BUILD_DIR_KERNEL)/trap.o    \
		$(BUILD_DIR_KERNEL)/entry.o    	$(BUILD_DIR_KERNEL)/memory.o  \
		$(BUILD_DIR_KERNEL)/interrupt.o $(BUILD_DIR_KERNEL)/cpu.o     \
		$(BUILD_DIR_KERNEL)/task.o  	$(BUILD_DIR_KERNEL)/APIC.o    \
		$(BUILD_DIR_KERNEL)/keyboard.o 	$(BUILD_DIR_KERNEL)/mouse.o   \
		$(BUILD_DIR_KERNEL)/disk.o      $(BUILD_DIR_KERNEL)/SMP.o     \
		$(BUILD_DIR_KERNEL)/APU_boot.o 	$(BUILD_DIR_KERNEL)/time.o    \
		$(BUILD_DIR_KERNEL)/HPET.o     	$(BUILD_DIR_KERNEL)/softirq.o \
		$(BUILD_DIR_KERNEL)/timer.o     $(BUILD_DIR_KERNEL)/schedule.o \
		$(BUILD_DIR_KERNEL)/fat32.o 	$(BUILD_DIR_KERNEL)/log.o     \
		$(BUILD_DIR_KERNEL)/VFS.o      	$(BUILD_DIR_KERNEL)/head.o    \
		$(BUILD_DIR_KERNEL)/syscall.o   $(BUILD_DIR_KERNEL)/sys.o


USER_OBJS = $(BUILD_DIR_USER)/stdio.o


PIC := PIC_APIC

.PHONY: check clean build disk bochs qemu default

.DEFAULT_GOAL := default

compile: $(OBJS)

link: compile
	@$(LD) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR_KERNEL)/system

copy: link
	@objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $(BUILD_DIR_KERNEL)/system $(BUILD_DIR_KERNEL)/kernel.bin



#bootloader程序
$(BUILD_DIR_BOOT)/boot.bin: boot/boot.asm
	$(NASM) $< -o $@

$(BUILD_DIR_BOOT)/loader.bin: boot/loader.asm
	$(NASM) $< -o $@



#内核层程序
$(BUILD_DIR_KERNEL)/head.o: kernel/head.S kernel/linkage.h
	@gcc -E kernel/head.S > $(BUILD_DIR_KERNEL)/head.s
	@$(AS)  $(BUILD_DIR_KERNEL)/head.s -o $@ -a=$(BUILD_DIR_KERNEL)/1.lst

$(BUILD_DIR_KERNEL)/entry.o: kernel/entry.S
	@gcc -E kernel/entry.S > $(BUILD_DIR_KERNEL)/entry.s
	@$(AS)  $(BUILD_DIR_KERNEL)/entry.s -o $@

$(BUILD_DIR_KERNEL)/APU_boot.o: kernel/APU_boot.S
	@gcc -E kernel/APU_boot.S > $(BUILD_DIR_KERNEL)/APU_boot.s
	@$(AS)  $(BUILD_DIR_KERNEL)/APU_boot.s -o $@




$(BUILD_DIR_KERNEL)/main.o: kernel/main.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/printk.o: lib/printk.c
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/screen.o: device/screen.c
	@@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/init.o:kernel/init.c
	@$(CC) $(CFLAGS) -c $< -o $@ -D$(PIC)

$(BUILD_DIR_KERNEL)/trap.o:kernel/trap.c kernel/trap.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/memory.o:kernel/memory.c kernel/memory.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/interrupt.o:kernel/interrupt.c kernel/interrupt.h
	@$(CC) $(CFLAGS) -c $< -o $@ -D$(PIC)

$(BUILD_DIR_KERNEL)/cpu.o:kernel/cpu.c kernel/cpu.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/task.o:task/task.c task/task.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/schedule.o:task/schedule.c task/schedule.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/string.o:lib/string.c lib/string.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/APIC.o:device/APIC.c device/APIC.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/keyboard.o:device/keyboard.c device/keyboard.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/mouse.o:device/mouse.c device/mouse.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/disk.o:device/disk.c device/disk.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/time.o:device/time.c device/time.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/timer.o:device/timer.c device/timer.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/HPET.o:device/HPET.c device/HPET.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/SMP.o:kernel/SMP.c kernel/SMP.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/softirq.o:kernel/softirq.c kernel/softirq.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/fat32.o:fs/fat32.c fs/fat32.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/log.o:lib/log.c lib/log.h
	@$(CC) $(CFLAGS) -c $< -o $@


$(BUILD_DIR_KERNEL)/VFS.o:fs/VFS.c fs/VFS.h
	@$(CC) $(CFLAGS) -c $< -o $@


$(BUILD_DIR_KERNEL)/syscall.o:user/syscall.c user/syscall.h
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_KERNEL)/sys.o:user/sys.c
	@$(CC) $(CFLAGS) -c $< -o $@



#应用层程序
$(BUILD_DIR_USER)/stdio.o:posix/stdio.c posix/stdio.h
	@$(CC) $(CFLAGS) -c $< -o $@




check:
	@if command -v bochs &> /dev/null; then \
		echo "bochs is exist!"; \
	else \
		echo "bochs is not exist!"; \
		echo "Please visit https://github.com/hehellooedas/ToyOS/releases/tag/Kernel to download bochs"; \
	fi


clean:
	mkdir -p build $(BUILD_DIR_BOOT) $(BUILD_DIR_KERNEL) $(BUILD_DIR_USER)
	$(RM) $(RMFLAGS) kernel/head.s tools/boot.img $(BUILD_DIR_KERNEL)/* tools/*.lock


disk: copy $(BUILD_DIR_BOOT)/loader.bin $(BUILD_DIR_BOOT)/boot.bin
	cp ./disk/hard.img tools/hd60M.img
	cp ./disk/hard.img tools/boot.img
	dd if=$(BUILD_DIR_BOOT)/boot.bin of=tools/boot.img bs=512 count=1 conv=notrunc
	sudo mount tools/boot.img ./disk -t vfat -o loop
	sudo cp $(BUILD_DIR_BOOT)/loader.bin ./disk
	sudo cp $(BUILD_DIR_KERNEL)/kernel.bin ./disk
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
