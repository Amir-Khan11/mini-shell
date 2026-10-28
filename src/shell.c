#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "parser.h"
#include "builtin.h"
#include "execute.h"
#include "pipeline.h"
#include "jobs.h"

#define MAX_ARGS 64

void shell_run(void) {
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t chars_read;
    char *args[MAX_ARGS];

    init_shell_terminal(); // Initialize terminal control and block signals

    while (1) {
        reap_jobs();

        printf("myshell> ");
        fflush(stdout);

        chars_read = getline(&line, &bufsize, stdin);
        if (chars_read == -1) {
            printf("\n"); break;
        }

        parse_input(line, args);
        if (args[0] == NULL) continue;

        int is_bg = 0, i = 0;
        while (args[i] != NULL) i++;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            is_bg = 1;
            args[i - 1] = NULL; 
        }

        if (is_pipeline(args)) execute_pipeline(args, is_bg);
        else if (is_builtin(args)) execute_builtin(args);
        else execute_command(args, is_bg);
    }
    free(line);
}
