#include <xv6/pcspkr.h>

#include <xv6/x86.h>

void beep(int value) {
  unsigned int count = 0;

  if (value == 0) {
    beep_off();
    return;
  }

  if (value > 20 && value < 32767)
    count = 1193182ul / value;
  
  outb(0x43, 0xb6);
  // Select desired HZ
  outb(0x42, count & 0xff);
  outb(0x42, (count >> 8) & 0xff);
  // Enable counter 2
  outb(0x61, inb(0x61) | 3);
}

void beep_off(void) { outb(0x61, inb(0x61) & 0xFC); }
