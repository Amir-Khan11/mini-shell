#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "redirection.h"

// Scans args for <, >, >> and sets up file descriptors. 
// Returns 0 on success, -1 on failure.
int handle_redirection(char **args) {
    int j = 0;
    int command_end = -1;

    while (args[j] != NULL) {
        if (strcmp(args[j], ">") == 0 || strcmp(args[j], ">>") == 0 || strcmp(args[j], "<") == 0) {
            if (command_end == -1) command_end = j;

            if (args[j+1] == NULL) {
                printf("Syntax error: expected filename after %s\n", args[j]);
                return -1;
            }

            if (strcmp(args[j], ">") == 0) {
                int fd = open(args[j+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open failed"); return -1; }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } else if (strcmp(args[j], ">>") == 0) {
                int fd = open(args[j+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd < 0) { perror("open failed"); return -1; }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } else if (strcmp(args[j], "<") == 0) {
                int fd = open(args[j+1], O_RDONLY);
                if (fd < 0) { perror("open failed"); return -1; }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            j++; 
        }
        j++;
    }

    if (command_end != -1) {
        args[command_end] = NULL; // Hide the redirection part from execvp
    }
    return 0;
}
