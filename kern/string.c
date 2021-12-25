#include <xv6/types.h>
#include <xv6/x86.h>

void*
memset(void *dst, int c, uint n)
{
  if ((int)dst%4 == 0 && n%4 == 0){
    c &= 0xFF;
    stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
  } else
    stosb(dst, c, n);
  return dst;
}

int
memcmp(const void *v1, const void *v2, uint n)
{
  const uchar *s1, *s2;
  
  s1 = v1;
  s2 = v2;
  while(n-- > 0){
    if(*s1 != *s2)
      return *s1 - *s2;
    s1++, s2++;
  }

  return 0;
}

void*
memmove(void *dst, const void *src, uint n)
{
  const char *s;
  char *d;

  s = src;
  d = dst;
  if(s < d && s + n > d){
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void*
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

int
strncmp(const char *p, const char *q, uint n)
{
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  if(n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}

char*
strncpy(char *s, const char *t, int n)
{
  char *os;
  
  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char*
safestrcpy(char *s, const char *t, int n)
{
  char *os;
  
  os = s;
  if(n <= 0)
    return os;
  while(--n > 0 && (*s++ = *t++) != 0)
    ;
  *s = 0;
  return os;
}

int
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

int
strcmp(const char *p, const char *q)
{
  int gtrlen = strlen(p);
  int qlen = strlen(q);

  if (qlen > gtrlen) gtrlen = qlen;

  return strncmp(p, q, gtrlen);
}

void
itoa(int xx, char *buf)
{
  char tbuf[16];
  static char digits[] = "0123456789abcdef";
  int i, j;
  int x = xx;

  i = 0;
  do{
    tbuf[i++] = digits[x % 10];
  }while((x /= 10) != 0);

  i--;
  for (j = 0; i >= 0; i--, j++)
    buf[j] = tbuf[i];
}

void
strconcat(char* r, const char* a, const char* b)
{
  int asize = strlen(a), bsize = strlen(b);
  int i, j;

  strncpy(r, a, asize);

  for (i = asize, j = 0; j < bsize; j++) {
   r[i] = b[j];
  }

  r[asize + bsize] = 0;
}

void *
memscan(void *addr, int c, int size)
{
  if (!size)
    return addr;
  asm volatile("repnz; scasb\n\t"
      "jnz 1f\n\t"
      "dec %%edi\n"
      "1:"
      : "=D" (addr), "=c" (size)
      : "0" (addr), "1" (size), "a" (c)
      : "memory");
  return addr;
}

