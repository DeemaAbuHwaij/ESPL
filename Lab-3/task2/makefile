all: main 


start:
	nasm -f elf32 start.s -o start.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 util.o start.o -o start

main:
	nasm -g -f elf32 start.s -o start.o
	gcc -g -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -g -m32 -Wall -ansi -c -nostdlib -fno-stack-protector task2.c -o main.o
	ld -g -m elf_i386 start.o main.o util.o -o main

.PHONY: clean
clean:
	rm -f *.o main 