# byteOS
<p>Independent OS in C++. Also try Linux!</p>

MIT/byteOS is an independent operating system written by one person (me), it currently has working keyboard, a command handling function (currently one command called neofetch) and a bootloader in ASM

Here are the compilation instructions:

BOOTLOADER

`i686-elf-as boot.s -o boot.o`

KERNEL

`gcc -m32 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -c kernel.cpp -o kernel.o`

LINKING

`ld -m elf_i386 -T linker.ld -o byte.bin boot.o kernel.o`

BOOTING (QEMU

`qemu-system-i386 -kernel kernel.bin`


The current version is version 0.0.1 (codename and kernel name nebula)


![byteos 0.0.1](https://github.com/user-attachments/assets/07575e66-cabf-484f-bd87-fd1e56d4600c)


