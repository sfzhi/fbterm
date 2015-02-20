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

#include <stdlib.h>
#include <string.h>
#include "screen.h"
#include "fbconfig.h"

#define writeb(addr, val) (*(volatile u8 *)(addr) = (val))
#define writew(addr, val) (*(volatile u16 *)(addr) = (val))
#define writel(addr, val) (*(volatile u32 *)(addr) = (val))

static u32 bytes_per_pixel;
static u32 ppl, ppw, ppb;
static u32 fillColors[NR_COLORS];

static u8 *bgimage_mem;
static u8 bgcolor;

void Screen::setPalette(const Color *palette)
{
	if (mPalette == palette) return;
	mPalette = palette;

	for (u32 i = 0; i < NR_COLORS; i++) {
		switch (mBitsPerPixel) {
		case 8:
			fillColors[i] = (i << 24) | (i << 16) | (i << 8) | i;
			break;
		case 15:
			fillColors[i] = ((palette[i].red >> 3) << 10) | ((palette[i].green >> 3) << 5) | (palette[i].blue >> 3);
			fillColors[i] |= fillColors[i] << 16;
			break;
		case 16:
			fillColors[i] = ((palette[i].red >> 3) << 11) | ((palette[i].green >> 2) << 5) | (palette[i].blue >> 3);
			fillColors[i] |= fillColors[i] << 16;
			break;
		case 32:
			fillColors[i] = (palette[i].red << 16) | (palette[i].green << 8) | palette[i].blue;
			break;
		}
	}

	setupPalette(false);
	eraseMargin(true, mRows);
}

void Screen::initFillDraw()
{
	if (mBitsPerPixel == 15) bytes_per_pixel = 2;
	else bytes_per_pixel = (mBitsPerPixel >> 3);

	ppl = 4 / bytes_per_pixel;
	ppw = ppl >> 1;
	ppb = ppl >> 2;

	bool bg = false;
	if (getenv("FBTERM_BACKGROUND_IMAGE")) {
		bg = true;
		mScrollType = Redraw;

		u32 color = 0;
		Config::instance()->getOption("color-background", color);
		if (color > 7) color = 0;
		bgcolor = color;

		u32 size = mBytesPerLine * ((mRotateType == Rotate0 || mRotateType == Rotate180) ? mHeight : mWidth);
		bgimage_mem = new u8[size];
		memcpy(bgimage_mem, mVMemBase, size);
	}

	fill = bg ? &Screen::fillXBg : &Screen::fillX;

	switch (mBitsPerPixel) {
	case 8:
		draw = bg ? &Screen::draw8Bg : &Screen::draw8;
		break;
	case 15:
		draw = bg ? &Screen::draw15Bg : &Screen::draw15;
		break;
	case 16:
		draw = bg ? &Screen::draw16Bg : &Screen::draw16;
		break;
	case 32:
		draw = bg ? &Screen::draw32Bg : &Screen::draw32;
		break;
	}
}

void Screen::endFillDraw()
{
	if (bgimage_mem) delete[] bgimage_mem;
}

void Screen::fillX(u32 x, u32 y, u32 w, u8 color)
{
	u32 c = fillColors[color];
	u8 *dst = mVMemBase + y * mBytesPerLine + x * bytes_per_pixel;

	// get better performance if write-combining not enabled for video memory
	for (u32 i = w / ppl; i--; dst += 4) {
		writel(dst, c);
	}

	if (w & ppw) {
		writew(dst, c);
		dst += 2;
	}

	if (w & ppb) {
		writeb(dst, c);
	}
}

void Screen::fillXBg(u32 x, u32 y, u32 w, u8 color)
{
	if (color == bgcolor) {
		u32 offset = y * mBytesPerLine + x * bytes_per_pixel;
		memcpy(mVMemBase + offset, bgimage_mem + offset, w * bytes_per_pixel);
	} else {
		fillX(x, y, w, color);
	}
}

void Screen::draw8(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap)
{
	bool isfg;
	u8 *dst = mVMemBase + y * mBytesPerLine + x * bytes_per_pixel;

	for (; w--; pixmap++, dst++) {
		isfg = (*pixmap & 0x80);
		writeb(dst, fillColors[isfg ? fc : bc]);
	}
}

void Screen::draw8Bg(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap)
{
	if (bc != bgcolor) {
		draw8(x, y, w, fc, bc, pixmap);
		return;
	}

	bool isfg;
	u32 offset = y * mBytesPerLine + x * bytes_per_pixel;
	u8 *dst = mVMemBase + offset;
	u8 *bgimg = bgimage_mem + offset;

	for (; w--; pixmap++, dst++, bgimg++) {
		isfg = (*pixmap & 0x80);
		writeb(dst, isfg ? fillColors[fc] : (*bgimg));
	}
}

#define drawX(bits, lred, lgreen, lblue, type, fbwrite) \
 \
void Screen::draw##bits(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap) \
{ \
	u8 red, green, blue; \
	u8 pixel; \
	type color; \
	type *dst = (type *)(mVMemBase + y * mBytesPerLine + x * bytes_per_pixel); \
 \
	for (; w--; pixmap++, dst++) { \
		pixel = *pixmap; \
 \
		if (!pixel) fbwrite(dst, fillColors[bc]); \
		else if (pixel == 0xff) fbwrite(dst, fillColors[fc]); \
		else { \
			red = mPalette[bc].red + (((mPalette[fc].red - mPalette[bc].red) * pixel) >> 8); \
			green = mPalette[bc].green + (((mPalette[fc].green - mPalette[bc].green) * pixel) >> 8); \
			blue = mPalette[bc].blue + (((mPalette[fc].blue - mPalette[bc].blue) * pixel) >> 8); \
 \
			color = ((red >> (8 - lred) << (lgreen + lblue)) | (green >> (8 - lgreen) << lblue) | (blue >> (8 - lblue))); \
			fbwrite(dst, color); \
		} \
	} \
}

drawX(15, 5, 5, 5, u16, writew)
drawX(16, 5, 6, 5, u16, writew)
drawX(32, 8, 8, 8, u32, writel)

#define drawXBg(bits, lred, lgreen, lblue, type, fbwrite) \
 \
void Screen::draw##bits##Bg(u32 x, u32 y, u32 w, u8 fc, u8 bc, u8 *pixmap) \
{ \
	if (bc != bgcolor) { \
		draw##bits(x, y, w, fc, bc, pixmap); \
		return; \
	} \
 \
	u8 red, green, blue; \
	u8 redbg, greenbg, bluebg; \
	u8 pixel; \
	type color; \
 \
	u32 offset = y * mBytesPerLine + x * bytes_per_pixel; \
	type *dst = (type *)(mVMemBase + offset); \
	type *bgimg = (type *)(bgimage_mem + offset); \
 \
	for (; w--; pixmap++, dst++, bgimg++) { \
		pixel = *pixmap; \
 \
		if (!pixel) fbwrite(dst, *bgimg); \
		else if (pixel == 0xff) fbwrite(dst, fillColors[fc]); \
		else { \
			color = *bgimg; \
 \
			redbg = ((color >> (lgreen + lblue)) & ((1 << lred) - 1)) << (8 - lred); \
			greenbg = ((color >> lblue) & ((1 << lgreen) - 1)) << (8 - lgreen); \
			bluebg = (color & ((1 << lblue) - 1)) << (8 - lblue); \
 \
			red = redbg + (((mPalette[fc].red - redbg) * pixel) >> 8); \
			green = greenbg + (((mPalette[fc].green - greenbg) * pixel) >> 8); \
			blue = bluebg + (((mPalette[fc].blue - bluebg) * pixel) >> 8); \
 \
			color = ((red >> (8 - lred) << (lgreen + lblue)) | (green >> (8 - lgreen) << lblue) | (blue >> (8 - lblue))); \
			fbwrite(dst, color); \
		} \
	} \
}

drawXBg(15, 5, 5, 5, u16, writew)
drawXBg(16, 5, 6, 5, u16, writew)
drawXBg(32, 8, 8, 8, u32, writel)
