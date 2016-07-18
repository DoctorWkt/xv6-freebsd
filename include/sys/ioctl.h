/*	sys/ioctl.h - All ioctl() command codes.	Author: Kees J. Bot
 *								23 Nov 2002
 *
 * This header file includes all other ioctl command code headers.
 */

#ifndef _S_IOCTL_H
#define _S_IOCTL_H

int ioctl(int _fd, int _request, void *_data);

#endif /* _S_IOCTL_H */
