#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "execute.h"
#include "redirection.h"

void execute_command(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child Process: setup redirection first
        if (handle_redirection(args) == -1) {
            exit(1);
        }

        // Then execute the command
        if (execvp(args[0], args) == -1) {
            printf("Command not found: %s\n", args[0]);
        }
        exit(1); 
    } 
    else if (pid > 0) {
        // Parent Process
        int status;
        waitpid(pid, &status, 0);
    } 
    else {
        perror("fork failed");
    }
}
