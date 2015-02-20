/*
 *   Copyright © 2008-2010 dragchan <zgchan317@gmail.com>
 *   This file is part of FbTerm.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "fbio.h"

#define NR_FDS 32

#ifdef HAVE_EPOLL
#include <sys/epoll.h>
#define NR_EPOLL_FDS 10
s32 epollFd;
#else
static fd_set fds;
static u32 maxfd = 0;
#endif

static IoPipe *ioPipeMap[NR_FDS];

IoDispatcher *IoDispatcher::createInstance()
{
	return new FbIoDispatcher();
}

FbIoDispatcher::FbIoDispatcher()
{
#ifdef HAVE_EPOLL
	epollFd = epoll_create(NR_EPOLL_FDS);
	fcntl(epollFd, F_SETFD, fcntl(epollFd, F_GETFD) | FD_CLOEXEC);
#else
	FD_ZERO(&fds);
#endif
}

FbIoDispatcher::~FbIoDispatcher()
{
	for (u32 i = NR_FDS; i--;) {
		if (ioPipeMap[i]) delete ioPipeMap[i];
	}

#ifdef HAVE_EPOLL
	close(epollFd);
#endif
}

void FbIoDispatcher::addIoSource(IoPipe *src, bool isread)
{
	if (src->fd() >= NR_FDS) return;
	ioPipeMap[src->fd()] = src;

#ifdef HAVE_EPOLL
	epoll_event ev;
	ev.data.fd = src->fd();
	ev.events = (isread ? EPOLLIN : EPOLLOUT);
	epoll_ctl(epollFd, EPOLL_CTL_ADD, src->fd(), &ev);
#else
	FD_SET(src->fd(), &fds);
	if (src->fd() > maxfd) maxfd = src->fd();
#endif
}

void FbIoDispatcher::removeIoSource(IoPipe *src, bool isread)
{
	if (src->fd() >= NR_FDS) return;
	ioPipeMap[src->fd()] = 0;

#ifdef HAVE_EPOLL
	epoll_event ev;
	ev.data.fd = src->fd();
	ev.events = (isread ? EPOLLIN : EPOLLOUT);
	epoll_ctl(epollFd, EPOLL_CTL_DEL, src->fd(), &ev);
#else
	FD_CLR(src->fd(), &fds);
#endif
}

void FbIoDispatcher::poll()
{
#ifdef HAVE_EPOLL
	epoll_event evs[NR_EPOLL_FDS];
	s32 nfds = epoll_wait(epollFd, evs, NR_EPOLL_FDS, -1);

	for (s32 i = 0; i < nfds; i++) {
		IoPipe *src = ioPipeMap[evs[i].data.fd];
		if (!src) continue;

		if (evs[i].events & EPOLLIN) {
			src->ready(true);
		}

		if (evs[i].events & EPOLLOUT) {
			src->ready(false);
		}

		if (evs[i].events & EPOLLHUP) {
			delete src;
		}
	}
#else
	fd_set rfds = fds;
	s32 num = select(maxfd + 1, &rfds, 0, 0, 0);
	if (num <= 0) return;

	for (u32 i = 0; i <= maxfd; i++) {
		if (FD_ISSET(i, &rfds)) {
			if (ioPipeMap[i]) {
				ioPipeMap[i]->ready(true);
			}
			if (!--num) break;
		}
	}
#endif
}
