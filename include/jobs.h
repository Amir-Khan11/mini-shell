#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

void add_job(pid_t pid, char **args);
void remove_job(pid_t pid);
void display_jobs();
void reap_jobs();

#endif