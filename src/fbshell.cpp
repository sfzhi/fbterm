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
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "fbshell.h"
#include "fbshellman.h"
#include "fbconfig.h"
#include "screen.h"
#include "improxy.h"
#include "fbterm.h"
#include "font.h"
#include "input.h"

#define screen (Screen::instance())
#define manager (FbShellManager::instance())

static const Color defaultPalette[NR_COLORS] = {
	{0x00, 0x00, 0x00}, /* 0 */
	{0xaa, 0x00, 0x00}, /* 1 */
	{0x00, 0xaa, 0x00}, /* 2 */
	{0xaa, 0x55, 0x00}, /* 3 */
	{0x00, 0x00, 0xaa}, /* 4 */
	{0xaa, 0x00, 0xaa}, /* 5 */
	{0x00, 0xaa, 0xaa}, /* 6 */
	{0xaa, 0xaa, 0xaa}, /* 7 */
	{0x55, 0x55, 0x55}, /* 8 */
	{0xff, 0x55, 0x55}, /* 9 */
	{0x55, 0xff, 0x55}, /* 10 */
	{0xff, 0xff, 0x55}, /* 11 */
	{0x55, 0x55, 0xff}, /* 12 */
	{0xff, 0x55, 0xff}, /* 13 */
	{0x55, 0xff, 0xff}, /* 14 */
	{0xff, 0xff, 0xff}, /* 15 */

	{0x00, 0x00, 0x00}, /* 16 */
	{0x00, 0x00, 0x5f}, /* 17 */
	{0x00, 0x00, 0x87}, /* 18 */
	{0x00, 0x00, 0xaf}, /* 19 */
	{0x00, 0x00, 0xd7}, /* 20 */
	{0x00, 0x00, 0xff}, /* 21 */
	{0x00, 0x5f, 0x00}, /* 22 */
	{0x00, 0x5f, 0x5f}, /* 23 */
	{0x00, 0x5f, 0x87}, /* 24 */
	{0x00, 0x5f, 0xaf}, /* 25 */
	{0x00, 0x5f, 0xd7}, /* 26 */
	{0x00, 0x5f, 0xff}, /* 27 */
	{0x00, 0x87, 0x00}, /* 28 */
	{0x00, 0x87, 0x5f}, /* 29 */
	{0x00, 0x87, 0x87}, /* 30 */
	{0x00, 0x87, 0xaf}, /* 31 */
	{0x00, 0x87, 0xd7}, /* 32 */
	{0x00, 0x87, 0xff}, /* 33 */
	{0x00, 0xaf, 0x00}, /* 34 */
	{0x00, 0xaf, 0x5f}, /* 35 */
	{0x00, 0xaf, 0x87}, /* 36 */
	{0x00, 0xaf, 0xaf}, /* 37 */
	{0x00, 0xaf, 0xd7}, /* 38 */
	{0x00, 0xaf, 0xff}, /* 39 */
	{0x00, 0xd7, 0x00}, /* 40 */
	{0x00, 0xd7, 0x5f}, /* 41 */
	{0x00, 0xd7, 0x87}, /* 42 */
	{0x00, 0xd7, 0xaf}, /* 43 */
	{0x00, 0xd7, 0xd7}, /* 44 */
	{0x00, 0xd7, 0xff}, /* 45 */
	{0x00, 0xff, 0x00}, /* 46 */
	{0x00, 0xff, 0x5f}, /* 47 */
	{0x00, 0xff, 0x87}, /* 48 */
	{0x00, 0xff, 0xaf}, /* 49 */
	{0x00, 0xff, 0xd7}, /* 50 */
	{0x00, 0xff, 0xff}, /* 51 */

	{0x5f, 0x00, 0x00}, /* 52 */
	{0x5f, 0x00, 0x5f}, /* 53 */
	{0x5f, 0x00, 0x87}, /* 54 */
	{0x5f, 0x00, 0xaf}, /* 55 */
	{0x5f, 0x00, 0xd7}, /* 56 */
	{0x5f, 0x00, 0xff}, /* 57 */
	{0x5f, 0x5f, 0x00}, /* 58 */
	{0x5f, 0x5f, 0x5f}, /* 59 */
	{0x5f, 0x5f, 0x87}, /* 60 */
	{0x5f, 0x5f, 0xaf}, /* 61 */
	{0x5f, 0x5f, 0xd7}, /* 62 */
	{0x5f, 0x5f, 0xff}, /* 63 */
	{0x5f, 0x87, 0x00}, /* 64 */
	{0x5f, 0x87, 0x5f}, /* 65 */
	{0x5f, 0x87, 0x87}, /* 66 */
	{0x5f, 0x87, 0xaf}, /* 67 */
	{0x5f, 0x87, 0xd7}, /* 68 */
	{0x5f, 0x87, 0xff}, /* 69 */
	{0x5f, 0xaf, 0x00}, /* 70 */
	{0x5f, 0xaf, 0x5f}, /* 71 */
	{0x5f, 0xaf, 0x87}, /* 72 */
	{0x5f, 0xaf, 0xaf}, /* 73 */
	{0x5f, 0xaf, 0xd7}, /* 74 */
	{0x5f, 0xaf, 0xff}, /* 75 */
	{0x5f, 0xd7, 0x00}, /* 76 */
	{0x5f, 0xd7, 0x5f}, /* 77 */
	{0x5f, 0xd7, 0x87}, /* 78 */
	{0x5f, 0xd7, 0xaf}, /* 79 */
	{0x5f, 0xd7, 0xd7}, /* 80 */
	{0x5f, 0xd7, 0xff}, /* 81 */
	{0x5f, 0xff, 0x00}, /* 82 */
	{0x5f, 0xff, 0x5f}, /* 83 */
	{0x5f, 0xff, 0x87}, /* 84 */
	{0x5f, 0xff, 0xaf}, /* 85 */
	{0x5f, 0xff, 0xd7}, /* 86 */
	{0x5f, 0xff, 0xff}, /* 87 */

	{0x87, 0x00, 0x00}, /* 88 */
	{0x87, 0x00, 0x5f}, /* 89 */
	{0x87, 0x00, 0x87}, /* 90 */
	{0x87, 0x00, 0xaf}, /* 91 */
	{0x87, 0x00, 0xd7}, /* 92 */
	{0x87, 0x00, 0xff}, /* 93 */
	{0x87, 0x5f, 0x00}, /* 94 */
	{0x87, 0x5f, 0x5f}, /* 95 */
	{0x87, 0x5f, 0x87}, /* 96 */
	{0x87, 0x5f, 0xaf}, /* 97 */
	{0x87, 0x5f, 0xd7}, /* 98 */
	{0x87, 0x5f, 0xff}, /* 99 */
	{0x87, 0x87, 0x00}, /* 100 */
	{0x87, 0x87, 0x5f}, /* 101 */
	{0x87, 0x87, 0x87}, /* 102 */
	{0x87, 0x87, 0xaf}, /* 103 */
	{0x87, 0x87, 0xd7}, /* 104 */
	{0x87, 0x87, 0xff}, /* 105 */
	{0x87, 0xaf, 0x00}, /* 106 */
	{0x87, 0xaf, 0x5f}, /* 107 */
	{0x87, 0xaf, 0x87}, /* 108 */
	{0x87, 0xaf, 0xaf}, /* 109 */
	{0x87, 0xaf, 0xd7}, /* 110 */
	{0x87, 0xaf, 0xff}, /* 111 */
	{0x87, 0xd7, 0x00}, /* 112 */
	{0x87, 0xd7, 0x5f}, /* 113 */
	{0x87, 0xd7, 0x87}, /* 114 */
	{0x87, 0xd7, 0xaf}, /* 115 */
	{0x87, 0xd7, 0xd7}, /* 116 */
	{0x87, 0xd7, 0xff}, /* 117 */
	{0x87, 0xff, 0x00}, /* 118 */
	{0x87, 0xff, 0x5f}, /* 119 */
	{0x87, 0xff, 0x87}, /* 120 */
	{0x87, 0xff, 0xaf}, /* 121 */
	{0x87, 0xff, 0xd7}, /* 122 */
	{0x87, 0xff, 0xff}, /* 123 */

	{0xaf, 0x00, 0x00}, /* 124 */
	{0xaf, 0x00, 0x5f}, /* 125 */
	{0xaf, 0x00, 0x87}, /* 126 */
	{0xaf, 0x00, 0xaf}, /* 127 */
	{0xaf, 0x00, 0xd7}, /* 128 */
	{0xaf, 0x00, 0xff}, /* 129 */
	{0xaf, 0x5f, 0x00}, /* 130 */
	{0xaf, 0x5f, 0x5f}, /* 131 */
	{0xaf, 0x5f, 0x87}, /* 132 */
	{0xaf, 0x5f, 0xaf}, /* 133 */
	{0xaf, 0x5f, 0xd7}, /* 134 */
	{0xaf, 0x5f, 0xff}, /* 135 */
	{0xaf, 0x87, 0x00}, /* 136 */
	{0xaf, 0x87, 0x5f}, /* 137 */
	{0xaf, 0x87, 0x87}, /* 138 */
	{0xaf, 0x87, 0xaf}, /* 139 */
	{0xaf, 0x87, 0xd7}, /* 140 */
	{0xaf, 0x87, 0xff}, /* 141 */
	{0xaf, 0xaf, 0x00}, /* 142 */
	{0xaf, 0xaf, 0x5f}, /* 143 */
	{0xaf, 0xaf, 0x87}, /* 144 */
	{0xaf, 0xaf, 0xaf}, /* 145 */
	{0xaf, 0xaf, 0xd7}, /* 146 */
	{0xaf, 0xaf, 0xff}, /* 147 */
	{0xaf, 0xd7, 0x00}, /* 148 */
	{0xaf, 0xd7, 0x5f}, /* 149 */
	{0xaf, 0xd7, 0x87}, /* 150 */
	{0xaf, 0xd7, 0xaf}, /* 151 */
	{0xaf, 0xd7, 0xd7}, /* 152 */
	{0xaf, 0xd7, 0xff}, /* 153 */
	{0xaf, 0xff, 0x00}, /* 154 */
	{0xaf, 0xff, 0x5f}, /* 155 */
	{0xaf, 0xff, 0x87}, /* 156 */
	{0xaf, 0xff, 0xaf}, /* 157 */
	{0xaf, 0xff, 0xd7}, /* 158 */
	{0xaf, 0xff, 0xff}, /* 159 */

	{0xd7, 0x00, 0x00}, /* 160 */
	{0xd7, 0x00, 0x5f}, /* 161 */
	{0xd7, 0x00, 0x87}, /* 162 */
	{0xd7, 0x00, 0xaf}, /* 163 */
	{0xd7, 0x00, 0xd7}, /* 164 */
	{0xd7, 0x00, 0xff}, /* 165 */
	{0xd7, 0x5f, 0x00}, /* 166 */
	{0xd7, 0x5f, 0x5f}, /* 167 */
	{0xd7, 0x5f, 0x87}, /* 168 */
	{0xd7, 0x5f, 0xaf}, /* 169 */
	{0xd7, 0x5f, 0xd7}, /* 170 */
	{0xd7, 0x5f, 0xff}, /* 171 */
	{0xd7, 0x87, 0x00}, /* 172 */
	{0xd7, 0x87, 0x5f}, /* 173 */
	{0xd7, 0x87, 0x87}, /* 174 */
	{0xd7, 0x87, 0xaf}, /* 175 */
	{0xd7, 0x87, 0xd7}, /* 176 */
	{0xd7, 0x87, 0xff}, /* 177 */
	{0xd7, 0xaf, 0x00}, /* 178 */
	{0xd7, 0xaf, 0x5f}, /* 179 */
	{0xd7, 0xaf, 0x87}, /* 180 */
	{0xd7, 0xaf, 0xaf}, /* 181 */
	{0xd7, 0xaf, 0xd7}, /* 182 */
	{0xd7, 0xaf, 0xff}, /* 183 */
	{0xd7, 0xd7, 0x00}, /* 184 */
	{0xd7, 0xd7, 0x5f}, /* 185 */
	{0xd7, 0xd7, 0x87}, /* 186 */
	{0xd7, 0xd7, 0xaf}, /* 187 */
	{0xd7, 0xd7, 0xd7}, /* 188 */
	{0xd7, 0xd7, 0xff}, /* 189 */
	{0xd7, 0xff, 0x00}, /* 190 */
	{0xd7, 0xff, 0x5f}, /* 191 */
	{0xd7, 0xff, 0x87}, /* 192 */
	{0xd7, 0xff, 0xaf}, /* 193 */
	{0xd7, 0xff, 0xd7}, /* 194 */
	{0xd7, 0xff, 0xff}, /* 195 */

	{0xff, 0x00, 0x00}, /* 196 */
	{0xff, 0x00, 0x5f}, /* 197 */
	{0xff, 0x00, 0x87}, /* 198 */
	{0xff, 0x00, 0xaf}, /* 199 */
	{0xff, 0x00, 0xd7}, /* 200 */
	{0xff, 0x00, 0xff}, /* 201 */
	{0xff, 0x5f, 0x00}, /* 202 */
	{0xff, 0x5f, 0x5f}, /* 203 */
	{0xff, 0x5f, 0x87}, /* 204 */
	{0xff, 0x5f, 0xaf}, /* 205 */
	{0xff, 0x5f, 0xd7}, /* 206 */
	{0xff, 0x5f, 0xff}, /* 207 */
	{0xff, 0x87, 0x00}, /* 208 */
	{0xff, 0x87, 0x5f}, /* 209 */
	{0xff, 0x87, 0x87}, /* 210 */
	{0xff, 0x87, 0xaf}, /* 211 */
	{0xff, 0x87, 0xd7}, /* 212 */
	{0xff, 0x87, 0xff}, /* 213 */
	{0xff, 0xaf, 0x00}, /* 214 */
	{0xff, 0xaf, 0x5f}, /* 215 */
	{0xff, 0xaf, 0x87}, /* 216 */
	{0xff, 0xaf, 0xaf}, /* 217 */
	{0xff, 0xaf, 0xd7}, /* 218 */
	{0xff, 0xaf, 0xff}, /* 219 */
	{0xff, 0xd7, 0x00}, /* 220 */
	{0xff, 0xd7, 0x5f}, /* 221 */
	{0xff, 0xd7, 0x87}, /* 222 */
	{0xff, 0xd7, 0xaf}, /* 223 */
	{0xff, 0xd7, 0xd7}, /* 224 */
	{0xff, 0xd7, 0xff}, /* 225 */
	{0xff, 0xff, 0x00}, /* 226 */
	{0xff, 0xff, 0x5f}, /* 227 */
	{0xff, 0xff, 0x87}, /* 228 */
	{0xff, 0xff, 0xaf}, /* 229 */
	{0xff, 0xff, 0xd7}, /* 230 */
	{0xff, 0xff, 0xff}, /* 231 */

	{0x08, 0x08, 0x08}, /* 232 */
	{0x12, 0x12, 0x12}, /* 233 */
	{0x1c, 0x1c, 0x1c}, /* 234 */
	{0x26, 0x26, 0x26}, /* 235 */
	{0x30, 0x30, 0x30}, /* 236 */
	{0x3a, 0x3a, 0x3a}, /* 237 */
	{0x44, 0x44, 0x44}, /* 238 */
	{0x4e, 0x4e, 0x4e}, /* 239 */
	{0x58, 0x58, 0x58}, /* 240 */
	{0x62, 0x62, 0x62}, /* 241 */
	{0x6c, 0x6c, 0x6c}, /* 242 */
	{0x76, 0x76, 0x76}, /* 243 */
	{0x80, 0x80, 0x80}, /* 244 */
	{0x8a, 0x8a, 0x8a}, /* 245 */
	{0x94, 0x94, 0x94}, /* 246 */
	{0x9e, 0x9e, 0x9e}, /* 247 */
	{0xa8, 0xa8, 0xa8}, /* 248 */
	{0xb2, 0xb2, 0xb2}, /* 249 */
	{0xbc, 0xbc, 0xbc}, /* 250 */
	{0xc6, 0xc6, 0xc6}, /* 251 */
	{0xd0, 0xd0, 0xd0}, /* 252 */
	{0xda, 0xda, 0xda}, /* 253 */
	{0xe4, 0xe4, 0xe4}, /* 254 */
	{0xee, 0xee, 0xee}, /* 255 */
};

static bool firstShell = true;

u16 VTerm::init_history_lines()
{
	u32 val = 1000;
	Config::instance()->getOption("history-lines", val);
	if (val > 65535) val = 65535;
	return val;
}

u8 VTerm::init_default_color(bool foreground)
{
	u32 color;

	if (foreground) {
		color = 7;
		Config::instance()->getOption("color-foreground", color);
		if (color > 7) color = 7;
	} else {
		color = 0;
		Config::instance()->getOption("color-background", color);
		if (color > 7) color = 0;
	}

	return color;
}

bool VTerm::init_ambiguous_wide()
{
	bool val = false;
	Config::instance()->getOption("ambiguous-wide", val);
	return val;
}

void Shell::initWordChars(s8 *buf, u32 len)
{
	Config::instance()->getOption("word-chars", buf, len);
}

FbShell::FbShell()
{
	mImProxy = 0;
	mPaletteChanged = false;
	mPalette = 0;
	createShellProcess(Config::instance()->getShellCommand());
	resize(screen->cols(), screen->rows());

	firstShell = false;
}

FbShell::~FbShell()
{
	if (mImProxy) delete mImProxy;

	manager->shellExited(this);
	if (mPalette) delete[] mPalette;
}

void FbShell::drawChars(CharAttr attr, u16 x, u16 y, u16 w, u16 num, u16 *chars, bool *dws)
{
	if (manager->activeShell() != this) return;

	adjustCharAttr(attr);
	screen->drawText(FW(x), FH(y), attr.fcolor, attr.bcolor, num, chars, dws);

	if (mImProxy) {
		Rectangle rect = { FW(x), FH(y), FW(w), FH(1) };
		mImProxy->redrawImWin(rect);
	}
}

bool FbShell::moveChars(u16 sx, u16 sy, u16 dx, u16 dy, u16 w, u16 h)
{
	if (manager->activeShell() != this) return true;
	return screen->move(sx, sy, dx, dy, w, h);
}

void FbShell::drawCursor(CharAttr attr, u16 x, u16 y, u16 c)
{
	u16 oldX = mCursor.x, oldY = mCursor.y;

	adjustCharAttr(attr);
	mCursor.attr = attr;
	mCursor.x = x;
	mCursor.y = y;
	mCursor.code = c;
	mCursor.showed = false;

	updateCursor();

	if (manager->activeShell() == this && (oldX != x || oldY != y)) {
		reportCursor();
	}
}

void FbShell::updateCursor()
{
	if (manager->activeShell() != this || mCursor.x >= w() || mCursor.y >= h()) return;
	mCursor.showed ^= true;

	u16 shape = mode(CursorShape);
	if (shape == CurDefault) {
		static bool inited = false;
		static u32 default_shape = 0;
		if (!inited) {
			inited = true;
			Config::instance()->getOption("cursor-shape", default_shape);

			if (!default_shape) default_shape = CurUnderline;
			else default_shape = CurBlock;
		}

		shape = default_shape;
	}

	switch (shape) {
	case CurNone:
		break;

	case CurUnderline:
		screen->fillRect(FW(mCursor.x), FH(mCursor.y + 1) - 1, FW(1), 1, mCursor.showed ? mCursor.attr.fcolor : mCursor.attr.bcolor);
		if (mImProxy) {
			Rectangle rect = { FW(mCursor.x), FH(mCursor.y + 1) - 1, FW(1), 1 };
			mImProxy->redrawImWin(rect);
		}
		break;

	default: {
		bool dw = (mCursor.attr.type != CharAttr::Single);

		u16 x = mCursor.x;
		if (mCursor.attr.type == CharAttr::DoubleRight) x--;

		CharAttr attr = mCursor.attr;
		if (mCursor.showed) {
			u8 temp = attr.fcolor;
			attr.fcolor = attr.bcolor;
			attr.bcolor = temp;
		}

		drawChars(attr, x, mCursor.y, dw ? FW(2) : FW(1), 1, &mCursor.code, &dw);
		break;
	}
	}
}

void FbShell::enableCursor(bool enable)
{
	static u32 interval = 500;
	static bool inited = false;
	if (!inited) {
		inited = true;
		Config::instance()->getOption("cursor-interval", interval);
	}

	if (!interval) return;

	static bool enabled = false;
	if (enabled == enable) return;
	enabled = enable;

	u32 val = (enable ? interval : 0);
	u32 sec = val / 1000, usec = (val % 1000) * 1000;

	struct itimerval timer;
	timer.it_interval.tv_usec = usec;
	timer.it_interval.tv_sec = sec;
	timer.it_value.tv_usec = usec;
	timer.it_value.tv_sec = sec;

	setitimer(ITIMER_REAL, &timer, NULL);
}

void FbShell::modeChanged(ModeType type)
{
	if (manager->activeShell() != this) return;

	if (type & (CursorVisible | CursorShape)) {
		enableCursor(mode(CursorVisible) && mode(CursorShape) != CurNone);
	}

	if (type & CursorKeyEscO) {
		changeMode(CursorKeyEscO, mode(CursorKeyEscO));
	}

	if (type & AutoRepeatKey) {
		changeMode(AutoRepeatKey, mode(AutoRepeatKey));
	}

	if (type & ApplicKeypad) {
		changeMode(ApplicKeypad, mode(ApplicKeypad));
	}

	if (type & CRWithLF) {
		changeMode(CRWithLF, mode(CRWithLF));
	}

	if (type & (CursorKeyEscO | ApplicKeypad | CRWithLF)) {
		reportMode();
	}
}

void FbShell::request(RequestType type,  u32 val)
{
	bool active = (manager->activeShell() == this);

	switch (type) {
	case PaletteSet:
		if ((val >> 24) >= NR_COLORS) break;

		if (!mPaletteChanged) {
			mPaletteChanged = true;

			if (!mPalette) mPalette = new Color[NR_COLORS];
			memcpy(mPalette, defaultPalette, sizeof(defaultPalette));
		}

		mPalette[val >> 24].red = (val >> 16) & 0xff;
		mPalette[val >> 24].green = (val >> 8) & 0xff;
		mPalette[val >> 24].blue = val & 0xff;

		if (active) {
			screen->setPalette(mPalette);
		}
		break;

	case PaletteClear:
		if (!mPaletteChanged) break;
		mPaletteChanged = false;

		if (active) {
			screen->setPalette(defaultPalette);
		}
		break;

	case VcSwitch:
		break;

	default:
		break;
	}
}

static s32 tty0_fd = -1;

void FbShell::switchVt(bool enter, FbShell *peer)
{
	if (tty0_fd == -1) tty0_fd = open("/dev/tty0", O_RDWR);
	if (tty0_fd != -1) {
		seteuid(0);
		ioctl(tty0_fd, TIOCCONS, 0);
		if (enter) {
			s32 slavefd = open(ptsname(fd()), O_RDWR);
			ioctl(slavefd, TIOCCONS, 0);
			close(slavefd);
		}
		seteuid(getuid());
	}

	if (mImProxy) {
		mImProxy->switchVt(enter, peer ? peer->mImProxy : 0);
	}

	if (enter) {
		screen->setPalette(mPaletteChanged ? mPalette : defaultPalette);
		modeChanged(AllModes);
		reportCursor();
	} else if (!peer) {
		changeMode(CursorKeyEscO, false);
		changeMode(ApplicKeypad, false);
		changeMode(CRWithLF, false);
		changeMode(AutoRepeatKey, true);

		enableCursor(false);
		screen->setPalette(defaultPalette);
	}
}

void FbShell::initShellProcess()
{
	if (tty0_fd != -1) close(tty0_fd);
	FbTerm::instance()->initChildProcess();

	if (!firstShell) return;

	bool verbose = false;
	Config::instance()->getOption("verbose", verbose);

	TtyInput::instance()->showInfo(verbose);
	Screen::instance()->showInfo(verbose);
	Font::instance()->showInfo(verbose);

	if (verbose) {
		printf("[term] size: %dx%d, default codec: %s\n", screen->cols(), screen->rows(), localCodec());
	}
}

void FbShell::switchCodec(u8 index)
{
	if (!index) {
		setCodec("UTF-8", localCodec());
		return;
	}

	#define NR_CODECS 5
	if (index > NR_CODECS) return;

	static s8 buf[128], *codecs[NR_CODECS];
	static bool inited = false;
	if (!inited) {
		inited = true;
		Config::instance()->getOption("text-encodings", buf, sizeof(buf));
		if (!*buf) return;

		s8 *cur = buf, *next;
		u8 i = 0;
		while (1) {
			next = strchr(cur, ',');
			codecs[i++] = cur;

			if (!next) break;
			*next = 0;

			if (i == NR_CODECS) break;

			cur = next + 1;
		}
	}

	if (codecs[index - 1]) {
		setCodec("UTF-8", codecs[index - 1]);
	}
}

void FbShell::keyInput(s8 *buf, u32 len)
{
	if (mImProxy && mImProxy->actived()) {
		mImProxy->sendKey(buf, len);
	} else {
		imInput(buf, len);
	}
}

void FbShell::mouseInput(u16 x, u16 y, s32 type, s32 buttons)
{
	if (type == Move) {
		clearMousePointer();

		CharAttr attr = charAttr(x, y);
		adjustCharAttr(attr);

		bool dw = (attr.type != CharAttr::Single);
		u16 code = charCode(x, y);

		if (attr.type == CharAttr::DoubleRight) x--;
		screen->drawText(FW(x), FH(y), attr.bcolor, attr.fcolor, 1, &code, &dw);

		mMousePointer.x = x;
		mMousePointer.y = y;
		mMousePointer.drawed = true;
	}

	Shell::mouseInput(x, y, type, buttons);
}

void FbShell::readyRead(s8 *buf, u32 len)
{
	clearMousePointer();
	Shell::readyRead(buf, len);
}

void FbShell::clearMousePointer()
{
	if (mMousePointer.drawed) {
		mMousePointer.drawed = false;
		expose(mMousePointer.x, mMousePointer.y, 1, 1);
	}
}

void FbShell::expose(u16 x, u16 y, u16 w, u16 h)
{
	VTerm::expose(x, y, w, h);

	if (mode(CursorVisible) && mCursor.y >= y && mCursor.y < (y + h) && mCursor.x >= x && mCursor.x < (x + w)) {
		mCursor.showed = false;
		updateCursor();
	}
}

void FbShell::adjustCharAttr(CharAttr &attr)
{
	if (attr.italic) attr.fcolor = 2; // green
	else if (attr.underline) attr.fcolor = 6; // cyan
	else if (attr.intensity == 0) attr.fcolor = 8; // gray

	if (attr.blink && attr.bcolor < 8) attr.bcolor ^= 8;
	if (attr.intensity == 2 && attr.fcolor < 8) attr.fcolor ^= 8;

	if (attr.reverse) {
		u16 temp = attr.bcolor;
		attr.bcolor = attr.fcolor;
		attr.fcolor = temp;

		if (attr.bcolor > 8 && attr.bcolor < 16) attr.bcolor -= 8;
	}
}

void FbShell::changeMode(ModeType type, u16 val)
{
	const s8 *str = 0;

	if (type == CursorKeyEscO) str = (val ? "\e[?1h" : "\e[?1l");
	else if (type == AutoRepeatKey) str = (val ? "\e[?8h" : "\e[?8l");
	else if (type == ApplicKeypad) str = (val ? "\e=" : "\e>");
	else if (type == CRWithLF) str = (val ? "\e[20h" : "\e[20l");

	if (str) {
		s32 ret = ::write(STDIN_FILENO, str, strlen(str));
	}
}

void FbShell::reportCursor()
{
	if (mImProxy) mImProxy->changeCursorPos(mCursor.x, mCursor.y);
}

void FbShell::reportMode()
{
	if (mImProxy) mImProxy->changeTermMode(mode(CRWithLF), mode(ApplicKeypad), mode(CursorKeyEscO));
}

void FbShell::killIm()
{
	if (mImProxy) delete mImProxy;
}
void FbShell::toggleIm()
{
	if (!mImProxy) mImProxy = new ImProxy(this);

	if (mImProxy) {
		mImProxy->toggleActive();
		reportCursor();
		reportMode();
	}
}

void FbShell::imInput(s8 *buf, u32 len)
{
	clearMousePointer();
	Shell::keyInput(buf, len);
}

bool FbShell::childProcessExited(s32 pid)
{
	if (mImProxy && pid == mImProxy->imProcessId()) {
		delete mImProxy;
		return true;
	}

	if (pid == shellProcessId()) {
		delete this;
		return true;
	}

	return false;
}
