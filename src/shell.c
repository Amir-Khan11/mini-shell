#include <stdio.h>
#include <stdlib.h>
#include "shell.h"
#include "parser.h"
#include "builtin.h"
#include "execute.h"

#define MAX_ARGS 64

void shell_run(void) {
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t chars_read;
    char *args[MAX_ARGS];

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        chars_read = getline(&line, &bufsize, stdin);
        if (chars_read == -1) {
            printf("\n");
            break;
        }

        parse_input(line, args);

        if (args[0] == NULL) continue;

        if (is_builtin(args)) {
            execute_builtin(args);
        } else {
            execute_command(args);
        }
    }
    free(line);
}
