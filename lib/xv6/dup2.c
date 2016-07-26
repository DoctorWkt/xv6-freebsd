#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int dup2(int oldd, int newd)
{
        int i = 0, fd, tmp;
        int fdbuf[_POSIX_OPEN_MAX];

        /* ignore the error on the close() */
        tmp = errno; (void) close(newd); errno = tmp;
        while ((fd = dup(oldd)) != newd) {
                if (fd == -1) break;
                fdbuf[i++] = fd;
        }
        tmp = errno;
        while (--i >= 0) {
                close(fdbuf[i]);
        }
        errno = tmp;
        return -(fd == -1);
}
