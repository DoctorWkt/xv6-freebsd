#include <unistd.h>

char **environ=NULL;
extern int exec(const char *path, char *const argv[]);

int execl(const char *path, const char *arg, ...)
{
  return exec(path, (char * const *) &arg);
}
                       
int execlp(const char *file, const char *arg, ...)
{
  return exec(file, (char * const *) &arg);
}

int execle(const char *path, const char *arg, ...)
{
  return exec(path, (char * const *) &arg);
}

int execv(const char *path, char *const argv[])
{
  return(exec(path, argv));
}

int execvp(const char *file, char *const argv[])
{
  return(exec(file, argv));
}

int execvpe(const char *file, char *const argv[], char *const envp[])
{
  return(exec(file, argv));
}
