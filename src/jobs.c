#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jobs.h"

#define MAX_JOBS 100

typedef struct
{
    int id;
    pid_t pid;
    char cmd[256];
} Job;

Job jobs[MAX_JOBS];
int next_job_id = 1;

void add_job(pid_t pid, char **args)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pid == 0)
        { // Find an empty slot
            jobs[i].id = next_job_id++;
            jobs[i].pid = pid;

            // Save the command name for the 'jobs' output
            jobs[i].cmd[0] = '\0';
            for (int j = 0; args[j] != NULL; j++)
            {
                strcat(jobs[i].cmd, args[j]);
                strcat(jobs[i].cmd, " ");
            }

            printf("[%d] %d\n", jobs[i].id, pid);
            return;
        }
    }
    printf("Error: Background jobs limit reached.\n");
}

void remove_job(pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pid == pid)
        {
            jobs[i].pid = 0; // Clear the slot

            // Reset job ID counter if no jobs are left
            int active = 0;
            for (int j = 0; j < MAX_JOBS; j++)
            {
                if (jobs[j].pid != 0)
                    active = 1;
            }
            if (!active)
                next_job_id = 1;

            return;
        }
    }
}

void display_jobs()
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].pid != 0)
        {
            printf("[%d] Running\t\t%s\n", jobs[i].id, jobs[i].cmd);
        }
    }
}

// Cleans up any background processes that have finished
void reap_jobs()
{
    pid_t pid;
    int status;
    // WNOHANG tells waitpid to return immediately if no children have exited yet
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < MAX_JOBS; i++)
        {
            if (jobs[i].pid == pid)
            {
                printf("\n[%d] Done\t\t%s\n", jobs[i].id, jobs[i].cmd);
                remove_job(pid);
                break;
            }
        }
    }
}