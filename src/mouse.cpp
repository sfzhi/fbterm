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

#include "mouse.h"
DEFINE_INSTANCE(Mouse)

#include "config.h"
#ifdef ENABLE_GPM

#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <gpm.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/keyboard.h>
#include "fbshellman.h"
#include "fbshell.h"
#include "screen.h"

static s32 open_gpm(Gpm_Connect *conn)
{
	s8 buf[64];
	s32 ret = ttyname_r(STDIN_FILENO, buf, sizeof(buf));

	u32 index = strlen(buf) - 1;
	while (buf[index] >= '0' && buf[index] <= '9') index--;

	s8 *tail;
	conn->vc = strtol(buf + index + 1, &tail, 10);
	if (*tail) return -1;

	conn->pid = getpid();

	s32 gpm_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (gpm_fd == -1) return -1;

	struct sockaddr_un addr;
	memset((s8 *)&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, GPM_NODE_CTL, sizeof(addr.sun_path) - 1);

	#define OFFSET(TYPE, MEMBER) ((size_t)(&(((TYPE *)0)->MEMBER)))
	u32 len = OFFSET(sockaddr_un, sun_path) + sizeof(GPM_NODE_CTL) - 1;
	if (len > sizeof(addr)) len = sizeof(addr);

	if(connect(gpm_fd, (struct sockaddr *)(&addr), len) < 0 ||
	   write(gpm_fd, conn, sizeof(*conn)) != sizeof(*conn)) {
		close(gpm_fd);
		return -1;
	}

	return gpm_fd;
}

Mouse *Mouse::createInstance()
{
	Gpm_Connect conn;
	conn.eventMask = ~0;
	conn.defaultMask = ~GPM_HARD;
	conn.maxMod = ~0;
	conn.minMod = 0;

	s32 fd = open_gpm(&conn);
	if (fd == -1) return 0;

	Mouse *mouse = new Mouse();
	mouse->setFd(fd);

	return mouse;
}

Mouse::Mouse()
{
	x = y = 0;
}

Mouse::~Mouse()
{
}

void Mouse::readyRead(s8 *buf, u32 len)
{
	if (len % sizeof(Gpm_Event)) return;

	FbShell *shell = FbShellManager::instance()->activeShell();
	if (!shell) return;

	u16 maxx = Screen::instance()->cols(), maxy = Screen::instance()->rows();

	len /= sizeof(Gpm_Event);
	Gpm_Event *ev = (Gpm_Event *)buf;

	for (; len--; ev++) {
		s32 type = -1, buttons = 0;

		if (ev->wdy) type = Wheel;
		else if ((ev->type & GPM_MOVE) || (ev->type & GPM_DRAG)) type = Move;
		else if (ev->type & GPM_DOWN) type = (ev->type & GPM_DOUBLE) ? DblClick : Press;
		else if (ev->type & GPM_UP) type = Release;
		if (type == -1) continue;

		if (ev->buttons & GPM_B_LEFT) buttons |= LeftButton;
		if (ev->buttons & GPM_B_MIDDLE) buttons |= MidButton;
		if (ev->buttons & GPM_B_RIGHT) buttons |= RightButton;
		if (type == Wheel) buttons |= ((ev->wdy < 0) ? WheelDown : WheelUp);

		if (ev->modifiers & (1 << KG_SHIFT)) buttons |= ShiftButton;
		if (ev->modifiers & (1 << KG_CTRL)) buttons |= ControlButton;
		if (ev->modifiers & ((1 << KG_ALT) | (1 << KG_ALTGR))) buttons |= AltButton;

		s16 newx = (s16)x + ev->dx, newy =(s16)y + ev->dy;
		if (newx < 0) newx = 0;
		if (newy < 0) newy = 0;
		if (newx >= maxx) newx = maxx - 1;
		if (newy >= maxy) newy = maxy - 1;
		if (newx == x && newy == y && !(buttons & MouseButtonMask)) continue;

		shell->mouseInput(newx, newy, type, buttons);
		x = newx;
		y = newy;
	}
}

void Mouse::switchVc(bool enter)
{
}

#else

#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

Mouse *Mouse::createInstance()
{
	return new Mouse();
}

Mouse::Mouse()
{
}

Mouse::~Mouse()
{
}

void Mouse::switchVc(bool enter)
{
	ioctl(STDIN_FILENO, KDSETMODE, enter ? KD_GRAPHICS : KD_TEXT);
}

void Mouse::readyRead(s8 *buf, u32 len)
{
}
#endif
