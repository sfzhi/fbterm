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

#include <errno.h>
#include <pty.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "shell.h"

void waitChildProcessExit(s32 pid)
{
	if (pid < 0) return;

	kill(pid, SIGTERM);
	sched_yield();

	s32 ret = waitpid(pid, 0, WNOHANG);
	if (ret > 0 || (ret == -1 && errno == ECHILD)) return;

	for (u32 i = 5; i--;) {
		usleep(100 * 1000);

		ret = waitpid(pid, 0, WNOHANG);
		if (ret > 0) break;
	}

	if (ret <= 0) {
		kill(pid, SIGKILL);
		waitpid(pid, 0, 0);
	}
}

Shell::SelectedText Shell::mSelText;

Shell::Shell()
{
	mPid = -1;
}

Shell::~Shell()
{
	setFd(-1);
	waitChildProcessExit(mPid);
}

void Shell::createShellProcess(s8 **command)
{
	s32 fd;
	mPid = forkpty(&fd, NULL, NULL, NULL);

	switch (mPid) {
	case -1:
		break;

	case 0:  // child process
		initShellProcess();
		setenv("TERM", "linux", 1);

		if (command) {
			execvp(command[0], command);
		} else {
			s8 *shell = getenv("SHELL");
			if (shell) execlp(shell, shell, NULL);

			struct passwd *userpd = getpwuid(getuid());
			execlp(userpd->pw_shell, userpd->pw_shell, NULL);

			execlp("/bin/sh", "/bin/sh", NULL);
		}

		exit(1);
		break;

	default:
		setFd(fd);
		setCodec("UTF-8", localCodec());
		resize(w(), h());
		break;
	}
}

void Shell::readyRead(s8 *buf, u32 len)
{
	resetTextSelect();
	input((const u8 *)buf, len);
}

void Shell::sendBack(const s8 *data)
{
	write((s8*)data, strlen(data));
}

void Shell::resize(u16 w, u16 h)
{
	if (!w || !h) return;

	resetTextSelect();
	VTerm::resize(w, h);

	struct winsize size;
	size.ws_xpixel = 0;
	size.ws_ypixel = 0;
	size.ws_col = w;
	size.ws_row = h;
	ioctl(fd(), TIOCSWINSZ, &size);
}

void Shell::keyInput(s8 *buf, u32 len)
{
	resetTextSelect();
	write(buf, len);
}

void Shell::mouseInput(u16 x, u16 y, s32 type, s32 buttons)
{
	if (x >= w()) x = w() - 1;
	if (y >= h()) y = h() - 1;

	s32 btn = buttons & MouseButtonMask;
	s32 modifies = buttons & ModifyButtonMask;
	u16 rtype = mode(MouseReport);

	if (btn && rtype != Wheel && (rtype == MouseNone || (modifies & ShiftButton))) {
		textSelect(x, y, type, btn);
		return;
	}

	if (rtype == MouseNone) return;

	s32 val = -1;

	switch (type) {
	case Press:
	case DblClick:
		if (btn & LeftButton) val = 0;
		else if (btn & MidButton) val = 1;
		else if (btn & RightButton) val = 2;
		break;
	case Release:
		if (rtype == MouseX11) val = 3;
		break;
	case Wheel:
		if (rtype == MouseX11) {
			val = 64;
			if (btn & WheelDown) val |= 1;
		}
		break;
	default:
		break;
	}

	if (rtype == MouseX11 && val != -1) {
		if (modifies & ShiftButton)	val |= 4;
		if (modifies & AltButton) val |= 8;
		if (modifies & ControlButton) val |= 16;
	}

	if (val != -1) {
		s8 buf[8];
		snprintf(buf, sizeof(buf), "\e[M%c%c%c", ' ' + val, ' ' + x + 1, ' ' + y + 1);
		sendBack(buf);
	}
}

void Shell::textSelect(u16 x, u16 y, s32 type, s32 btn)
{
	if (btn == LeftButton) {
		switch (type) {
		case Press:
			resetTextSelect();
			mSelState.selecting = true;
			startTextSelect(x, y);
			break;
		case Move:
			if (mSelState.selecting) {
				middleTextSelect(x, y);
			} else {
				resetTextSelect();
				mSelState.selecting = true;
				startTextSelect(x, y);
			}
			break;
		case Release:
			mSelState.selecting = false;
			endTextSelect();
			break;
		case DblClick:
			resetTextSelect();
			autoTextSelect(x, y);
			break;
		default:
			break;
		}
	} else if (btn == RightButton) {
		if (type == Press || type == DblClick) {
			resetTextSelect();
			putSelectedText();
		}
	}
}

void Shell::startTextSelect(u16 x, u16 y)
{
	mSelState.start = mSelState.end = y * w() + x;

	inverseTextColor(mSelState.start, mSelState.end);
	mSelState.color_inversed = true;
}

void Shell::middleTextSelect(u16 x, u16 y)
{
	u32 start = mSelState.start, end = mSelState.end;
	u32 new_end = y * w() + x;

	bool dir_sel = (end >= start);
	bool dir_new_sel = (new_end >= start);

	mSelState.end = new_end;

	if (dir_sel == dir_new_sel) {
		bool dir_change = (new_end > end);

		u32 &pos = (dir_sel == dir_change) ? end : new_end;
		CharAttr attr = charAttr(pos % w(), pos / w());

		if (dir_sel) {
			if (attr.type == CharAttr::DoubleLeft) pos++;
			pos++;
		} else {
			if (attr.type == CharAttr::DoubleRight) pos--;
			pos--;
		}

		bool dir_new_change = (new_end == end) ? dir_change : (new_end > end);

		if (dir_change == dir_new_change) {
			inverseTextColor(end, new_end);
		}
	} else {
		inverseTextColor(start, end);
		inverseTextColor(start, new_end);
	}
}

static void utf16_to_utf8(u16 *buf16, u32 num, s8 *buf8)
{
	u16 code;
	u32 index = 0;
	for (; num--; buf16++) {
		code = *buf16;
		if (code >> 11) {
			buf8[index++] = 0xe0 | (code >> 12);
			buf8[index++] = 0x80 | ((code >> 6) & 0x3f);
			buf8[index++] = 0x80 | (code & 0x3f);
		} else if (code >> 7) {
			buf8[index++] = 0xc0 | ((code >> 6) & 0x1f);
			buf8[index++] = 0x80 | (code & 0x3f);
		} else {
			buf8[index++] = code;
		}
	}

	buf8[index] = 0;
}

#define SWAP(a, b) do { \
	if (a > b) { \
		u32 tmp = a; \
		a = b; \
		b = tmp; \
	} \
} while (0)

void Shell::endTextSelect()
{
	u32 start = mSelState.start, end = mSelState.end;
	SWAP(start, end);

	u32 len = end - start + 1;
	u16 buf[len];
	s8 *text = new s8[len * 3];

	u16 sx, sy, ex, ey;
	sx = start % w(), sy = start / w();
	ex = end % w(), ey = end / w();

	u32 index = 0;
	for (u16 y = sy; y <= ey; y++) {
		u16 x = (y == sy ? sx : 0);
		u16 end = (y == ey ? ex : (w() -1));
		for (; x <= end; x++) {
			buf[index++] = charCode(x, y);
			if (charAttr(x, y).type == CharAttr::DoubleLeft) x++;
		}
	}

	utf16_to_utf8(buf, index, text);
	mSelText.setText(text);
}

void Shell::resetTextSelect()
{
	mSelState.selecting = false;
	if (mSelState.color_inversed) {
		mSelState.color_inversed = false;
		inverseTextColor(mSelState.start, mSelState.end);
	}
}

void Shell::autoTextSelect(u16 x, u16 y)
{
	static u32 inwordLut[8] = {
		0x00000000,
		0x03FF0000, /* digits */
		0x07FFFFFE, /* uppercase */
		0x07FFFFFE, /* lowercase */
		0x00000000,
		0x00000000,
		0xFF7FFFFF, /* latin-1 accented letters, not multiplication sign */
		0xFF7FFFFF  /* latin-1 accented letters, not division sign */
	};

	static bool inited = false;
	if (!inited) {
		inited = true;

		u8 chrs[32];
		initWordChars((s8*)chrs, sizeof(chrs));

		for (u32 i = 0; chrs[i]; i++) {
			if (chrs[i] > 0x7f || chrs[i] <= ' ') continue;
			inwordLut[chrs[i] >> 5] |= 1 << (chrs[i] & 0x1f);
		}
	}

	#define inword(c) ((c) > 0xff || (( inwordLut[(c) >> 5] >> ((c) & 0x1f) ) & 1))

	u16 code = charCode(x, y);
	bool inw = inword(code);
	bool found = false;

	u16 sx = x;
	while (sx) {
		if (inw ^ inword(code)) {
			found = true;
			break;
		}

		code = charCode(--sx, y);
	}

	if (found) sx++;

	code = charCode(x, y);
	inw = inword(code);
	found = false;

	u16 ex = x;
	while (ex != w() -1) {
		if (inw ^ inword(code)) {
			found = true;
			break;
		}

		code = charCode(++ex, y);
	}

	if (found) ex--;

	mSelState.start = y * w() + sx;
	mSelState.end = y * w() + ex;
	inverseTextColor(mSelState.start, mSelState.end);
	mSelState.color_inversed = true;
}

void Shell::putSelectedText()
{
	if (mSelText.text) {
		sendBack(mSelText.text);
	}
}

void Shell::inverseTextColor(u32 start, u32 end)
{
	SWAP(start, end);

	u16 sx, sy, ex, ey;
	sx = start % w(), sy = start / w();
	ex = end % w(), ey = end / w();

	inverse(sx, sy, ex, ey);

	if (charAttr(sx, sy).type == CharAttr::DoubleRight) sx--;
	if (charAttr(ex, ey).type == CharAttr::DoubleLeft) ex++;

	requestUpdate(sx, sy, (sy == ey ? (ex + 1) : w()) - sx, 1);

	if (ey > sy + 1) {
		requestUpdate(0, sy + 1, w(), ey - sy - 1);
	}

	if (ey > sy) {
		requestUpdate(0, ey, ex + 1, 1);
	}
}
