/*
sys/svrctl.h

Created:	Feb 15, 1994 by Philip Homburg <philip@cs.vu.nl>
*/

#ifndef _SYS__SVRCTL_H
#define _SYS__SVRCTL_H

#ifndef _TYPES_H
#include <sys/types.h>
#endif

/* Server control commands have the same encoding as the commands for ioctls. */
#include <minix/ioctl.h>

/* MM controls. */
#define MMSIGNON	_IO ('M',  4)
#define MMSWAPON	_IOW('M',  5, struct mmswapon)
#define MMSWAPOFF	_IO ('M',  6)

/* FS controls. */
#define FSSIGNON	_IOW('F',  2, struct fssignon)
#define FSDEVMAP	_IORW('F', 5, struct fsdevmap)

/* Kernel controls. */
#define SYSSIGNON	_IOR('S',  2, struct systaskinfo)
#define SYSGETENV	_IOW('S',  5, struct sysgetenv)

struct mmswapon {
	u32_t		offset;		/* Starting offset within file. */
	u32_t		size;		/* Size of swap area. */
	char		file[128];	/* Name of swap file/device. */
};

enum fsdevstyle { STYLE_DEV, STYLE_NDEV, STYLE_TTY, STYLE_CLONE };

struct fssignon {
	dev_t		dev;		/* Example device to manage. */
	enum fsdevstyle	style;		/* Management style. */
};

struct systaskinfo {
	int		proc_nr;	/* Process number of caller. */
};

struct sysgetenv {
	char		*key;		/* Name requested. */
	size_t		keylen;		/* Length of name including \0. */
	char		*val;		/* Buffer for returned data. */
	size_t		vallen;		/* Size of return data buffer. */
};

_PROTOTYPE( int svrctl, (int _request, void *_data)			);

#endif /* _SYS__SVRCTL_H */
