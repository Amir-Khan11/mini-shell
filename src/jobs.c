#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jobs.h"

#define MAX_JOBS 100
#define STATE_RUNNING 1
#define STATE_STOPPED 2

typedef struct {
    int id;
    pid_t pid;
    int state;
    char cmd[256];
} Job;

Job jobs[MAX_JOBS];
int next_job_id = 1;
pid_t shell_pgid;

void init_shell_terminal(void) {
    shell_pgid = getpid();
    setpgid(shell_pgid, shell_pgid);
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    // Shell ignores interactive signals so Ctrl+C/Z doesn't kill the shell
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
}

void add_job(pid_t pid, char **args, int state) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].id = next_job_id++;
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].cmd[0] = '\0';
            for (int j = 0; args[j] != NULL; j++) {
                strcat(jobs[i].cmd, args[j]); strcat(jobs[i].cmd, " ");
            }
            if (state == STATE_RUNNING) printf("[%d] %d\n", jobs[i].id, pid);
            else printf("\n[%d] Stopped\t\t%s\n", jobs[i].id, jobs[i].cmd);
            return;
        }
    }
    printf("Error: Background jobs limit reached.\n");
}

void remove_job(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].pid = 0;
            int active = 0;
            for (int j = 0; j < MAX_JOBS; j++) if (jobs[j].pid != 0) active = 1;
            if (!active) next_job_id = 1;
            return;
        }
    }
}

void display_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] %s\t\t%s\n", jobs[i].id, 
                   (jobs[i].state == STATE_RUNNING) ? "Running" : "Stopped", jobs[i].cmd);
        }
    }
}

void reap_jobs(void) {
    pid_t pid;
    int status;
    // WUNTRACED and WCONTINUED let us detect stopped/resumed jobs
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].pid == pid) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    printf("\n[%d] Done\t\t%s\n", jobs[i].id, jobs[i].cmd);
                    remove_job(pid);
                } else if (WIFSTOPPED(status)) {
                    jobs[i].state = STATE_STOPPED;
                    printf("\n[%d] Stopped\t\t%s\n", jobs[i].id, jobs[i].cmd);
                } else if (WIFCONTINUED(status)) {
                    jobs[i].state = STATE_RUNNING;
                    printf("\n[%d] Running\t\t%s\n", jobs[i].id, jobs[i].cmd);
                }
                break;
            }
        }
    }
}

void wait_for_job(pid_t pid, char **args) {
    int status;
    waitpid(pid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shell_pgid); // Shell reclaims terminal control
    if (WIFSTOPPED(status)) add_job(pid, args, STATE_STOPPED);
}

Job* find_job(int id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != 0 && jobs[i].id == id) return &jobs[i];
    }
    return NULL;
}

void do_fg(char **args) {
    if (!args[1]) { printf("fg: missing job id\n"); return; }
    Job *j = find_job(atoi(args[1]));
    if (!j) { printf("fg: no such job\n"); return; }

    printf("%s\n", j->cmd);
    tcsetpgrp(STDIN_FILENO, j->pid); // Give terminal to the job
    
    if (j->state == STATE_STOPPED) kill(-j->pid, SIGCONT); // Wake it up
    
    int status;
    waitpid(j->pid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shell_pgid); // Shell reclaims terminal

    if (WIFSTOPPED(status)) j->state = STATE_STOPPED;
    else remove_job(j->pid);
}

void do_bg(char **args) {
    if (!args[1]) { printf("bg: missing job id\n"); return; }
    Job *j = find_job(atoi(args[1]));
    if (!j) { printf("bg: no such job\n"); return; }

    if (j->state == STATE_STOPPED) {
        j->state = STATE_RUNNING;
        printf("[%d] %s\n", j->id, j->cmd);
        kill(-j->pid, SIGCONT);
    }
}

void do_kill(char **args) {
    if (!args[1]) { printf("kill: missing job id\n"); return; }
    Job *j = find_job(atoi(args[1]));
    if (!j) { printf("kill: no such job\n"); return; }
    kill(-j->pid, SIGTERM);
    printf("[%d] Terminated\t%s\n", j->id, j->cmd);
}
