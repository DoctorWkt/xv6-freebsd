/*
 * fgetc - get an unsigned character and return it as an int
 */

#include	<stdio.h>

int
fgetc(FILE *stream)
{
	return getc(stream);
}
