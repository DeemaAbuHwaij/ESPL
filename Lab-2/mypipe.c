#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>

int main() {
    int pipeFile[2]; 
    char buffer[10];
    
    if (pipe(pipeFile) != 0) {
        perror("ERROR!, Pipe function failed");
        _exit(1);
    } 
    pid_t pid = fork(); 
    if (pid == -1) {
        perror("ERROR!, Fork failed");
        _exit(1);
    }
    else if (pid == 0) {
        char message[] = "hello";
        close(pipeFile[0]); 
        write(pipeFile[1], message, sizeof(message)); 
    }
    else{
        close(pipeFile[1]); 
        read(pipeFile[0], buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }
}