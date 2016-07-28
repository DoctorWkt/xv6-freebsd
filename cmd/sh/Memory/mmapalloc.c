#include "mmapalloc.h"

static size_t get_size(size_t size) {
	size_t len;
	if (size==0) return 0;
	
	len = size + sizeof(__alloc_t);
	len = PAGE_ALIGN(len);
	if (len <= size) return 0;
	return len;
}


void* malloc(size_t size) {
	__alloc_t *x;
	
	size = get_size(size);
	if (size==0) return 0;
	
	x = mmap(0, size, PROT_READ|PROT_WRITE,
			 MAP_ANONYMOUS|MAP_PRIVATE, -1, (size_t)0);
	if (x == MAP_FAILED) return 0;
	
	x->size = size;
	return BLOCK_RET(x);
}


void free(void *_ptr) {
	if (_ptr) {
		__alloc_t *ptr = BLOCK_START(_ptr);
		if (ptr->size) munmap(ptr,ptr->size);
	}
}


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
			__alloc_t *x, *t = BLOCK_START(ptr);
			size_t len = get_size(size);
			if (len==0) return 0;
			
			old_size = t->size;
			if (old_size < len) {
			do_malloc:
				x = ptr;
				ptr = malloc(size);
				if (ptr && old_size) {
					memcpy(ptr, x, old_size - sizeof(__alloc_t));
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
	if (nmemb && size/nmemb!=_size) return 0;
	return malloc(size);
}
#endif
