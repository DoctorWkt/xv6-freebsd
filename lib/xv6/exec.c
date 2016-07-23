#include <unistd.h>
#include <string.h>

char **environ=NULL;

// If the exec path doesn't stat with a /, append it to "/bin"
char *_execfullpath="/bin/                                                                          ";

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
  const char *f= path;
  if (f[0] != '/') { strcpy(&(_execfullpath[5]), f); f=_execfullpath; }
  return(exec(f, argv));
}

int execvp(const char *file, char *const argv[])
{
  const char *f= file;
  if (f[0] != '/') { strcpy(&(_execfullpath[5]), f); f=_execfullpath; }
  return(exec(f, argv));
}

int execvpe(const char *file, char *const argv[], char *const envp[])
{
  const char *f= file;
  if (f[0] != '/') { strcpy(&(_execfullpath[5]), f); f=_execfullpath; }
  return(exec(f, argv));
}
