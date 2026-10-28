#include <stdio.h>     // For printf, perror
#include <stdlib.h>    // For malloc, free, exit
#include <string.h>    // For strtok, strcmp
#include <unistd.h>    // For fork, execvp, chdir, getcwd
#include <sys/types.h> // For pid_t
#include <sys/wait.h>  // For waitpid

#define MAX_ARGS 64

int main()
{
    char *line = NULL;  // Pointer to hold the user's input
    size_t bufsize = 0; // Size of the buffer (getline will manage this)
    ssize_t chars_read; // Number of characters read

    char *args[MAX_ARGS]; // Array to hold the command and its arguments

    // The shell runs in an infinite loop until the user types "exit"
    while (1)
    {
        // 1. Print the shell prompt
        printf("myshell> ");
        fflush(stdout); // Ensure the prompt prints immediately to the screen

        // 2. Read the user's input
        // getline automatically allocates enough memory for what we type
        chars_read = getline(&line, &bufsize, stdin);

        // If the user presses Ctrl+D (End of File), chars_read is -1
        if (chars_read == -1)
        {
            printf("\n");
            break; // Exit the loop
        }

        // 3. Break the input line into separate words (tokens)
        // We use spaces, tabs, and newlines (" \t\n") as separators
        int i = 0;
        char *token = strtok(line, " \t\n");

        while (token != NULL && i < (MAX_ARGS - 1))
        {
            args[i] = token; // Store the word in our array
            i++;
            token = strtok(NULL, " \t\n"); // Get the next word
        }
        args[i] = NULL; // The array of arguments must end with NULL for execvp to work

        // If the user just pressed Enter (empty command), skip to the next loop
        if (args[0] == NULL)
        {
            continue;
        }

        // 4. Check for Built-in Commands (commands our shell handles itself)
        if (strcmp(args[0], "exit") == 0)
        {
            break; // Exit the while loop and close the shell
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (args[1] == NULL)
            {
                printf("cd: missing folder name\n");
            }
            else
            {
                // chdir changes the directory of the current program
                if (chdir(args[1]) != 0)
                {
                    perror("cd failed");
                }
            }
            continue; // Command finished, show the prompt again
        }
        else if (strcmp(args[0], "pwd") == 0)
        {
            char cwd[1024];
            // getcwd gets the current working directory path
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("%s\n", cwd);
            }
            else
            {
                perror("pwd failed");
            }
            continue; // Command finished, show the prompt again
        }

        // 5. Execute External Commands (like ls, date, whoami)
        // We must create a new process (child) to run the external command,
        // so our shell (parent) doesn't replace itself and crash.
        pid_t pid = fork();

        if (pid == 0)
        {
            // --- CHILD PROCESS ---
            // execvp replaces this child process with the new command we typed
            if (execvp(args[0], args) == -1)
            {
                // If execvp fails (e.g., we typed a command that doesn't exist), it reaches here
                printf("Command not found: %s\n", args[0]);
            }
            exit(1); // The child must exit if execvp fails
        }
        else if (pid > 0)
        {
            // --- PARENT PROCESS (Our Shell) ---
            // The parent waits for the child program to finish its job before showing the prompt again
            int status;
            waitpid(pid, &status, 0);
        }
        else
        {
            // Fork failed to create a process
            perror("fork failed");
        }
    }

    // Clean up the memory allocated by getline before exiting
    free(line);
    return 0;
}