#pragma once

#include <stddef.h>

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#define TPS 1000   // ticks-per-second
#define SCHED_INTERVAL (TPS/100)  // See kern/trap.c

/*
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 */
#define container_of(ptr, type, member) ({                      \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
            (type *)( (char *)__mptr - offsetof(type,member) );})

/**
 * ilog2 - log of base 2 of 32-bit or a 64-bit unsigned value
 * @n - parameter
 *
 * constant-capable log of base 2 calculation
 * - this can be used to initialise global variables from constant data, hence
 *   the massive ternary operator construction
 *
 * selects the appropriately-sized optimised version depending on sizeof(n)
 */
#define ilog2(n)                     \
(                                    \
  __builtin_constant_p(n) ? (        \
    (n) < 1 ? ____ilog2_NaN() :      \
    (n) & (1ULL << 63) ? 63 :        \
    (n) & (1ULL << 62) ? 62 :        \
    (n) & (1ULL << 61) ? 61 :        \
    (n) & (1ULL << 60) ? 60 :        \
    (n) & (1ULL << 59) ? 59 :        \
    (n) & (1ULL << 58) ? 58 :        \
    (n) & (1ULL << 57) ? 57 :        \
    (n) & (1ULL << 56) ? 56 :        \
    (n) & (1ULL << 55) ? 55 :        \
    (n) & (1ULL << 54) ? 54 :        \
    (n) & (1ULL << 53) ? 53 :        \
    (n) & (1ULL << 52) ? 52 :        \
    (n) & (1ULL << 51) ? 51 :        \
    (n) & (1ULL << 50) ? 50 :        \
    (n) & (1ULL << 49) ? 49 :        \
    (n) & (1ULL << 48) ? 48 :        \
    (n) & (1ULL << 47) ? 47 :        \
    (n) & (1ULL << 46) ? 46 :        \
    (n) & (1ULL << 45) ? 45 :        \
    (n) & (1ULL << 44) ? 44 :        \
    (n) & (1ULL << 43) ? 43 :        \
    (n) & (1ULL << 42) ? 42 :        \
    (n) & (1ULL << 41) ? 41 :        \
    (n) & (1ULL << 40) ? 40 :        \
    (n) & (1ULL << 39) ? 39 :        \
    (n) & (1ULL << 38) ? 38 :        \
    (n) & (1ULL << 37) ? 37 :        \
    (n) & (1ULL << 36) ? 36 :        \
    (n) & (1ULL << 35) ? 35 :        \
    (n) & (1ULL << 34) ? 34 :        \
    (n) & (1ULL << 33) ? 33 :        \
    (n) & (1ULL << 32) ? 32 :        \
    (n) & (1ULL << 31) ? 31 :        \
    (n) & (1ULL << 30) ? 30 :        \
    (n) & (1ULL << 29) ? 29 :        \
    (n) & (1ULL << 28) ? 28 :        \
    (n) & (1ULL << 27) ? 27 :        \
    (n) & (1ULL << 26) ? 26 :        \
    (n) & (1ULL << 25) ? 25 :        \
    (n) & (1ULL << 24) ? 24 :        \
    (n) & (1ULL << 23) ? 23 :        \
    (n) & (1ULL << 22) ? 22 :        \
    (n) & (1ULL << 21) ? 21 :        \
    (n) & (1ULL << 20) ? 20 :        \
    (n) & (1ULL << 19) ? 19 :        \
    (n) & (1ULL << 18) ? 18 :        \
    (n) & (1ULL << 17) ? 17 :        \
    (n) & (1ULL << 16) ? 16 :        \
    (n) & (1ULL << 15) ? 15 :        \
    (n) & (1ULL << 14) ? 14 :        \
    (n) & (1ULL << 13) ? 13 :        \
    (n) & (1ULL << 12) ? 12 :        \
    (n) & (1ULL << 11) ? 11 :        \
    (n) & (1ULL << 10) ? 10 :        \
    (n) & (1ULL <<  9) ?  9 :        \
    (n) & (1ULL <<  8) ?  8 :        \
    (n) & (1ULL <<  7) ?  7 :        \
    (n) & (1ULL <<  6) ?  6 :        \
    (n) & (1ULL <<  5) ?  5 :        \
    (n) & (1ULL <<  4) ?  4 :        \
    (n) & (1ULL <<  3) ?  3 :        \
    (n) & (1ULL <<  2) ?  2 :        \
    (n) & (1ULL <<  1) ?  1 :        \
    (n) & (1ULL <<  0) ?  0 :        \
    ____ilog2_NaN()                  \
                       ) :           \
  (sizeof(n) <= 4) ?                 \
  __ilog2_u32(n) :                   \
  ____ilog2_NaN()                    \
)

static inline int
____ilog2_NaN()
{
  //panic("ilog2 invalid number");
  return 0;
}

/**
 * fls - find last set bit in word
 * @x: the word to search
 *
 * This is defined in a similar way as the libc and compiler builtin
 * ffs, but returns the position of the most significant set bit.
 *
 * fls(value) returns 0 if value is 0 or the position of the last
 * set bit if value is nonzero. The last (most significant) bit is
 * at position 32.
 */
static inline int fls(int x)
{
  int r;

  asm("bsrl %1,%0\n\t"
      "jnz 1f\n\t"
      "movl $-1,%0\n"
      "1:" : "=r" (r) : "rm" (x));
  return r + 1;
}

static inline __attribute__((const)) int
__ilog2_u32(uint32 n)
{
  return fls(n) - 1;
}

static inline int
test_bit(long nr, volatile const unsigned long *addr)
{
  int oldbit;

  asm volatile("bt %2,%1\n\t"
    "sbb %0,%0"
    : "=r" (oldbit)
    : "m" (*(unsigned long *)addr), "Ir" (nr));

  return oldbit;
}

#define __GEN_RMWcc(fullop, var, cc, ...)                         \
do {                                                              \
  char c;                                                         \
  asm volatile (fullop "; set" cc " %1"                           \
                  : "+m" (var), "=qm" (c)                         \
                  : __VA_ARGS__ : "memory");                      \
  return c != 0;                                                  \
} while (0)

#define GEN_BINARY_RMWcc(op, var, vcon, val, arg0, cc)            \
  __GEN_RMWcc(op " %2, " arg0, var, cc, vcon (val))

/**
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static inline int test_and_set_bit(long nr, volatile unsigned long *addr)
{
  GEN_BINARY_RMWcc("lock; bts", *addr, "Ir", nr, "%0", "c");
}

/**
 * test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static inline int test_and_clear_bit(long nr, volatile unsigned long *addr)
{
  GEN_BINARY_RMWcc("lock; btr", *addr, "Ir", nr, "%0", "c");
}

