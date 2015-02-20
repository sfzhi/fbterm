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

#include <string.h>
#include "fbshellman.h"
#include "fbshell.h"
#include "fbterm.h"
#include "screen.h"
#include "improxy.h"
#include "font.h"

#define screen (Screen::instance())
#define SHELL_ANY ((FbShell *)-1)

DEFINE_INSTANCE_DEFAULT(FbShellManager)

FbShellManager::FbShellManager()
{
	mVcCurrent = false;
	mShellCount = 0;
	mCurShell = 0;
	mActiveShell = 0;
	memset(mShellList, 0, sizeof(mShellList));
}

FbShellManager::~FbShellManager()
{
}

void FbShellManager::createShell()
{
	if (mShellCount == NR_SHELLS) return;
	mShellCount++;

	u32 index = getIndex(0, true, false);
	mShellList[index] = new FbShell();
	switchShell(index);
}

void FbShellManager::deleteShell()
{
	if (mShellList[mCurShell]) delete mShellList[mCurShell];
}

void FbShellManager::nextShell()
{
	switchShell(getIndex(SHELL_ANY, true, true));
}

void FbShellManager::prevShell()
{
	switchShell(getIndex(SHELL_ANY, false, true));
}

void FbShellManager::switchShell(u32 num)
{
	if (num >= NR_SHELLS) return;

	mCurShell = num;
	if (mVcCurrent && setActive(mShellList[mCurShell])) {
		redraw(0, 0, screen->cols(), screen->rows());
	}
}

void FbShellManager::drawCursor()
{
	if (mActiveShell) {
		mActiveShell->updateCursor();
	}
}

void FbShellManager::historyScroll(bool down)
{
	if (mActiveShell) {
		mActiveShell->historyDisplay(false, down ? mActiveShell->h() : -mActiveShell->h());
	}
}

void FbShellManager::switchVc(bool enter)
{
	mVcCurrent = enter;
	setActive(enter ? mShellList[mCurShell] : 0);

	if (enter) {
		redraw(0, 0, screen->cols(), screen->rows());
	}
}

void FbShellManager::shellExited(FbShell *shell)
{
	if (!shell) return;

	u8 index = getIndex(shell, true, false);
	mShellList[index] = 0;

	if (index == mCurShell) {
		prevShell();
	}

	if (!--mShellCount) {
		FbTerm::instance()->exit();
	}
}

u32 FbShellManager::getIndex(FbShell *shell, bool forward,  bool stepfirst)
{
	u32 index, temp = mCurShell + NR_SHELLS;

	#define STEP() do { \
		if (forward) temp++; \
		else temp--; \
	} while (0)

	if (stepfirst) STEP();

	for (u32 i = NR_SHELLS; i--;) {
		index = temp % NR_SHELLS;
		if ((shell == SHELL_ANY && mShellList[index]) || shell == mShellList[index]) break;
		STEP();
	}

	return index;
}

bool FbShellManager::setActive(FbShell *shell)
{
	if (mActiveShell == shell) return false;

	if (mActiveShell) {
		mActiveShell->switchVt(false, shell);
	}

	FbShell *oldActiveShell = mActiveShell;
	mActiveShell = shell;

	if (mActiveShell) {
		mActiveShell->switchVt(true, oldActiveShell);
	}

	return true;
}

void FbShellManager::redraw(u16 x, u16 y, u16 w, u16 h)
{
	if (mActiveShell) {
		mActiveShell->expose(x, y, w, h);
	} else {
		screen->fillRect(FW(x), FH(y), FW(w), FH(h), 0);
	}
}

void FbShellManager::childProcessExited(s32 pid)
{
	for (u32 i = 0; i < NR_SHELLS; i++) {
		if (mShellList[i] && mShellList[i]->childProcessExited(pid)) break;
	}
}
