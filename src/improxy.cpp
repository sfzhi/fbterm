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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "improxy.h"
#include "immessage.h"
#include "fbconfig.h"
#include "fbshell.h"
#include "fbshellman.h"
#include "font.h"
#include "screen.h"
#include "input.h"
#include "fbterm.h"
#include <time.h>

#define NR_WIN_MSGS 512
#define MSG_DRAWTEXT_MAX_LEN 10240

#define sw FW(Screen::instance()->cols())
#define sh FH(Screen::instance()->rows())

#define OFFSET(TYPE, MEMBER) ((size_t)(&(((TYPE *)0)->MEMBER)))
#define MSG(a) ((Message *)(a))

ImProxy::ImProxy(FbShell *shell)
{
	mShell = shell;
	mPid = -1;
	mConnected = false;
	mActive = false;
	mMsgWaitState = NoMessageToWait;

	mValidWinNum = 0;
	memset(&mWins, 0, sizeof(mWins));

	createImProcess();
}

ImProxy::~ImProxy()
{
	mShell->ImExited();
	if (!mConnected) return;

	if (FbShellManager::instance()->activeShell() == mShell) {
		TtyInput::instance()->setRawMode(false);
	}

	sendDisconnect();
	setFd(-1);

	extern void waitChildProcessExit(s32 pid);
	waitChildProcessExit(mPid);

	Rectangle rect = { 0, 0, 0, 0 };
	for (u32 i = 0; i < NR_IM_WINS; i++) {
		if (mWins[i].w) setImWin(i, rect);
	}
}

void ImProxy::createImProcess()
{
	s8 app[128];
	Config::instance()->getOption("input-method", app, sizeof(app));
	if (!app[0]) return;

	int fds[2];
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) == -1) return;

	mPid = fork();

	switch (mPid) {
	case -1:
		close(fds[0]);
		close(fds[1]);
		break;

	case 0: {
		FbTerm::instance()->initChildProcess();
		close(fds[0]);

		s8 buf[16];
		snprintf(buf, sizeof(buf), "%d", fds[1]);
		setenv("FBTERM_IM_SOCKET", buf, 1);

		execlp(app, app, NULL);
		fprintf(stderr, "can't execute IM program %s!\n", app);
		exit(1);
		break;
	}

	default:
		close(fds[1]);
		setFd(fds[0]);
		waitImMessage(Connect);
		break;
	}
}

bool ImProxy::actived()
{
	return mActive;
}

void ImProxy::toggleActive()
{
	if (!mConnected) return;

	mActive ^= true;
	TtyInput::instance()->setRawMode(mRawInput && mActive);

	Message msg;
	msg.type = (mActive ? Active : Deactive);
	msg.len = sizeof(msg);
	write((s8 *)&msg, sizeof(msg));
}

void ImProxy::changeCursorPos(u16 col, u16 row)
{
	if (!mConnected || !mActive) return;

	Message msg;
	msg.type = CursorPosition;
	msg.len = sizeof(msg);
	msg.cursor.x = FW(col);
	msg.cursor.y = FH(row + 1);

	write((s8 *)&msg, sizeof(msg));
}

void ImProxy::changeTermMode(bool crlf, bool appkey, bool curo)
{
	if (!mConnected || !mActive) return;

	Message msg;
	msg.type = TermMode;
	msg.len = sizeof(msg);
	msg.term.crWithLf = crlf;
	msg.term.applicKeypad = appkey;
	msg.term.cursorEscO = curo;

	write((s8 *)&msg, sizeof(msg));
}

void ImProxy::switchVt(bool enter, ImProxy *peer)
{
	if (!mConnected || !mActive) return;

	TtyInput::instance()->setRawMode(enter && mRawInput);

	Message msg;
	msg.type = (enter ? ShowUI : HideUI);
	msg.len = sizeof(msg);
	if (enter) msg.winid = (unsigned)-1;

	write((s8 *)&msg, sizeof(msg));

	if (!enter) {
		waitImMessage(AckHideUI);
		Screen::instance()->enableScroll(true);
	}
}

void ImProxy::sendKey(s8 *keys, u32 len)
{
	if (!mConnected || !mActive || !keys || !len) return;

	s8 buf[OFFSET(Message, keys) + len];

	MSG(buf)->type = SendKey;
	MSG(buf)->len = sizeof(buf);
	memcpy(MSG(buf)->keys, keys, len);

	write(buf, MSG(buf)->len);
}

void ImProxy::sendInfo()
{
	Message msg;
	msg.type = FbTermInfo;
	msg.len = sizeof(msg);
	msg.info.fontHeight = FH(1);
	msg.info.fontWidth = FW(1);
	msg.info.screenHeight = sh;
	msg.info.screenWidth = sw;

	write((s8 *)&msg, sizeof(msg));
}

void ImProxy::sendAckWin()
{
	Message msg;
	msg.type = AckWin;
	msg.len = sizeof(msg);
	write((s8 *)&msg, sizeof(msg));
}

void ImProxy::sendAckPing()
{
	Message msg;
	msg.type = AckPing;
	msg.len = sizeof(msg);
	write((s8 *)&msg, sizeof(msg));
}

void ImProxy::sendDisconnect()
{
	if (!mConnected) return;

	Message msg;
	msg.type = Disconnect;
	msg.len = sizeof(msg);
	write((s8 *)&msg, sizeof(msg));
}

void ImProxy::readyRead(s8 *buf, u32 len)
{
	for (s8 *end = buf + len; buf < end && MSG(buf)->len && MSG(buf)->len <= (end - buf); buf += MSG(buf)->len) {
		if (mMsgWaitState == WaitingMessage && mMsgWaitType == MSG(buf)->type) mMsgWaitState = GotMessage;
		if (!mConnected && MSG(buf)->type != Connect) continue;

		switch (MSG(buf)->type) {
		case Connect:
			mConnected = true;
			mRawInput = MSG(buf)->raw;

			sendInfo();
			break;

		case PutText:
			if (MSG(buf)->len > OFFSET(Message, texts)) {
				mShell->imInput(MSG(buf)->texts, MSG(buf)->len - OFFSET(Message, texts));
			}
			break;

		case SetWin:
			if (MSG(buf)->len == sizeof(Message)) { // keep compatibility with v1.5
				setImWin(MSG(buf)->win.winid, MSG(buf)->win.rect);
			}
			sendAckWin();
			break;

		case FillRect:
			if (addWinMsg(MSG(buf)) && FbShellManager::instance()->activeShell() == mShell) {
				doFillRect(MSG(buf));
			}
			break;

		case DrawText:
			if (addWinMsg(MSG(buf)) && FbShellManager::instance()->activeShell() == mShell) {
				doDrawText(MSG(buf));
			}
			break;

		case Ping:
			sendAckPing();
			break;

		default:
			break;
		}
	}
}

typedef enum { Intersect, Inside, Outside } IntersectState;

static IntersectState intersectRectangles(const Rectangle &r1, const Rectangle &r2)
{
	if (!r1.w || !r1.h) return Inside;
	if (!r2.w || !r2.h) return Outside;
	if (r1.x >= (r2.x + r2.w) || r1.y >= (r2.y + r2.h) || (r1.x + r1.w) <= r2.x || (r1.y + r1.h) <= r2.y) return Outside;
	if (r1.x >= r2.x && r1.y >= r2.y && (r1.x + r1.w) <= (r2.x + r2.w) && (r1.y + r1.h) <= (r2.y + r2.h)) return Inside;
	return Intersect;
}

void ImProxy::redrawImWin(const Rectangle &rect)
{
	for (u32 num = mValidWinNum, i = 0; num && i < NR_IM_WINS; i++) {
		if (!mWins[i].w || !mWins[i].h) continue;
		num--;

		IntersectState state = intersectRectangles(rect, mWins[i]);
		if (state == Outside) continue;

		for (MsgList::iterator it = mWinMsgs[i].begin(); it != mWinMsgs[i].end(); it++) {
			Message *m = *it;

			if (m->type == FillRect) doFillRect(m);
			else if (m->type == DrawText) doDrawText(m);
		}
	}
}

void ImProxy::setImWin(u32 id, Rectangle &rect)
{
	if (id >= NR_IM_WINS) return;

	if (!rect.w || !rect.h || rect.x >= sw || rect.y >= sh)
		memset(&rect, 0, sizeof(rect));

	if (rect.x + rect.w >= sw) rect.w = sw - rect.x;
	if (rect.y + rect.h >= sh) rect.h = sh - rect.y;

	bool active = (FbShellManager::instance()->activeShell() == mShell);

	if (active && intersectRectangles(mWins[id], rect) != Inside) {
		Rectangle oldrect = mWins[id];

		memset(&mWins[id], 0, sizeof(rect));
		mValidWinNum--;

		u32 endx = oldrect.x + oldrect.w, endy = oldrect.y + oldrect.h;
		u16 col = oldrect.x / FW(1);
		u16 row = oldrect.y / FH(1);
		u16 endcol = endx / FW(1);
		u16 endrow = endy / FH(1);

		mShell->expose(col, row, endcol - col + 1, endrow - row + 1);
	}

	if (!mWins[id].w && rect.w) mValidWinNum++;
	mWins[id] = rect;

	if (active) {
		Screen::instance()->enableScroll(!mValidWinNum);
	}

	for (MsgList::iterator it = mWinMsgs[id].begin(); it != mWinMsgs[id].end(); it++) {
		delete[] (s8 *)(*it);
	}

	mWinMsgs[id].clear();
}

bool ImProxy::addWinMsg(Message *m)
{
	Rectangle rect = { 0, 0, 1, 1 };
	if (m->type == FillRect) rect = m->fillRect.rect;
	else {
		if (m->len > MSG_DRAWTEXT_MAX_LEN) return false;

		rect.x = m->drawText.x;
		rect.y = m->drawText.y;
	}

	for (u32 num = mValidWinNum, i = 0; num && i < NR_IM_WINS; i++) {
		if (!mWins[i].w || !mWins[i].h) continue;
		num--;

		IntersectState state = intersectRectangles(rect, mWins[i]);
		if (state != Inside) continue;

		if (mWinMsgs[i].size() >= NR_WIN_MSGS) return false;

		s8 *buf = new s8[m->len];
		memcpy(buf, m, m->len);

		mWinMsgs[i].push_back((Message *)buf);
		break;
	}

	return true;
}

void ImProxy::doFillRect(Message *m)
{
	Rectangle &rect = m->fillRect.rect;
	Screen::instance()->fillRect(rect.x, rect.y, rect.w, rect.h, m->fillRect.color);
}

static void utf8_to_utf16(u8 *utf8, u16 *utf16, u16 &len)
{
	u8 *end = utf8 + len;
	len = 0;

	for (; utf8 < end;) {
		if ((*utf8 & 0x80) == 0) {
			utf16[len++] = *utf8;
			utf8++;
		} else if ((*utf8 & 0xe0) == 0xc0) {
			utf16[len++] = ((*utf8 & 0x1f) << 6) | (utf8[1] & 0x3f);
			utf8 += 2;
		} else if ((*utf8 & 0xf0) == 0xe0) {
			utf16[len++] = ((*utf8 & 0xf) << 12) | ((utf8[1] & 0x3f) << 6) | (utf8[2] & 0x3f);
			utf8 += 3;
		} else utf8++;
	}
}

void ImProxy::doDrawText(Message *m)
{
	if (m->len <= OFFSET(Message, drawText.texts)) return;

	u16 len = m->len - OFFSET(Message, drawText.texts);
	u8 *utf8 = (u8 *)(m->drawText.texts);

	u16 utf16[len];
	utf8_to_utf16(utf8, utf16, len);

	if (!len) return;

	bool dws[len];
	for (u16 i = 0; i < len; i++) {
		dws[i] = (VTerm::charWidth(utf16[i]) == 2);
	}

	Screen::instance()->drawText(m->drawText.x, m->drawText.y, m->drawText.fc, m->drawText.bc, len, utf16, dws);
}

void ImProxy::waitImMessage(u32 type)
{
	mMsgWaitState = WaitingMessage;
	mMsgWaitType = type;

	timeval tv = {2, 0};

	while (1) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd(), &fds);

		s32 ret = select(fd() + 1, &fds, 0, 0, &tv);

		if (ret > 0 && FD_ISSET(fd(), &fds)) ready(true);
		if ((ret == -1 && errno != EINTR) || !ret || mMsgWaitState == GotMessage) break;
	};

	mMsgWaitState = NoMessageToWait;
}

void ImProxy::ioError(bool read, s32 err)
{
	if (read && mMsgWaitState == WaitingMessage) mMsgWaitState = GotMessage;
}
