#include <xv6/pcspkr.h>

#include <xv6/types.h>
#include <xv6/x86.h>

void beep(int value) {
  unsigned int count = 0;

#define PIT_TICK_RATE 1193182ul
  if (value > 20 && value < 32767)
    count = PIT_TICK_RATE / value;

  outb(0x42, 0xB6);
  /* select desired HZ */
  outb(0x42, count & 0xff);
  outb(0x42, (count >> 8) & 0xff);
  /* enable counter 2 */
  outb(0x61, inb(0x61) | 3);
}

void beep_off(void) { outb(0x61, inb(0x61) & 0xFC); }
