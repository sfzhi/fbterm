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

#ifndef FBSHELL_H
#define FBSHELL_H

#include "shell.h"

class FbShell : public Shell {
public:
	void keyInput(s8 *buf, u32 len);
	void mouseInput(u16 x, u16 y, s32 type, s32 buttons);
	void switchCodec(u8 index);
	void expose(u16 x, u16 y, u16 w, u16 h);
	void toggleIm();
	void killIm();
	void imInput(s8 *buf, u32 len);
	void ImExited() { mImProxy = 0; }
	bool childProcessExited(s32 pid);

private:
	friend class FbShellManager;
	FbShell();
	~FbShell();

	virtual void drawChars(CharAttr attr, u16 x, u16 y, u16 w, u16 num, u16 *chars, bool *dws);
	virtual bool moveChars(u16 sx, u16 sy, u16 dx, u16 dy, u16 w, u16 h);
	virtual void drawCursor(CharAttr attr, u16 x, u16 y, u16 c);
	virtual void modeChanged(ModeType type);
	virtual void request(RequestType type, u32 val = 0);

	virtual void initShellProcess();
	virtual void readyRead(s8 *buf, u32 len);

	void switchVt(bool enter, FbShell *peer);
	void adjustCharAttr(CharAttr &attr);
	void enableCursor(bool enable);
	void updateCursor();
	void clearMousePointer();

	void changeMode(ModeType type, u16 val);
	void reportCursor();
	void reportMode();

	struct Cursor {
		Cursor() {
			x = y = (u16)-1;
			showed = false;
		}
		bool showed;
		u16 x, y;
		u16 code;
		CharAttr attr;
	} mCursor;

	struct MousePointer {
		MousePointer() {
			drawed = false;
		}
		u16 x, y;
		bool drawed;
	} mMousePointer;

	bool mPaletteChanged;
	struct Color *mPalette;
	class ImProxy *mImProxy;
};

#endif
