#include <string.h>
#include "parser.h"

void parse_input(char *line, char **args) {
    int i = 0;
    char *token = strtok(line, " \t\n");
    while (token != NULL && i < 63) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}
