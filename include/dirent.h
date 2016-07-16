#ifndef _DIRENT_H
#define _DIRENT_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

#ifndef _DIR_H
#include <sys/dir.h>
#endif

/* Definitions for the directory(3) routines: */
typedef struct {
	char		fd;	/* Filedescriptor of open directory */
#if 0
	short		_count;	/* This many objects in buf */
	off_t		_pos;	/* Position in directory file */
	struct _fl_direct  *_ptr;	/* Next slot in buf */
	struct _fl_direct  _buf[128];	/* One block of a directory file */
	struct _fl_direct  _v7f[3];	/* V7 entry transformed to flex */
#endif
} DIR;

/* Function Prototypes. */
_PROTOTYPE( int closedir, (DIR *_dirp)					);
_PROTOTYPE( DIR *opendir, (const char *_dirname)			);
_PROTOTYPE( struct dirent *readdir, (DIR *_dirp)			);
_PROTOTYPE( void rewinddir, (DIR *_dirp)				);

#ifdef _MINIX
_PROTOTYPE( int seekdir, (DIR *_dirp, off_t _loc)			);
_PROTOTYPE( off_t telldir, (DIR *_dirp)					);
#endif

#endif /* _DIRENT_H */
