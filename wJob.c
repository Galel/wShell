/*
 *  wJob.c
 *  wShell
 *
 *  Created by windtw on 5/28/11.
 *  Copyright 2011 Light wind. All rights reserved.
 *
 */

#include "wJob.h"

void init_jobs()
{
	joblist = malloc(sizeof(JobList));
	joblist->size = 0;
}

void list_jobs()
{
	JobNode *curr = joblist->head;
	while (curr) {
		printf("[%d] %d\n", curr->jobid, curr->pid);
		curr = curr->next;
	}
}

int add_job(pid_t pid, pid_t pgid, int is_bg)
{	
	JobNode *node = malloc(sizeof(JobNode));
	node->pid = pid;
	node->pgid = pgid;
	node->is_bg = is_bg;
	node->next = NULL;
	setpgid(pid, pgid);
	
	if (joblist->size) {
		node->jobid = joblist->end->jobid;
		joblist->end->next = node;
		joblist->end = node;
	} else {
		joblist->head = joblist->end = node;
		node->jobid = 1;
	}
	
	joblist->size += 1;
	
	return node->jobid;
}

int remove_job(pid_t pid)
{
	int jobid;
	JobNode *curr = joblist->head;
	JobNode *prev = NULL;
	while (curr) {
		if (curr->pid == pid) {
			if (curr == joblist->head)
				joblist->head = curr->next;
			else if (prev)
				prev->next = curr->next;
			jobid = curr->jobid;
			free(curr);
			joblist->size -= 1;
			break;
		}
		prev = curr;
		curr = curr->next;
	}
	return jobid;
}
