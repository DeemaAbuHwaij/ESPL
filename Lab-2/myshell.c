#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include <signal.h>
#include "LineParser.h"
#include <fcntl.h>
#include<sys/stat.h>

int debugMode = 0;

void handle_signal(int signalReceived) {
    char* signalName = strsignal(signalReceived);
    fprintf(stderr, "Signal %s received:\n", signalName);
    if (signalReceived == SIGTSTP) {
        signal(SIGTSTP, SIG_DFL);
        raise(SIGTSTP);
    }
    else if (signalReceived == SIGINT) {
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }
    else if (signalReceived == SIGCONT) {
        signal(SIGTSTP, handle_signal);
    }
}

void execute(cmdLine *pCmdLine) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        if (debugMode) {
            fprintf(stderr, "PID: %d\nExecuting command: %s\n", getpid(), pCmdLine->arguments[0]);
        }
        if (pCmdLine->inputRedirect != NULL) {
            int fd = open(pCmdLine->inputRedirect, O_RDONLY);
            if (fd == -1) {
                perror("open input file error");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (pCmdLine->outputRedirect != NULL) {
            int fd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd == -1) {
                perror("open output file error");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
}
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execvp failed");
            _exit(EXIT_FAILURE);
        }
    }
    if (pCmdLine->blocking) {
        int mode;
        waitpid(pid, &mode, 0);
    }
}

void change_cwd(cmdLine *pCmdLine) {
    if (chdir(pCmdLine->arguments[1]) == -1) {
        perror("chdir failed");
    }
}

void suspend_implement(int pid) {
    if (kill(pid, SIGTSTP) == -1) {
        perror("kill failed");
    }
}

void wake_implement(int pid) {
    if (kill(pid, SIGCONT) == -1) {
        perror("kill failed");
    }
}

void kill_implement(int pid) {
    if (kill(pid, SIGKILL) == -1) {
        perror("kill failed");
    }
}

int main(int argc, char **argv) {
    char input[2048];
    char cwd[PATH_MAX];
    cmdLine *line;
    int quit = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            debugMode = 1;
        }
    }

    signal(SIGTSTP, handle_signal);
    signal(SIGINT, handle_signal);

    while (!quit) {
        getcwd(cwd, PATH_MAX);
        printf("%s$ ", cwd);
        fgets(input, 2048, stdin);
        line = parseCmdLines(input);
        if (strcmp(line->arguments[0], "quit") == 0) {
            quit = 1;
        }
        else if (strcmp(line->arguments[0], "cd") == 0) {
            change_cwd(line);
        }
        else if (strcmp(line->arguments[0], "suspend") == 0) {
            pid_t pid = atoi(line->arguments[1]);
            kill(pid, SIGTSTP);
        }
        else if (strcmp(line->arguments[0], "wake") == 0) {
            pid_t pid = atoi(line->arguments[1]);
            kill(pid, SIGCONT);
        }
        else if (strcmp(line->arguments[0], "kill") == 0) {
            pid_t pid = atoi(line->arguments[1]);
            kill(pid, SIGTERM);
        }
        else {
            execute(line);
        }
        freeCmdLines(line);
    }
    return 0;
}