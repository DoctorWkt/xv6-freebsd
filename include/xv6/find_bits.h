#pragma once

unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size, unsigned long offset);

#define BITS_PER_LONG 32

#define BITOP_WORD(nr) ((nr) / BITS_PER_LONG)

/**
 * __ffs - find first set bit in word
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long __ffs(unsigned long word)
{
  asm("rep; bsf %1,%0"
      : "=r" (word)
      : "rm" (word));
  return word;
}

/**
 * ffz - find first zero bit in word
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
static inline unsigned long ffz(unsigned long word)
{
  asm("rep; bsf %1,%0"
      : "=r" (word)
      : "r" (~word));
  return word;
}
