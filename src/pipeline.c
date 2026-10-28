#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pipeline.h"
#include "redirection.h"
#include "jobs.h"

#define MAX_CMDS 20
#define MAX_ARGS 64

int is_pipeline(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) return 1;
    }
    return 0;
}

void execute_pipeline(char **args, int is_bg) {
    char *cmds[MAX_CMDS][MAX_ARGS];
    int num_cmds = 0, j = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            cmds[num_cmds][j] = NULL;
            num_cmds++;
            j = 0;
        } else {
            cmds[num_cmds][j++] = args[i];
        }
    }
    cmds[num_cmds][j] = NULL;
    num_cmds++; 

    int num_pipes = num_cmds - 1;
    int pipefds[2 * num_pipes];

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe failed");
            return;
        }
    }

    int pids[MAX_CMDS];
    pid_t pgid = 0; // Process group ID for the pipeline

    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();

        if (pids[i] == 0) { 
            // CHILD: Restore normal signal behavior so Ctrl+C/Z work
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);

            if (i > 0) dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            if (i < num_cmds - 1) dup2(pipefds[i * 2 + 1], STDOUT_FILENO);

            for (int k = 0; k < 2 * num_pipes; k++) close(pipefds[k]);

            if (handle_redirection(cmds[i]) == -1) exit(1);

            if (execvp(cmds[i][0], cmds[i]) == -1) {
                printf("Command not found: %s\n", cmds[i][0]);
                exit(1);
            }
        } else if (pids[i] < 0) {
            perror("fork failed");
            return;
        } else {
            // PARENT: Group all children in the pipeline into the same process group
            if (i == 0) pgid = pids[0]; 
            setpgid(pids[i], pgid);
        }
    }

    for (int i = 0; i < 2 * num_pipes; i++) close(pipefds[i]);

    if (is_bg) {
        // FIX: Passed '1' (STATE_RUNNING). We track the whole pipeline via its Process Group ID
        add_job(pgid, args, 1);
    } else {
        // Give terminal to the pipeline
        tcsetpgrp(STDIN_FILENO, pgid);
        
        int status;
        for (int i = 0; i < num_cmds; i++) {
            waitpid(pids[i], &status, WUNTRACED);
        }
        
        // Shell reclaims terminal
        tcsetpgrp(STDIN_FILENO, getpid());

        // If it was stopped via Ctrl+Z, add it to our jobs list as Stopped (2)
        if (WIFSTOPPED(status)) {
            add_job(pgid, args, 2);
        }
    }
}
