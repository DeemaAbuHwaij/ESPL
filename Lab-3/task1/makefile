all: start 
task0:
	nasm -f elf32 start.s -o start.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o
	ld -m elf_i386 start.o main.o util.o -o task0 

task0b:
	nasm -f elf32 task0b.s -o task0b.o
	ld -m elf_i386 task0b.o -o task0b

start:
	nasm -f elf32 start.s -o start.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 util.o start.o -o start

task2:
	nasm -g -f elf32 start.s -o start.o
	gcc -g -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -g -m32 -Wall -ansi -c -nostdlib -fno-stack-protector task2.c -o task2.o
	ld -g -m elf_i386 start.o task2.o util.o -o task2

.PHONY: clean
clean:
	rm -f *.o start 