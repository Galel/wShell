/*
 *  wsystools.h
 *  wShell
 *
 *  Created by windtw on 5/26/11.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <errno.h>
#include <signal.h>

extern char **environ; /* defined by libc */

/* function for error handling */
void unix_error(char *msg);
void Getcwd(char **ptr);
void Getenv(char **ptr, const char *name);

/* Wrappers for Process control */
pid_t Fork(void);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *iptr, int options);
void Kill(pid_t pid, int signum);
unsigned int Sleep(unsigned int secs);
unsigned int Alarm(unsigned int secs);
void Setpgid(pid_t pid, pid_t pgid);

/* Wrapper for Unix signal */
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
