#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define max_FILES 1024
/* create random files with names Z_??? */

int main() {
	char buf[64], *p;
	unsigned int u;
	int k, fd = open("/dev/urandom", O_RDONLY);

	if (fd<0) return 1;
	for (k=0; k<max_FILES; k++) {
		if ((int)sizeof(u) != read(fd, &u, sizeof(u))) return 2;
		p = buf;
		*p++ = 'Z';
		*p++ = '_';
		do { *p++ = 'a' + u%16; u /= 16; } while(u);
		*p = 0;
		if (symlink("Makefile", buf))
			if (errno != EEXIST) return 3;
	}
	close(fd);
	return 0;
}
