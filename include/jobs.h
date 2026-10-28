#ifndef JOBS_H
#define JOBS_H
#include <sys/types.h>

void init_shell_terminal(void);
void add_job(pid_t pid, char **args, int state);
void display_jobs(void);
void reap_jobs(void);
void wait_for_job(pid_t pid, char **args);
void do_fg(char **args);
void do_bg(char **args);
void do_kill(char **args);

#endif
