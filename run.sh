rm -rf tools/boot.img build/*
cp ./a.img tools/boot.img
nasm boot/boot.asm -o build/boot.bin
nasm boot/loader.asm -o build/loader.bin
gcc  -x c -mcmodel=large -fno-builtin -m64 -I ./lib -I ./kernel -c kernel/main.c -o build/main.o
gcc -mcmodel=large -fno-builtin -m64 -I ./lib -I ./kernel -c lib/printk.c -o build/printk.o
gcc -mcmodel=large -fno-builtin -m64 -I ./lib -I ./kernel -c device/screen.c -o build/screen.o
gcc -mcmodel=large -fno-builtin -m64 -I ./lib -I ./kernel -c kernel/init.c -o build/init.o
gcc -E kernel/head.S > kernel/head.s
as --64 kernel/head.s -o build/head.o -a=build/1.lst
ld -b elf64-x86-64 -z muldefs -o build/system build/head.o build/main.o build/printk.o build/screen.o build/init.o -T kernel/kernel.lds 
objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary build/system build/kernel.bin
dd if=build/boot.bin of=tools/boot.img bs=512 count=1 conv=notrunc
sudo mount tools/boot.img /media/tf -t vfat -o loop
sudo cp build/loader.bin /media/tf
sudo cp build/kernel.bin /media/tf
sudo sync
sudo umount /media/tf
bochs/bin/bochs -f tools/bochsrc
