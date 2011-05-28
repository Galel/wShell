/*
 *  wJob.h
 *  wShell
 *
 *  Created by windtw on 5/28/11.
 *  Copyright 2011 Light wind. All rights reserved.
 *
 */

#ifndef WJOB_H
#define WJOB_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

typedef struct jobNode {
	int jobid;
	pid_t pid;
	pid_t pgid;
	int is_bg;
	struct jobNode *next;
} JobNode;

typedef struct jobList {
	int size;
	JobNode *head, *end;
} JobList;

extern JobList *joblist;

void init_jobs();
int add_job(pid_t pid, pid_t pgid, int is_bg);
int remove_job(pid_t pid);
void list_jobs();

#endif