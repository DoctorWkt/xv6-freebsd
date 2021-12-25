#include "shfeatures.h"

#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#ifndef MEM_BLOCK_SIZE
#include <sys/shm.h>
#ifdef PAGE_SIZE
#define MEM_BLOCK_SIZE PAGE_SIZE
#endif
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

#ifndef MEM_BLOCK_SIZE
#if defined(__alpha__) || defined(__sparc__)
#define MEM_BLOCK_SIZE 8192UL
#else
#define MEM_BLOCK_SIZE 4096UL
#endif
#endif

#if MEM_BLOCK_SIZE == (1<<17)
#elif MEM_BLOCK_SIZE == (1<<16)
#elif MEM_BLOCK_SIZE == (1<<15)
#elif MEM_BLOCK_SIZE == (1<<14)
#elif MEM_BLOCK_SIZE == (1<<13)
#elif MEM_BLOCK_SIZE == (1<<12)
#else
#error unknown MEM_BLOCK_SIZE
#endif

typedef struct { void* next; size_t size; } __alloc_t;

#define BLOCK_START(b)	(((void*)(b))-sizeof(__alloc_t))
#define BLOCK_RET(b)	(((void*)(b))+sizeof(__alloc_t))
#define PAGE_ALIGN(s)	((s+(MEM_BLOCK_SIZE-1)) & ~(MEM_BLOCK_SIZE-1))
