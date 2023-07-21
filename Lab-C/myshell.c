#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "linux/limits.h"
#include "LineParser.h"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#define WCONTINUED 8
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20

typedef struct process{
    cmdLine* cmd; 
    pid_t pid; 
    int status; 
    struct process *next;
} process;

struct process* process_list = NULL;
int debugMode = 0;

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* newProcess = malloc(sizeof(struct process));
    newProcess->cmd = cmd;
    newProcess->pid = pid;
    newProcess->next = NULL;
    if(cmd->blocking || cmd->next){
        newProcess->status = TERMINATED;
    } else {
        newProcess->status = RUNNING;
    }
    if(*process_list == NULL){
        *process_list = newProcess;
    } else {
        process* curr = *process_list;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newProcess;
    }
}

void freeProcessList(process* process_list){
    if(process_list != NULL){
        freeProcessList(process_list->next);
        freeCmdLines(process_list->cmd);
        free(process_list);
    }
}

void updateProcessStatus(process* process_list, int pid, int status) {
    process* curr_process = process_list;
    while (curr_process != NULL) {
        if (curr_process->pid == pid) {
            curr_process->status = status;
            break;
        }
        curr_process = curr_process->next;
    }
}

void updateProcessList(process **process_list){
    int status;
    int pid;
    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0){
        if(WIFEXITED(status) || WIFSIGNALED(status)){
            updateProcessStatus(*process_list, pid, TERMINATED);
        }
        if(WIFSTOPPED(status)){
            updateProcessStatus(*process_list, pid, SUSPENDED);
        }
        if(WIFCONTINUED(status)){
            updateProcessStatus(*process_list, pid, RUNNING);
        }
    }
}

void removeProcess(process** process_list){
    process* curr_process = *process_list;
    process* prev_process = *process_list;
    while(curr_process != NULL){
        if(curr_process->status == TERMINATED){
            if(prev_process == curr_process){
                *process_list = (*process_list)->next;
                freeCmdLines(curr_process->cmd);
                free(curr_process);
                curr_process = *process_list;
                prev_process = *process_list;
            } else {
                prev_process->next = curr_process->next;
                freeCmdLines(curr_process->cmd);
                free(curr_process);
                curr_process = prev_process->next;
            }
        } else {
            prev_process = curr_process;
            curr_process = curr_process->next;
        }
    }
}


void printProcessList(process** process_list) {
    updateProcessList(process_list);
    process* curr_process = *process_list;
    printf("PID\tCommand\t\tSTATUS\n");
    while (curr_process != NULL) {
        char* status;
        if (curr_process->status == TERMINATED) {
            status = "TERMINATED";
        } else if (curr_process->status == RUNNING) {
            status = "RUNNING";
        } else {
            status = "SUSPENDED";
        }
        printf("%d\t%s\t%s\n", curr_process->pid,curr_process->cmd->arguments[0], status);
        curr_process = curr_process->next;
    }
    removeProcess(process_list);
}


void change_cwd(cmdLine *pCmdLine) {
    if (chdir(pCmdLine->arguments[1]) == -1) {
        perror("chdir failed");
    }
}

void execute(cmdLine *pCmdLine){
    if(pCmdLine -> inputRedirect){
        fclose(stdin);
        if(fopen(pCmdLine ->inputRedirect,"r") == NULL){
            perror("Execution Failed");
            _exit(1);
        }
    }
    if(pCmdLine -> outputRedirect){
        fclose(stdout);
        if(fopen(pCmdLine ->outputRedirect,"w") == NULL){
            perror("Execution Failed");
            _exit(1);
        }
    }
    char *command = (*pCmdLine).arguments[0];
    if(execvp(command,(*pCmdLine).arguments) == -1){
        perror("Execution Failed");
        _exit(1);
    }
}

void executePipe(cmdLine *cmd){
    int pipefd[2]; 
    pid_t pid1, pid2;
    if(cmd->outputRedirect || cmd->next->inputRedirect){
        fprintf(stderr, "Invalid redirection\n");
        return;
    }
    if (pipe(pipefd) == -1) {
        perror("Failed to pipe");
        _exit(-1);
    }
    pid1 = fork();
    if (pid1 == 0) { 
        close(STDOUT_FILENO); 
        dup(pipefd[1]); 
        close(pipefd[1]); 
        if(cmd -> inputRedirect){ 
        fclose(stdin);
            if(fopen(cmd ->inputRedirect,"r") == NULL){
                perror("Execution Failed");
                _exit(1);
            }
        }
        if(execvp(cmd->arguments[0],cmd->arguments) == -1){ 
            perror("Execution Failed");
            _exit(1);
        }
    }
    else { 
        close(pipefd[1]);
        if(cmd->next !=NULL){
            pid2 = fork();
            if (pid2 == 0) { 
                close(STDIN_FILENO);
                dup(pipefd[0]);
                close(pipefd[0]);
                if(cmd->next -> outputRedirect){ 
                    fclose(stdout);
                    if(fopen(cmd->next ->outputRedirect,"w") == NULL){
                        perror("Execution Failed");
                        _exit(1);
                    }
                }
                if(execvp(cmd->next->arguments[0],cmd->next->arguments) == -1){ 
                    perror("Execution Failed");
                    _exit(1);
                }
            }
            else {
                close(pipefd[0]);
                addProcess(&process_list, cmd, pid1);
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
            }
        }
    }
}

int main(int argc, char *argv[]){
    char cwd[PATH_MAX];
    char input[2048];
    struct cmdLine *cmd;
    char history[HISTLEN][2048];
    int newest = 0;
    int oldest = 0;
    int historySize = 0;
    for(int i=0; i<argc; i++){
    if(strcmp(argv[i],"-d") == 0)
        debugMode = 1;
    }
    while(1){
        int toFree = 1;
        if(getcwd(cwd, PATH_MAX) == 0) 
            return 1;
        printf("%s$ ", cwd);
        fgets(input,2048,stdin);
        if(strncmp("quit",input,4) == 0){ 
            freeProcessList(process_list);
            exit(0);
        }
        cmd = parseCmdLines(input); 
        if(strcmp(cmd -> arguments[0],"history") == 0){
            for(int i = 0; i < historySize; i++){
                printf("%d) %s", i + 1, history[(oldest + i) % HISTLEN]);
            }
            freeCmdLines(cmd);
            continue;
        }
        else if(strcmp(cmd -> arguments[0],"!!") == 0){
            freeCmdLines(cmd);
            if(historySize < 1){
                fprintf(stderr, "History is empty!\n");
                continue;
            } else {
                cmd = parseCmdLines(history[(newest-1 + HISTLEN) % HISTLEN]);
            }
        }
        else if(cmd -> arguments[0][0] == '!'){
            int index = atoi(cmd -> arguments[0]+1) - 1;
            freeCmdLines(cmd);
            if(index >= historySize || index < 0){
                fprintf(stderr, "index %d is out of bound %d!\n", index, historySize);
                continue;
            } else {
                cmd = parseCmdLines(history[(oldest + index + HISTLEN) % HISTLEN]);
            }
        }
        else {
            strcpy(history[newest], input);
            newest = (newest + 1) % HISTLEN;
            if(historySize == HISTLEN){
                oldest = (oldest + 1) % HISTLEN;
            } else {
                historySize++;
            }
        }
        if(cmd->next !=NULL){
            executePipe(cmd);
            toFree = 0;
        }
        else if(strcmp(cmd -> arguments[0],"cd") == 0){
            change_cwd(cmd);        
        }
        else if(strcmp(cmd -> arguments[0],"wake") == 0){
            kill(atoi(cmd -> arguments[1]),SIGCONT);
        }
        else if(strcmp(cmd -> arguments[0],"suspend") == 0){
            kill(atoi(cmd -> arguments[1]),SIGTSTP);
        }
        else if(strcmp(cmd -> arguments[0],"kill") == 0) {
            kill(atoi(cmd -> arguments[1]),SIGINT);
        }
        else if(strcmp(cmd -> arguments[0],"procs") == 0) {
            printProcessList(&process_list);
        }
        else {
            toFree = 0;
            int pid = fork();
            if(pid != 0){
                if(cmd->blocking != 0){ 
                    waitpid(pid, NULL, 0);
                }
                addProcess(&process_list, cmd, pid);
            }else{ 
                execute(cmd); 
            }
            if(debugMode == 1){
                fprintf(stderr,"PID: %d\n",pid);
                fprintf(stderr,"Executing command: %s\n",cmd->arguments[0]);
            }
        }
        if(toFree)
            freeCmdLines(cmd);
    }
    return 0;
}