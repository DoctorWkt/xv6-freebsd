/*
 * High level routines dealing with the output to the screen.
 */

#include "less.h"

public int errmsgs;	/* Count of messages displayed by error() */

extern int sigs;
extern int sc_width, sc_height;
extern int ul_width, ue_width;
extern int so_width, se_width;
extern int bo_width, be_width;
extern int tabstop;
extern int twiddle;
extern int any_display;
extern char *line;
extern char *first_cmd;

/*
 * Display the line which is in the line buffer.
 */
	public void
put_line()
{
	register char *p;
	register int c;
	register int column;
	extern int auto_wrap, ignaw;

	if (sigs)
		/*
		 * Don't output if a signal is pending.
		 */
		return;

	if (line == NULL)
		line = (twiddle) ? "~" : "";

	column = 0;
	for (p = line;  *p != '\0';  p++)
	{
		switch (c = *p)
		{
		case UL_CHAR:
			ul_enter();
			column += ul_width;
			break;
		case UE_CHAR:
			ul_exit();
			column += ue_width;
			break;
		case BO_CHAR:
			bo_enter();
			column += bo_width;
			break;
		case BE_CHAR:
			bo_exit();
			column += be_width;
			break;
		case '\t':
			do
			{
				putc(' ');
				column++;
			} while ((column % tabstop) != 0);
			break;
		case '\b':
			putbs();
			column--;
			break;
		default:
			if (c & 0200)
			{
				putc('^');
				putc(c & 0177);
				column += 2;
			} else
			{
				putc(c);
				column++;
			}
		}
	}
	if (column < sc_width || !auto_wrap || ignaw)
		putc('\n');
}

/*
 * Is a given character a "control" character?
 * {{ ASCII DEPENDENT }}
 */
	public int
control_char(c)
	int c;
{
	return (c < ' ' || c == '\177');
}

/*
 * Return the printable character used to identify a control character
 * (printed after a carat; e.g. '\3' => "^C").
 * {{ ASCII DEPENDENT }}
 */
	public int
carat_char(c)
	int c;
{
	return ((c == '\177') ? '?' : (c | 0100));
}


static char obuf[1024];
static char *ob = obuf;

/*
 * Flush buffered output.
 */
	public void
flush()
{
	write(1, obuf, ob-obuf);
	ob = obuf;
}

/*
 * Discard buffered output.
 */
	public void
dropout()
{
	ob = obuf;
}

/*
 * Output a character.
 */
	public void
putc(c)
	int c;
{
	if (ob >= &obuf[sizeof(obuf)])
		flush();
	*ob++ = c;
}

#if 0
/*
 * Output a string.
 */
	public void
puts(s)
	register char *s;
{
	while (*s != '\0')
		putc(*s++);
}
#endif

/*
 * Output a message in the lower left corner of the screen
 * and wait for carriage return.
 */

static char return_to_continue[] = "  (press RETURN)";

	public void
error(s)
	char *s;
{
	register int c;
	static char buf[2];

	errmsgs++;
	if (!any_display)
	{
		/*
		 * Nothing has been displayed yet.
		 * Output this message on error output (file
		 * descriptor 2) and don't wait for a keystroke
		 * to continue.
		 *
		 * This has the desirable effect of producing all
		 * error messages on error output if standard output
		 * is directed to a file.  It also does the same if
		 * we never produce any real output; for example, if
		 * the input file(s) cannot be opened.  If we do
		 * eventually produce output, code in edit() makes
		 * sure these messages can be seen before they are
		 * overwritten or scrolled away.
		 */
		write(2, s, strlen(s));
		write(2, "\n", 1);
		return;
	}

	lower_left();
	clear_eol();
	so_enter();
	puts(s);
	puts(return_to_continue);
	so_exit();

#if ONLY_RETURN
	while ((c = getc()) != '\n' && c != '\r')
		bell();
#else
	c = getc();
	if (c != '\n' && c != '\r' && c != ' ')
	{
		buf[0] = c;
		first_cmd = buf;
	}
#endif

	if (strlen(s) + sizeof(return_to_continue) + 
		so_width + se_width + 1 > sc_width)
		/*
		 * Printing the message has probably scrolled the screen.
		 * {{ Unless the terminal doesn't have auto margins,
		 *    in which case we just hammered on the right margin. }}
		 */
		repaint();
}

#ifdef notdef
	public int
error_width()
{
	/*
	 * Don't use the last position, because some terminals
	 * will scroll if you write in the last char of the last line.
	 */
	return (sc_width - 
		(sizeof(return_to_continue) + so_width + se_width + 1));
}
#endif
