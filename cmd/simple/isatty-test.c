#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	// stdin
	if (isatty(0))
		printf("stdin is a tty\n");
	else
		printf("stdin is not a tty\n");

	exit(0);
}
