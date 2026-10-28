#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "execute.h"
#include "redirection.h"
#include "jobs.h" // NEW: Include jobs.h

void execute_command(char **args, int is_bg)
{ // Added is_bg
    pid_t pid = fork();

    if (pid == 0)
    {
        if (handle_redirection(args) == -1)
            exit(1);

        if (execvp(args[0], args) == -1)
        {
            printf("Command not found: %s\n", args[0]);
        }
        exit(1);
    }
    else if (pid > 0)
    {
        if (is_bg)
        {
            // NEW: Don't wait. Add to background jobs.
            add_job(pid, args);
        }
        else
        {
            // NORMAL: Wait for foreground process
            int status;
            waitpid(pid, &status, 0);
        }
    }
    else
    {
        perror("fork failed");
    }
}