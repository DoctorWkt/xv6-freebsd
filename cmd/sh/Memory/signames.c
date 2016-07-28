#include <unistd.h>
#include <signal.h>

#if _NSIG > 255
#error to big signal set
#endif

unsigned int fmt_oct(char *x, unsigned char c) {
	x[3] = '0' + (c & 7); c >>= 3;
	x[2] = '0' + (c & 7); c >>= 3;
	x[1] = '0' + (c & 7);  
	x[0] = '\\';
	return 4;
}

unsigned int fmt_str(char *x, const char *s) {
	char *tmp = x;
	while (*s) { *tmp = *s; tmp++; s++; }
	return tmp - x;
}

#define P(a, b) if (x[SIG##a] == 0) { \
	p += fmt_str(p, "/* "); \
	p += fmt_str(p, #a); \
	p += fmt_str(p, " */\t\""); \
	p += fmt_oct(p, SIG##a); \
	len = fmt_str(p+4, b) + 3; \
	fmt_oct(p, len); \
	p += ++len; \
	p += fmt_str(p, "\\000\" \\\n"); \
} x[SIG##a] = 1

static char x[2048];

int main() {
	unsigned int len;
	char buf[8192], *p = buf;
	p += fmt_str(p, "/* this file is auto generated.  do not edit! */\n"
				 "#define signal_NAMES \\\n");

#ifdef SIGTERM
	P(TERM,"Terminated");
#endif
	
	P(HUP,"Hungup");
	P(INT,"Interrupt");
	P(QUIT,"Quit");
	
#ifdef SIGKILL
	P(KILL,"Killed");
#endif
	
#ifdef SIGALRM
	P(ALRM,"Alarm call");
#endif
	
#ifdef SIGPIPE
	P(PIPE,"Broken pipe");
#endif
	
#ifdef SIGILL
	P(ILL,"Illegal instruction");
#endif

#ifdef SIGTRAP
	P(TRAP,"Trace/BPT trap");
#endif

#ifdef SIGABRT
	P(ABRT,"Abort");
#endif
	
#ifdef SIGBUS
	P(BUS,"Bus error");
#endif

#ifdef SIGEMT
	P(EMT,"EMT trap");
#endif

#ifdef SIGIOT
	P(IOT,"IOT trap");
#endif

#ifdef SIGFPE
	P(FPE,"Floating exception");
#endif

#ifdef SIGSEGV
	P(SEGV,"Memory fault");
#endif

#if 0 && defined(SIGUSR1)
	P(USR1,"User1 call");
#endif

#if 0 && defined(SIGUSR2)
	P(USR2,"User2 call");
#endif

#ifdef SIGSYS
	P(SYS,"Bad system call");
#endif

	p += fmt_str(p, "\t\t\"\\000\\000Signal \"\n");

	write(1, buf, p-buf);
	fsync(1);
	close(1);
	return 0;
}


#if 0
STRING	sysmsg[] = {
	0,
	"Hangup",
	0,	/* Interrupt */
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	0,	/* Broken pipe */
	"Alarm call",
	"Terminated",
	"Signal 16",
};
#endif
