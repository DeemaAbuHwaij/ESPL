section .data
    argc: dd 0
    argv: dd 0
    inFile: dd 0
    outFile: dd 1
    myChar: db 0
section .rodata
    new_line: db 10
    hello_world: db "Hello world", 10
    hello_world_len: equ $ - hello_world
    error_msg: db "Can't open file!", 10
    error_msg_len: equ $ - error_msg

section .text
global main
extern strlen
main:
    call get_arguments
    call print_arguments
    call encoder
    jmp exit











    mov eax, 0x4                    ;sys_write
    mov ebx, 1                      ;stdout
    mov ecx, argc            ;buffer
    mov edx, 1        ;buffer length
    int 0x80

    jmp exit













get_arguments:
    ;push argv
    ;push argc
    ;push program return address
    ;push call get_arguments return address
    mov eax, dword[esp+12]
    mov dword[argv], eax
    mov eax, dword[esp+8]
    mov dword[argc], eax
    ret

print_arguments:
    mov edi, 0
    print_loop:
        mov ecx, dword[argv]                      ;ecx = argv = char**
        mov ecx, dword[ecx + edi * 4]             ;ecx = argv[i]
    get_argv_length:
        push ecx
        call strlen
        pop ecx
    scan_argv:
        cmp word[ecx], "-i"
        je open_input
        cmp word[ecx], "-o"
        je open_output
    print_argv:
        mov edx, eax
        mov eax, 0x4                    ;sys_write
        mov ebx, 1                      ;stdout
        int 0x80
    print_new_line:
        mov eax, 0x4                    ;sys_write
        mov ebx, 1                      ;stdout
        mov ecx, new_line            ;buffer
        mov edx, 1        ;buffer length
        int 0x80
    inc edi         ;add edi, 1
    cmp edi, dword[argc]
    jne print_loop
    ret
open_input:
    pushad
    mov eax, 0x5                    ;sys_open
    mov ebx, ecx                    ;file name
    add ebx, 2
    mov ecx, 0                      ;O_READ
    int 0x80
    cmp eax, 0
    jl error
    mov dword[inFile], eax
    popad
    jmp print_argv
open_output:
    pushad
    mov eax, 0x5                    ;sys_open
    mov ebx, ecx                    ;file name
    add ebx, 2
    mov ecx, 101o                      ;O_WRONLY | O_CREAT
    mov edx, 777o                      ;RWX permissions
    int 0x80
    cmp eax, 0
    jl error
    mov dword[outFile], eax
    popad
    jmp print_argv
error:
    popad
    mov eax, 0x4                    ;sys_write
    mov ebx, 1                      ;stdout
    mov ecx, error_msg            ;buffer
    mov edx, error_msg_len        ;buffer length
    int 0x80
    jmp exit
encoder:
    get_char:
        mov eax, 0x3                    ;sys_write
        mov ebx, dword[inFile]                      ;stdin
        mov ecx, myChar            ;buffer
        mov edx, 1        ;buffer length
        int 0x80
    check_eof:
        cmp eax, 0
        jle finished_encoder
    compare_char:
        cmp byte[myChar], 'A'
        jl print_char
        cmp byte[myChar], 'z'
        jg print_char
    encode_char:
        inc byte[myChar]
    print_char:
        mov eax, 0x4                    ;sys_write
        mov ebx, dword[outFile]                      ;stdout
        mov ecx, myChar            ;buffer
        mov edx, 1        ;buffer length
        int 0x80
        jmp encoder
    finished_encoder:
        ret
exit:
    mov eax, 0x1                    ;sys_exit
    mov ebx, 0                      ;exit code
    int 0x80
