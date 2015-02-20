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

#ifndef SCREEN_H
#define SCREEN_H

#include "type.h"
#include "instance.h"

#define NR_COLORS 256

struct Color {
	u8 red, green, blue;
};

typedef enum { Rotate0 = 0, Rotate90, Rotate180, Rotate270 } RotateType;

class Screen
{
	DECLARE_INSTANCE(Screen)
public :
	u32 width() { return mWidth; }
	u32 height() { return mHeight; }
	u16 cols() { return mCols; }
	u16 rows() { return mRows; }

	RotateType rotateType() { return mRotateType; }
	void rotateRect(u32 &x, u32 &y, u32 &w, u32 &h);
	void rotatePoint(u32 w, u32 h, u32 &x, u32 &y);

	void drawText(u32 x, u32 y, u8 fc, u8 bc, u16 num, u16 *text, bool *dw);
	void fillRect(u32 x, u32 y, u32 w, u32 h, u8 color);

	bool move(u16 scol, u16 srow, u16 dcol, u16 drow, u16 w, u16 h);
	void setPalette(const Color *palette);
	
	void enableScroll(bool enable) { mScrollEnable = enable; }

	void showInfo(bool verbose);
	virtual void switchVc(bool enter);

protected:
	u32 mWidth, mHeight;
	u16 mCols, mRows;
	u32 mBitsPerPixel, mBytesPerLine;

	RotateType mRotateType;

	typedef enum { Redraw = 0, YPan, YWrap, XPan } ScrollType;
	ScrollType mScrollType;
	u32 mOffsetMax;
	s32 mOffsetCur;

	u8 *mVMemBase;
	const Color *mPalette;

private:
	virtual void setupOffset() {}
	virtual void setupPalette(bool restore) {}
	virtual const s8 *drvId() = 0;

	void eraseMargin(bool top, u16 h);
	void drawGlyphs(u32 x, u32 y, u8 fc, u8 bc, u16 num, u16 *text, bool *dw);
	void drawGlyph(u32 x, u32 y, u8 fc, u8 bc, u16 code, bool dw);
	void adjustOffset(u32 &x, u32 &y);

	void initFillDraw();
	void endFillDraw();

	void fillX(u32 x, u32 y, u32 w, u8 color);
	void fillXBg(u32 x, u32 y, u32 w, u8 color);
	void draw8(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);
	void draw15(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);
	void draw16(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);
	void draw32(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);
	void draw8Bg(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);
	void draw15Bg(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);
	void draw16Bg(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);
	void draw32Bg(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);

	typedef void (Screen::*fillFun)(u32 x, u32 y, u32 w, u8 color);
	typedef void (Screen::*drawFun)(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap);

	fillFun fill;
	drawFun draw;
	bool mScrollEnable;
};
#endif
