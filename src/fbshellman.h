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

#ifndef FBSHELL_MANAGER_H
#define FBSHELL_MANAGER_H

#include "type.h"
#include "instance.h"

class FbShell;

class FbShellManager {
	DECLARE_INSTANCE(FbShellManager)
public:
	FbShell *activeShell() {
		return mActiveShell;
	}
	void createShell();
	void deleteShell();
	void shellExited(FbShell *shell);
	void switchShell(u32 num);
	void nextShell();
	void prevShell();

	void drawCursor();
	void historyScroll(bool down);
	void redraw(u16 x, u16 y, u16 w, u16 h);
	void switchVc(bool enter);
	void childProcessExited(s32 pid);

private:
	u32 getIndex(FbShell *shell, bool forward, bool stepfirst);
	bool setActive(FbShell *shell);

	#define NR_SHELLS 10
	FbShell *mShellList[NR_SHELLS], *mActiveShell;
	u32 mShellCount, mCurShell;
	bool mVcCurrent;
};

#endif
