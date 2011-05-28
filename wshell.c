/*
 *  wshell.c
 *  wShell
 *
 *  Created by windtw on 5/26/11.
 *  Copyright 2011 Light wind. All rights reserved.
 *
 */

#include "wsystools.h"

#define SHELLTITLE "[wShell]"
#define SHELLINFO "wShell v1105\t-\tCreated by windtw 2011."
#define MAXLINELEN 1024
#define MAXARGS 128

typedef struct cmdNode {
	int argc;
	char *argv[MAXARGS];
	struct cmdNode *next;
} CmdNode;

typedef struct cmdList {
	int size;
	CmdNode *head, *end;
} CmdList;

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

void get_cwd(char *path, char *home, int homelen, char *result);
int eval(char *cmdline);
int builtin_command(char *command);
void child_handler(int sig);
void interupt_handler(int sig);

int parseLine(char *buf, CmdList *cmdlist);
void remove_blank(char **buf);

void init_jobs();
int add_job(pid_t pid, pid_t pgid, int is_bg);
int remove_job(pid_t pid);
void list_jobs();

void prepare_job(sigset_t *mask);
pid_t execute_job(CmdNode *node, sigset_t *mask);
pid_t execute_job_with_dup(CmdNode *node, sigset_t *mask, int pfd[]);
void execute_jobs(CmdList *list, sigset_t *mask);

void init_cmd();
void add_cmd(CmdList *list, char *buf);
int check_bg(CmdNode *node);
void clean_cmd(CmdList *list);

int check_file(char *filename);
int absExecve(const char *filename, char *const argv[], char *const envp[]);

void show_help();

/* Global variables */
char *homePath = NULL, *searchPath = NULL;
JobList *joblist;

pid_t running_pid = 0;

int main (int argc, const char * argv[]) 
{
	char cmdline[MAXLINELEN];
	char cwdPath[MAXPATHLEN];
	char *cmdPath = NULL, *homePath = NULL;
	int homePathLen;
	
	printf("%s\nInitializing...\n\n", SHELLINFO);
	
	Signal(SIGCHLD, child_handler);
	Signal(SIGINT, interupt_handler);
	init_jobs();
	
	Getenv(&homePath, "HOME");
	Getenv(&searchPath, "PATH");
	homePathLen = strlen(homePath);
	
	while(1)
	{
		get_cwd(cmdPath, homePath, homePathLen, cwdPath);
		printf("%s: %s # ", SHELLTITLE, cwdPath);
				
		fgets(cmdline, MAXLINELEN, stdin);
		if (feof(stdin) || eval(cmdline) == -1)
			break;
	}
	
	if (cmdPath != NULL)
		free(cmdPath);

    return 0;
}

void get_cwd(char *path, char *home, int homelen, char *result)
{
	char *substr;
	
	Getcwd(&path);
	substr = strstr(path, home);
	
	if (substr != NULL) {
		strcpy(result, "~\0");
		strcat(result, (char *)(path + homelen));
	}
	else 
		strcpy(result, path);
}

int eval(char *cmdline)
{
	//char *argv[MAXARGS];
	CmdList *cmdList = malloc(sizeof(CmdList));
	char buf[MAXLINELEN];
	pid_t pid;
	int is_bg, result, jobid;
	sigset_t mask;

	strcpy(buf, cmdline);
	is_bg = parseLine(buf, cmdList);
	
	if (cmdList->size == 0) {
		clean_cmd(cmdList);
		return 0;
	}
	
	if ((result = builtin_command(cmdList->head->argv[0])) == 0) 
	{
		prepare_job(&mask);
		
		if (cmdList->size > 1) {  /* pipe */
			execute_jobs(cmdList, &mask);
		} else {
			pid = execute_job(cmdList->head, &mask);
			jobid = add_job(pid, pid, is_bg);
		}
		
		if (!is_bg) {
			int status;
			running_pid = pid;
			if ((pid = waitpid(pid, &status, 0)) > 0) {
				remove_job(pid);
				running_pid = 0;
			}
			else {
				unix_error("waitFg: waitpid error");
			}

/*			if (waitpid(pid, &status, 0) < 0)
				unix_error("waitFg: waitpid error");*/
		} else
			printf("[%d] %s", jobid, cmdline);
		
		Sigprocmask(SIG_UNBLOCK, &mask, NULL);
			
	} 
	else if (result < 0) { /* quit command */
		clean_cmd(cmdList);
		return -1;
	}
	clean_cmd(cmdList);
	return 0;
}

int builtin_command(char *command) 
{
	if (!strcmp(command, "quit"))
		return -1;
	if (!strcmp(command, "&"))
		return 1;
	if (!strcmp(command, "jobs"))
	{
		list_jobs();
		return 1;
	}
	if (!strcmp(command, "help"))
	{
		show_help();
		return 1;
	}
	
	
	return 0;
}

void remove_blank(char **buf)
{
	while (**buf && **buf == ' ')
		++(*buf);
}

int parseLine(char *buf, CmdList *cmdlist)
{
	char *delim;
	init_cmd(cmdlist);
	
	buf[strlen(buf)-1] = ' ';
	remove_blank(&buf);
	if (*buf == '\0') {
		return 0;
	}
	
	if ((delim = strchr(buf, '|'))) {
		do 
		{
			*delim = '\0';
			add_cmd(cmdlist, buf);
			buf = delim + 1;
			remove_blank(&buf);
			
			if( (delim = strchr(buf, '|')) == NULL)
				add_cmd(cmdlist, buf);
		} while (((delim = strchr(buf, '|'))));
	} else 
		add_cmd(cmdlist, buf);

	return check_bg(cmdlist->end);
}

void child_handler(int sig)
{
	pid_t pid;
	
	while ((pid = waitpid(-1, NULL, 0)) > 0){
		printf("[%d]+ %s\n", remove_job(pid), strsignal(sig));
	}
	
	if (errno != ECHILD)
		unix_error("waitpid error");
}

void interupt_handler(int sig)
{
	if (running_pid)
		Kill(running_pid, SIGINT);

	return;
}

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

void init_cmd(CmdList *list)
{
	list->size = 0;
	list->head = list->end = NULL;
}

void add_cmd(CmdList *list, char *buf)
{
	char *delim;
	
	CmdNode *node = malloc(sizeof(CmdNode));
	node->next = NULL;
	
	node->argc = 0;
	while ((delim = strchr(buf, ' '))) 
	{
		node->argv[node->argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		remove_blank(&buf);
	}
	
	node->argv[node->argc] = NULL;
	
	if (list->size) {
		list->end->next = node;
		list->end = node;
	}
	else
		list->head = list->end = node;

	list->size += 1;
}

int check_bg(CmdNode *node)
{
	if(*node->argv[node->argc-1] == '&') {
		node->argv[--(node->argc)] = NULL;
		return 1;
	} else
		return 0;
}

void clean_cmd(CmdList *list)
{
	CmdNode *curr = list->head;
	CmdNode *next;
	while (curr != NULL) 
	{
		next = curr->next;
		free(curr);
		curr = next;
	}
	free(list);
}

void prepare_job(sigset_t *mask)
{
	Sigemptyset(mask);
	Sigaddset(mask, SIGCHLD);
	Sigprocmask(SIG_BLOCK, mask, NULL);			
}

void execute_jobs(CmdList *list, sigset_t *mask)
{
	CmdNode *curr = list->head;
	pid_t pid;	
	int pfd[2], old_pfd[2];
	while (curr != NULL) 
	{
		if (curr->next != NULL) /* prepare pipe for next command */
			pipe(pfd);
		if ((pid = Fork()) == 0) {
			Sigprocmask(SIG_UNBLOCK, mask, NULL);
			if (curr != list->head) {
				dup2(old_pfd[0], STDIN_FILENO);
				close(old_pfd[0]);
				close(old_pfd[1]);
			}
			if (curr != list->end) {
				close(pfd[0]);
				dup2(pfd[1], STDOUT_FILENO);
				close(pfd[1]);
			}
			
			if (execve(curr->argv[0], curr->argv, environ) < 0) {
				printf("%s: Command not found.\n", curr->argv[0]);
				exit(0);
			}
		}
		
		if (curr != list->head) {
			close(old_pfd[0]);
			close(old_pfd[1]);
		}
		if (curr != list->end) {
			old_pfd[0] = pfd[0];
			old_pfd[1] = pfd[1];
		}
		curr = curr->next;
	}
	close(old_pfd[0]);
	close(old_pfd[1]);
}

pid_t execute_job(CmdNode *node, sigset_t *mask)
{
	pid_t pid;
	if ((pid = Fork()) == 0) {
		Sigprocmask(SIG_UNBLOCK, mask, NULL);
		if (absExecve(node->argv[0], node->argv, environ) < 0) {
			printf("%s: Command not found.\n", node->argv[0]);
			exit(0);
		}
	}
	return pid;
}

int check_file(char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp) {
		fclose(fp);
		return 1;
	} else {
		return 0;
	}
}

int absExecve(const char *filename, char *const argv[], char *const envp[])
{
	char *search = malloc(strlen(searchPath) * sizeof(char));
	strcpy(search, searchPath);
	char realpath[MAXPATHLEN];
	
	char *token = strtok(search, ":");
	while (token != NULL) {
		strcpy(realpath, token);
		strcat(realpath, "/");
		strcat(realpath, filename);
		
		if(check_file(realpath) != 0)
			break;
		
		token = strtok(NULL, ":");
	}
	
	if (token == NULL) 
	{
		if (check_file((char *)filename)) {
			strcpy(realpath, filename);
		} else {
			return -1;
		}
	}
	
	if (execve(realpath, argv, envp) < 0) 
		unix_error("absExecve error");
	
	return 0;
}

void show_help()
{
	printf("%s\n", SHELLINFO);
	printf("%8s - %s\n", "jobs", "Show jobs which is running");
	printf("%8s - %s\n", "help", "Display this help");
	printf("%8s - %s\n", "quit", "Leave the shell");
}
