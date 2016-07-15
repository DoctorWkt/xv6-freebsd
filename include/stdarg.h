/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This file includes code from FreeBSD.
 * See the file LICENSE.BSD for full license information.
 */

#ifndef _STDARG_H_
#define	_STDARG_H_

typedef char *va_list;

#define	__va_promote(type) \
	(((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define	va_start(ap, last) \
	(ap = ((va_list)&(last) + __va_promote(last)))

#define	va_arg(ap, type) \
	((type *)(ap += sizeof(type) < sizeof(int) ? \
		(exit(0), 0) : sizeof(type)))[-1]

#define	va_end(ap)

#endif /* !_STDARG_H_ */
