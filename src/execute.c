#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "execute.h"
#include "redirection.h"
#include "jobs.h"

void execute_command(char **args, int is_bg) {
    pid_t pid = fork();

    if (pid == 0) {
        setpgid(0, 0); // Put child in its own process group

        // Restore normal signal behavior for the child
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);

        if (handle_redirection(args) == -1) exit(1);
        if (execvp(args[0], args) == -1) printf("Command not found: %s\n", args[0]);
        exit(1); 
    } else if (pid > 0) {
        setpgid(pid, pid); // Prevent race conditions
        if (is_bg) {
            add_job(pid, args, 1); // 1 = RUNNING
        } else {
            tcsetpgrp(STDIN_FILENO, pid); // Give terminal to child
            wait_for_job(pid, args);
        }
    } else perror("fork failed");
}
