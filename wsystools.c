/*
 *  wsystools.c
 *  wShell
 *
 *  Created by windtw on 5/26/11.
 *
 */

#include "wsystools.h"

/* Error-handling functions */
void unix_error(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}

/* Unix System command */
void Getcwd(char **ptr)
{
	if ((*ptr = getcwd(NULL, 0)) == NULL)
		unix_error("Getcwd error");
}

void Getenv(char **ptr, const char *name)
{
	if ((*ptr = getenv(name)) == NULL)
		unix_error("Getenv error");
}

/* Wrappers for Process control */
pid_t Fork(void)
{
	pid_t pid;
	
	if ((pid = fork()) < 0)
		unix_error("Fork error");
	return pid;
}

void Execve(const char *filename, char *const argv[], char *const envp[])
{
	if (execve(filename, argv, envp) < 0) 
		unix_error("Execve error");
}

pid_t Wait(int *status)
{
	pid_t pid;
	if ((pid = wait(status)) < 0) 
		unix_error("Wait error");
	return pid;
}

pid_t Waitpid(pid_t pid, int *iptr, int options)
{
	pid_t retpid;
	
	if((retpid = waitpid(pid, iptr, options) < 0))
		unix_error("Waitpid error");
	return retpid;
}

void Kill(pid_t pid, int signum)
{
	int rc;
	
	if((rc = kill(pid, signum)) < 0)
		unix_error("Kill error");
}

unsigned int Sleep(unsigned int secs)
{
	unsigned int rc;
	
	if ((rc = sleep(secs) < 0)) 
		unix_error("Sleep error");
	return rc;
}

unsigned int Alarm(unsigned int secs)
{
	return alarm(secs);
}

void Setpgid(pid_t pid, pid_t pgid)
{
	int rc;
	
	if((rc = setpgid(pid, pgid)) < 0)
		unix_error("Setpgid error");
}

/* Wrapper for Unix signal */

handler_t *Signal(int signum, handler_t *handler) 
{
	struct sigaction action, prev_action;
	
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
	
    if (sigaction(signum, &action, &prev_action) < 0)
		unix_error("Signal error");
    return (prev_action.sa_handler);
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (sigprocmask(how, set, oldset) < 0)
		unix_error("Sigprocmask error");
    return;
}

void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0)
		unix_error("Sigemptyset error");
    return;
}

void Sigfillset(sigset_t *set)
{ 
    if (sigfillset(set) < 0)
		unix_error("Sigfillset error");
    return;
}

void Sigaddset(sigset_t *set, int signum)
{
    if (sigaddset(set, signum) < 0)
		unix_error("Sigaddset error");
    return;
}

void Sigdelset(sigset_t *set, int signum)
{
    if (sigdelset(set, signum) < 0)
		unix_error("Sigdelset error");
    return;
}

int Sigismember(const sigset_t *set, int signum)
{
    int rc;
    if ((rc = sigismember(set, signum)) < 0)
		unix_error("Sigismember error");
    return rc;
}


