// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include <xv6/types.h>
#include <xv6/defs.h>
#include <xv6/param.h>
#include <xv6/traps.h>
#include <xv6/spinlock.h>
#include <xv6/fs.h>
#include <xv6/file.h>
#include <xv6/memlayout.h>
#include <xv6/mmu.h>
#include <xv6/proc.h>
#include <xv6/x86.h>
#include <xv6/termios.h>
#include <xv6/ioctl.h>

static void consputc(int, int);

static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
  struct termios termios[2];	// 0 is /dev/serial, 1 is /dev/console
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i], 1);
}
//PAGEBREAK: 50

// Print to /dev/console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c, 1);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s, 1);
      break;
    case '%':
      consputc('%', 1);
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%', 1);
      consputc(c, 1);
      break;
    }
  }

  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];
  
  cli();
  cons.locking = 0;
  cprintf("cpu%d: panic: ", cpu->id);
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

#define C(x)  ((x)-'@')  // Control-x

static void
cgaputc(int c)
{
  int pos;
  
  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    crt[pos++] = (c&0xff) | 0x0700;  // black on white

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");
  
  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }
  
  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

void
consputc(int c, int minor)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(minor==0) {
    if(c == BACKSPACE){
      uartputc('\b'); uartputc(' '); uartputc('\b');
    } else
      uartputc(c);
  } else
    cgaputc(c);
}

void
consechoc(int c, int minor)
{
  if(c != C('D') && cons.termios[minor].c_lflag & ECHO)
    consputc(c, minor);
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input[2];	// 0 is /dev/serial, 1 is /dev/console

void
consoleintr(int (*getc)(void), int minor)
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
   if(cons.termios[minor].c_lflag & ICANON){
    switch(c){
    case C('P'):  // Process listing.
      doprocdump = 1;   // procdump() locks cons.lock indirectly; invoke later
      break;
    case C('U'):  // Kill line.
      while(input[minor].e != input[minor].w &&
            input[minor].buf[(input[minor].e-1) % INPUT_BUF] != '\n'){
        input[minor].e--;
	consechoc(BACKSPACE, minor);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input[minor].e != input[minor].w){
        input[minor].e--;
	consechoc(BACKSPACE, minor);
      }
      break;
    default:
      if(c != 0 && input[minor].e-input[minor].r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input[minor].buf[input[minor].e++ % INPUT_BUF] = c;
        consechoc(c, minor);
        if(c == '\n' || c == C('D') || input[minor].e == input[minor].r+INPUT_BUF){
          input[minor].w = input[minor].e;
          wakeup(&input[minor].r);
        }
      }
      break;
    }
   }
else {		// Not canonical input
      if(c != 0 && input[minor].e-input[minor].r < INPUT_BUF){
        input[minor].buf[input[minor].e++ % INPUT_BUF] = c;
        consechoc(c, minor);
        input[minor].w = input[minor].e;
        wakeup(&input[minor].r);
      }
   }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;
  short minor= ip->minor;

  // Not console, not serial
  if (minor < 0 || minor > 1) return(0);

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input[minor].r == input[minor].w){
      if(proc->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input[minor].r, &cons.lock);
    }
    c = input[minor].buf[input[minor].r++ % INPUT_BUF];
    if(c == C('D') && cons.termios[minor].c_lflag & ICANON){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input[minor].r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n' || ((cons.termios[minor].c_lflag & ICANON)==0))
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff, ip->minor);
  release(&cons.lock);
  ilock(ip);

  return n;
}

int
consoleioctl(struct inode *ip, int req)
{
  int minor= ip->minor;
  struct termios *termios_p;

  if(req != TCGETA && req != TCSETA)
    return -1;
  if(argptr(2, (void*)&termios_p, sizeof(*termios_p)) < 0)
    return -1;
  if(req == TCGETA)
    *termios_p = cons.termios[minor];
  else
    cons.termios[minor] = *termios_p;
  return(0);
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].ioctl = consoleioctl;
  cons.termios[0].c_lflag = ECHO | ICANON;
  cons.termios[1].c_lflag = ECHO | ICANON;
  cons.locking = 1;

  picenable(IRQ_KBD);
  ioapicenable(IRQ_KBD, 0);
}

