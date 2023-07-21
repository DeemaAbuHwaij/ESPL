#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int file_descriptor[2];
    pid_t child1, child2;
    if (pipe(file_descriptor) == -1) {     // Create a pipe
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "(parent_process>forking...)\n");  
    child1 = fork(); // Fork a first child process(child1)
    if (child1 == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if (child1 == 0) {
        fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);
        close(STDOUT_FILENO);         // Close the standard output
        if (dup(file_descriptor[1]) == -1) {         // Duplicate the write-end of the pipe using dup
            perror("dup");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n"); 
        close(file_descriptor[1]); // Close the file descriptor that was duplicated
        fprintf(stderr, "(going to execute cmd: ...)\n"); 
        char *cmd[] = {"ls", "-l", NULL}; // Execute "ls -l"
        execvp(cmd[0], cmd);
        exit(EXIT_FAILURE);
    } 
    else {// Parent process
        fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");  
        close(file_descriptor[1]);
        fprintf(stderr, "(parent_process>forking...)\n");  
        child2 = fork();         
        fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);
        if (child2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (child2 == 0) {// child2 process
            close(STDIN_FILENO); // Close the standard input
            if (dup(file_descriptor[0]) == -1) { // Duplicate the read-end of the pipe using dup
                perror("dup");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n"); // Debug message
            close(file_descriptor[0]); // Close the file descriptor that was duplicated
            fprintf(stderr, "(going to execute cmd: ...)\n"); // Debug message
            char *cmd[] = {"tail", "-n", "2", NULL}; // Execute "tail -n 2"
            execvp(cmd[0], cmd);
            exit(EXIT_FAILURE);
        } 
        else {// Parent process
            fprintf(stderr, "(closing the read end of the pipe)\n"); // Debug message
            close(file_descriptor[0]); // Close the read end of the pipe
            waitpid(child1, NULL, 0); // Wait for child processes to terminate, in the same order of their execution
            waitpid(child2, NULL, 0);
            fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n"); // Debug message
            fprintf(stderr, "(parent_process>exiting...)\n"); 
            exit(EXIT_SUCCESS);
        }
    }
}