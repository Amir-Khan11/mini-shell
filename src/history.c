#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

#define MAX_HISTORY 100

char *history[MAX_HISTORY];
int history_count = 0;

void add_history(const char *line) {
    // Don't add empty lines
    if (line == NULL || line[0] == '\n' || line[0] == '\0') return;

    // Create a copy of the line to store
    char *clean_line = strdup(line);
    
    // Remove the trailing newline for cleaner output
    clean_line[strcspn(clean_line, "\n")] = 0;

    if (history_count < MAX_HISTORY) {
        history[history_count++] = clean_line;
    } else {
        // Shift everything left by 1 to make room at the end
        free(history[0]);
        for (int i = 1; i < MAX_HISTORY; i++) {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY - 1] = clean_line;
    }
}

void print_history(void) {
    for (int i = 0; i < history_count; i++) {
        printf(" %4d  %s\n", i + 1, history[i]);
    }
}
