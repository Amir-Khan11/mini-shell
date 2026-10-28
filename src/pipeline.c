#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pipeline.h"
#include "redirection.h"

#define MAX_CMDS 20
#define MAX_ARGS 64

// Returns 1 if there is a pipe in the arguments, 0 otherwise
int is_pipeline(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            return 1;
        }
    }
    return 0;
}

void execute_pipeline(char **args)
{
    char *cmds[MAX_CMDS][MAX_ARGS];
    int num_cmds = 0;
    int j = 0;

    // 1. Split the arguments by "|" into separate command arrays
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            cmds[num_cmds][j] = NULL;
            num_cmds++;
            j = 0;
        }
        else
        {
            cmds[num_cmds][j++] = args[i];
        }
    }
    cmds[num_cmds][j] = NULL;
    num_cmds++; // Account for the last command

    int num_pipes = num_cmds - 1;
    int pipefds[2 * num_pipes];

    // 2. Create all necessary pipes
    for (int i = 0; i < num_pipes; i++)
    {
        if (pipe(pipefds + i * 2) < 0)
        {
            perror("pipe failed");
            return;
        }
    }

    int pids[MAX_CMDS];

    // 3. Fork and execute each command
    for (int i = 0; i < num_cmds; i++)
    {
        pids[i] = fork();

        if (pids[i] == 0)
        { // --- CHILD PROCESS ---
            // If not the first command, grab input from the previous pipe
            if (i > 0)
            {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            // If not the last command, push output to the current pipe
            if (i < num_cmds - 1)
            {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            // Close all pipe file descriptors in the child
            // (The child only needs the STDIN/STDOUT we just dup2'd)
            for (int k = 0; k < 2 * num_pipes; k++)
            {
                close(pipefds[k]);
            }

            // Allow redirection even within a pipeline step (e.g., cat < file.txt | grep a)
            if (handle_redirection(cmds[i]) == -1)
            {
                exit(1);
            }

            if (execvp(cmds[i][0], cmds[i]) == -1)
            {
                printf("Command not found: %s\n", cmds[i][0]);
                exit(1);
            }
        }
        else if (pids[i] < 0)
        {
            perror("fork failed");
            return;
        }
    }

    // --- PARENT PROCESS ---
    // Close all pipes in the parent so children know when to stop reading
    for (int i = 0; i < 2 * num_pipes; i++)
    {
        close(pipefds[i]);
    }

    // Wait for ALL children in the pipeline to finish
    for (int i = 0; i < num_cmds; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
}