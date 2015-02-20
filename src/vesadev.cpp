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

#include "config.h"
#ifdef ENABLE_VESA

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/io.h>
#include <sys/mman.h>
extern "C" {
#include <libx86.h>
}
#include "vesadev.h"
#include "vbe.h"
#include "fbterm.h"

#define MAX_VIDEO_MEM (16 * 1024 * 1024)
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static s16 cur_mode;
static vbe_mode_info_block mode_info;
static LRMI_regs r;
static u32 scanline_width, scanline_height;

extern u32 effective_uid;

static bool call_bios()
{
	bool success = true;
	u16 fun = r.eax;

	if (!LRMI_int(0x10, &r)) success = false;
	if ((fun >> 8) == 0x4f && (r.eax & 0xffff) != 0x4f) success = false;

	if (!success) {
		fprintf(stderr, "call VESA function 0x%x failure!\n", fun);
	}

	return success;
}

static void save_restore_state(bool save)
{
	static void *state_buf;
	static u32 state_size;
	static bool saved = false;

	if (save) {
		if (saved) return;

		r.eax = VBE_FUN_SAVE_RESTORE_STATE;
		r.ecx = 0xf; 	/* all states */
		r.edx = 0; 	/* get buffer size */

		if (!call_bios()) return;

		state_size = (r.ebx & 0xffff) * 64;
		state_buf = malloc(state_size);

		void *buf = LRMI_alloc_real(state_size);

		r.eax = VBE_FUN_SAVE_RESTORE_STATE;
		r.ecx = 0xf; 	/* all states */
		r.edx = 1; 	/* save state */
		r.es = (long)buf >> 4;
		r.ebx = (long)buf & 0xf;

		if (call_bios()) {
			saved = true;
			memcpy(state_buf, buf, state_size);
		}

		LRMI_free_real(buf);
	} else if (saved) {
		void *buf = LRMI_alloc_real(state_size);
		memcpy(buf, state_buf, state_size);

		r.eax = VBE_FUN_SAVE_RESTORE_STATE;
		r.ecx = 0xf; 	/* all states */
		r.edx = 2; 	/* restore state */
		r.es = (long)buf >> 4;
		r.ebx = (long)buf & 0xf;

		call_bios();
		LRMI_free_real(buf);
	}
}

static void save_restore_mode(bool save)
{
	static s16 mode = -1;

	if (save) {
		r.eax = VBE_FUN_GET_CURRENT_MODE;
		if (call_bios()) mode = r.ebx;
	} else if (mode != -1) {
		r.eax = VBE_FUN_SET_MODE;
		r.ebx = mode;
		call_bios();
	}
}

// input: mode 0 for auto-detect
// return: -1 for failure
static s16 get_mode(s16 mode, bool only_print)
{
	seteuid(0);
	bool ret = (!LRMI_init() || ioperm(0, 1024, 1) || iopl(3));
	seteuid(getuid());

	if (ret) {
		fprintf(stderr, "Using VESA requires root privilege\n");
		return -1;
	}

	vbe_info_block *info = (vbe_info_block *)LRMI_alloc_real(sizeof(*info));

	r.eax = VBE_FUN_GET_INFO;
	r.es = (long)info >> 4;
	memcpy(info->vbe_signature, "VBE2", 4);

	if (!call_bios() || strncmp(info->vbe_signature, "VESA", 4) != 0) {
		fprintf(stderr, "No VESA bios\n");
		LRMI_free_real(info);
		return -1;
	}

	s16 *mode_list = (s16*)(info->video_mode_list_seg * 16 + info->video_mode_list_off);
	vbe_mode_info_block *minfo = (vbe_mode_info_block *)LRMI_alloc_real(sizeof(*minfo));
	u32 xres_tmp, yres_tmp, bpp_tmp;
	s16 mode_tmp = -1;
	bool valid_mode = false;

	for (; *mode_list != -1; mode_list++) {
		r.eax = VBE_FUN_GET_MODE_INFO;
		r.ecx = *mode_list;
		r.es = (long)minfo >> 4;

		if (!call_bios()) continue;

		if (!(minfo->mode_attributes & VBE_ATTR_MODE_SUPPORTED)
			|| !(minfo->mode_attributes & VBE_ATTR_COLOR)
			|| !(minfo->mode_attributes & VBE_ATTR_GRAPHICS)
			|| !(minfo->mode_attributes & VBE_ATTR_LINEAR)) continue;

        switch (minfo->bits_per_pixel) {
        case 8:
                if (minfo->memory_model != VBE_MODEL_PACKED) continue;
                break;
        case 15:
        case 16:
        case 32:
                if (minfo->memory_model != VBE_MODEL_RGB) continue;
                break;
        default:
                continue;
        }

		valid_mode = true;

		if (only_print) {
			printf("[%d]: %dx%d-%dbpp\n", *mode_list,
				minfo->x_resolution, minfo->y_resolution, minfo->bits_per_pixel);
		} else {
			if (mode) {
				if (mode == *mode_list) return mode;
			} else {
				if ((mode_tmp == -1)
					|| (minfo->x_resolution > xres_tmp && minfo->y_resolution > yres_tmp)
					|| (minfo->x_resolution == xres_tmp && minfo->y_resolution == yres_tmp && minfo->bits_per_pixel > bpp_tmp)) {

					mode_tmp = *mode_list;
					xres_tmp = minfo->x_resolution;
					yres_tmp = minfo->y_resolution;
					bpp_tmp = minfo->bits_per_pixel;
				}
			}
		}
	}

	if (!valid_mode) fprintf(stderr, "can't find a valid VESA mode for your video card\n");
	else if (!only_print && mode_tmp == -1) fprintf(stderr, "%d isn't a valid VESA mode for your video card\n", mode);

	LRMI_free_real(info);
	LRMI_free_real(minfo);
	return mode_tmp;
}

void VesaDev::printModes()
{
	get_mode(0, true);
}

VesaDev *VesaDev::initVesaDev(s16 mode)
{
	cur_mode = get_mode(mode, false);
	if (cur_mode == -1) return 0;

	void *minfo = LRMI_alloc_real(sizeof(mode_info));

	r.eax = VBE_FUN_GET_MODE_INFO;
	r.ecx = cur_mode;
	r.es = (long)minfo >> 4;
	call_bios();

	memcpy(&mode_info, minfo, sizeof(mode_info));
	LRMI_free_real(minfo);

	cur_mode |= 0x4000; // use linear frame buffer mode
	return new VesaDev();
}

VesaDev::VesaDev()
{
	mWidth = mode_info.x_resolution;
	mHeight = mode_info.y_resolution;
	mBitsPerPixel = mode_info.bits_per_pixel;

	scanline_width = mWidth;
	scanline_height = mHeight;
}

VesaDev::~VesaDev()
{
	if (mVMemBase) munmap(mVMemBase, mBytesPerLine * scanline_height);
}

const s8 *VesaDev::drvId()
{
	return "VESA";
}

void VesaDev::switchVc(bool enter)
{
	if (enter) {
		static bool inited = false;

		if (!inited) {
			save_restore_mode(true);
			save_restore_state(true);
		}

		r.eax = VBE_FUN_SET_MODE;
		r.ebx = cur_mode;
		call_bios();

		r.eax = VBE_FUN_LOGICAL_SCANLINE;
		r.ebx = 0; // set scan line length in pixels
		r.ecx = scanline_width;
		call_bios();

		if (!inited) {
			inited = true;

			r.eax = VBE_FUN_LOGICAL_SCANLINE;
			r.ebx = 1; // get scan line length
			call_bios(); // return ebx: scan line length in bytes
						 //        ecx: scan line length in pixels
						 //        edx: maximum scan lines

			mBytesPerLine = r.ebx;
			initScrollType();

			seteuid(0);
			s32 fd = open("/dev/mem", O_RDWR);
			seteuid(getuid());

			mVMemBase = (u8 *)mmap(0, mBytesPerLine * scanline_height, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mode_info.phys_base_ptr);
			close(fd);
		}
	} else {
		save_restore_mode(false);
		save_restore_state(false);
	}

	Screen::switchVc(enter);
}

void VesaDev::initScrollType()
{
	r.eax = VBE_FUN_DISPLAY_START;
	r.ebx = 0; // set display start
	r.ecx = 0; // x = 0
	r.edx = 0; // y = 0
	if (!call_bios()) return;

	if (mRotateType == Rotate0 || mRotateType == Rotate180) {
		r.eax = VBE_FUN_LOGICAL_SCANLINE;
		r.ebx = 1; // get scan line length
		call_bios();

		u32 max_height = MIN(r.edx, MAX_VIDEO_MEM / mBytesPerLine);
		if (max_height <= mHeight) return;

		mScrollType = YPan;
		mOffsetMax = max_height - mHeight;
		scanline_height = max_height;
	} else {
		// mWidth/mHeight has been swapped
		u32 width = mHeight, height = mWidth;

		r.eax = VBE_FUN_LOGICAL_SCANLINE;
		r.ebx = 3; // get maximum scan line length
		if (!call_bios()) return;

		u32 max_width = r.ecx;

		r.eax = VBE_FUN_LOGICAL_SCANLINE;
		r.ebx = 0; // set scan line length in pixels
		call_bios();

		r.eax = VBE_FUN_LOGICAL_SCANLINE;
		r.ebx = 1; // get scan line length
		call_bios();

		if (r.ecx <= width || r.edx < height) return;

		scanline_width = max_width;
		mScrollType = XPan;
		mBytesPerLine = r.ebx;
		mOffsetMax = r.ecx - width;
	}
}

void VesaDev::setupOffset()
{
	if (mScrollType != YPan && mScrollType != XPan) return;

	r.eax = VBE_FUN_DISPLAY_START;
	r.ebx = 0; // set display start
	if (mScrollType == YPan) {
		r.ecx = 0;
		r.edx = mOffsetCur;
	} else {
		r.ecx = mOffsetCur;
		r.edx = 0;
	}
	call_bios();
}

void VesaDev::setupPalette(bool restore)
{
	if (mode_info.memory_model != VBE_MODEL_PACKED || restore) return;

	vbe_palette_entry *buf = (vbe_palette_entry *)LRMI_alloc_real(sizeof(*buf) * NR_COLORS);

	for (u32 i = 0; i < NR_COLORS; i++) {
		buf[i].red = mPalette[i].red >> 2;
		buf[i].green = mPalette[i].green >> 2;
		buf[i].blue = mPalette[i].blue >> 2;
	}

	r.eax = VBE_FUN_PALETTE_DATA;
	r.ebx = 0; // set palette data
	r.ecx = NR_COLORS; // number of palette entry
	r.edx = 0; // first palette entry
	r.es = (long)buf >> 4;
	call_bios();

	LRMI_free_real(buf);
}
#endif
