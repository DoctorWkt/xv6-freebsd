/* Job control. If UCBJOB is defined, we use BSD4.x job control. If POSIXJOB
 * is defined, we use POSIX job control. IF V7JOB is defined, we provide
 * job control using ptrace(). If none are defined, we simulate job control
 * as best we can.
 *
 * $Revision: 1.1 $ $Date: 2016/07/22 12:03:23 $
 */

#include "header.h"

/* We define the wait return status macros for SysV. We also define
 * WIFCORE, which evaluates as true if WIFSIGNALED is TRUE and
 * a core has been dumped. This only works under SysV and BSD.
 * Also defined are WRUNFG and WRUNBG, which are true is the
 * process is running in the fore or background.
 */
#define RUNFG	-1		/* Status if running in fg */
#define RUNBG	-2		/* Status if running in bg */

#define WAIT_T int
#define STATUS status
#ifdef UCBJOB
# undef  WAIT_T
# undef  STATUS
# define WAIT_T union wait
# define STATUS status.w_status
#endif


/* The job structure hold the information needed to manipulate the
 * jobs, using job numbers instead of pids. Note that the linked
 * list used by Wish is ordered by jobnumber.
 */
struct job
{
  int jobnumber;		/* The job number */
  int pid;			/* The pid of the job */
  char *name;			/* Job's argv[0]; */
  char *dir;			/* The job's working directory */
  WAIT_T status;		/* Job's status */
  bool changed;			/* Changed since last CLE? */
  struct job *next;		/* Pointer to next job */
};

int Exitstatus = 0;		/* The exit status of the last child */

#ifdef PROTO
# define P(s) s
#else
# define P(s) ()
#endif

#undef P

#ifdef POSIXJOB
# include "posixjob.c"
#endif

#ifdef UCBJOB
# include "ucbjob.c"
#endif

#ifdef V7JOB
# include "v7job.c"
#endif



/* Add the pid and it's argv[0] to the job list. Return the allocated
 * job number.
 */
int
addjob(pid, name, isbg)
  int pid, isbg;
  char *name;
{
  return (0);
}


/* Print out the list of current jobs and their status.
 * Note although this is a builtin, it is called from
 * main with an argc value of 0, to show new jobs only.
 */
int
joblist(argc, argv)
  int argc;
  char *argv[];

{
  return(0);
}

int
Kill(argc, argv)
  int argc;
  char *argv[];

{
  return(0);
}

void
waitfor(pid)
  int pid;
{
  int status;
  int wpid;
  do {
    wpid = wait(&status);
  } while (wpid != pid);
}


#if defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB)
/* Builtins */
int
bg(argc, argv)
  int argc;
  char *argv[];

{
  return (0);
}


/* Fg is a special builtin. Instead of returning an exitstatus, it returns
 * the pid of the fg'd process. Builtin() knows about this.
 */
int
fg(argc, argv)
  int argc;
  char *argv[];

{
    return (0);
}
#endif /* defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB) */
