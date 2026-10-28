#ifndef PIPELINE_H
#define PIPELINE_H
int is_pipeline(char **args);
void execute_pipeline(char **args, int is_bg); // Added is_bg
#endif