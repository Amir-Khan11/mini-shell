# Mini OS Shell with Job Control

**Course:** Operating Systems (CSC-251L)
**Author:** Amir Khan

## Overview
This is a custom, lightweight, Unix-like shell implemented in C for a university Operating Systems semester project. It supports executing external commands, built-in commands, file redirection, pipelines, background execution, and interactive job control.

## Supported Features
1. External Commands: Uses fork(), execvp(), and waitpid().
2. Built-ins: cd, pwd, history, jobs, fg, bg, kill, and exit.
3. Redirection: Input (<), Output (>), and Append (>>).
4. Pipelines: Multi-command pipelines (cmd1 | cmd2 | cmd3).
5. Background Execution: Appending & to run jobs in the background.
6. Job Control: Full support for Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP) using process groups and terminal control.
7. History: Tracks and displays up to the last 100 commands.

## Compilation and Execution
The project uses a clean, modular architecture and includes a Makefile.

To compile the shell: make
To run the shell interactively: ./myshell
To clean compiled objects: make clean

## Known Limitations
* Quoting: The basic parser uses strtok on spaces/tabs. It does not parse quote marks.
* Environment Variables: Variable expansion is not supported.
