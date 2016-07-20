/*
 * popen - open a pipe
 */

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<limits.h>
#include	<errno.h>
#include	<signal.h>
#include	<stdio.h>

typedef int wait_arg;

static int pids[_POSIX_OPEN_MAX];

FILE *
popen(const char *command, const char *type)
{
	int piped[2];
	int Xtype = *type == 'r' ? 0 : *type == 'w' ? 1 : 2;
	int pid;

	if (Xtype == 2 ||
	    pipe(piped) < 0 ||
	    (pid = fork()) < 0) return 0;
	
	if (pid == 0) {
		/* child */
		int *p;

		for (p = pids; p < &pids[_POSIX_OPEN_MAX]; p++) {
			if (*p) close((int)(p - pids));
		}
		close(piped[Xtype]);
		dup2(piped[!Xtype], !Xtype);
		close(piped[!Xtype]);
		execl("/bin/sh", "sh", "-c", command, (char *) 0);
		exit(127);	/* like system() ??? */
	}

	pids[piped[Xtype]] = pid;
	close(piped[!Xtype]);
	return fdopen(piped[Xtype], type);
}

int
pclose(stream)
FILE *stream;
{
	int ret_val = 0;
	int fd = fileno(stream);
	wait_arg status;
	int wret;

	fclose(stream);
	while ((wret = wait(&status)) != -1) {
		if (wret == pids[fd]) break;
	}
	if (wret == -1) ret_val = -1;
	pids[fd] = 0;
	return ret_val;
}
