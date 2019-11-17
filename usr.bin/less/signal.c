/*
 * Routines dealing with signals.
 *
 * A signal usually merely causes a bit to be set in the "signals" word.
 * At some convenient time, the mainline code checks to see if any
 * signals need processing by calling psignal().
 * An exception is made if we are reading from the keyboard when the
 * signal is received.  Some operating systems will simply call the
 * signal handler and NOT return from the read (with EINTR).
 * To handle this case, we service the interrupt directly from
 * the handler if we are reading from the keyboard.
 */

#include "less.h"
#include <signal.h>

/*
 * The type of signal handler functions.
 * Usually int, although it should be void.
 */
typedef	int		HANDLER;

/*
 * "sigs" contains bits indicating signals which need to be processed.
 */
public int sigs;
#define	S_INTERRUPT	01
#ifdef SIGTSTP
#define	S_STOP		02
#endif

extern int reading;
extern char *first_cmd;


/*
 * Expand a filename, substituting any environment variables, etc.
 * The implementation of this is necessarily very operating system
 * dependent.  This implementation is unabashedly only for Unix systems.
 */
char *glob(filename)
	char *filename;
{
	return (filename);
}
