#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

int main() {
	void *y, *x = sbrk(0);
	DIR *dir = opendir(".");
	if (!dir) return 5;

	y = sbrk(0);
	if (x != y) return 1;

	while (readdir(dir)) {
		y = sbrk(0);
		if (x != y) return 1;
	}

	malloc(1);
	y = sbrk(0);
	if (y != x) return 1;

	return 0;
}
