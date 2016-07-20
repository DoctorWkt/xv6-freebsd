#include <string.h>
#include <stdlib.h>

/*
**  Return an allocated copy of a string.
*/
char *
strdup(const char *p)
{
    char        *new;

    if ((new = malloc(strlen(p) + 1)) != NULL)
        (void)strcpy(new, p);
    return new;
}
