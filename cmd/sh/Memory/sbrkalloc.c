#include <unistd.h>
#include <stdlib.h>
#include "shfeatures.h"

extern void *shalloc(size_t size);
extern void shfree(void *ptr);

#if defined(WANT_REALLOC) || defined(WANT_CALLOC)
#undef STATIC_READDIR_BUFFER
#endif

#ifdef STATIC_READDIR_BUFFER
#define skip_me(x) x
static char B, Buf[STATIC_READDIR_BUFFER];

#else
#define skip_me(x)
#endif

void *malloc(size_t size) {
	skip_me(if (size <= sizeof(Buf) && B==0) { B=1; return Buf; });
	return shalloc(size); 
}

void free(void *ptr) {
	skip_me(if (ptr == Buf) { B=0; return; });
	shfree(ptr);
}


#include <string.h>
#define BLOCK_START(b)	(((void*)(b))-sizeof(void*))
#define PAGE_ALIGN(s)	((s+(sizeof(void*)-1)) & ~(sizeof(void *)-1))

#ifdef WANT_REALLOC
void *realloc(void *ptr, size_t size) {
	size_t old_size = 0;
	if (ptr == 0) {
		if (size)
			goto do_malloc;
	} else {
		if (size==0) {
			free(ptr);
			return 0;
		} else {
			void *x, **t = BLOCK_START(ptr);
			size_t len = PAGE_ALIGN(size);
			if (len==0) return 0;
			
			old_size = (*t-ptr) & ~1;
			if (old_size < len) {
			do_malloc:
				x = ptr;
				ptr = malloc(size);
				if (ptr && old_size) {
					memcpy(ptr, x, old_size);
					free(x);
				}
			}
		}
	}
	return ptr;
}
#endif


#ifdef WANT_CALLOC
void *calloc(size_t nmemb, size_t _size) {
	size_t size=_size*nmemb;
	void *x;
	if (nmemb && size/nmemb!=_size) return 0;
	x = malloc(size);
	if (x) memset(x,0,size);
	return x;
}
#endif
