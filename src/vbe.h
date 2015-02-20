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

#ifndef VBE_H
#define VBE_H

#define VBE_FUN_GET_INFO 0x4f00
#define VBE_FUN_GET_MODE_INFO 0x4f01
#define VBE_FUN_SET_MODE 0x4f02
#define VBE_FUN_GET_CURRENT_MODE 0x4f03
#define VBE_FUN_SAVE_RESTORE_STATE 0x4f04
#define VBE_FUN_DISPLAY_WIN_CONTROL 0x4f05
#define VBE_FUN_LOGICAL_SCANLINE 0x4f06
#define VBE_FUN_DISPLAY_START 0x4f07
#define VBE_FUN_PALETTE_FORMAT 0x4f08
#define VBE_FUN_PALETTE_DATA 0x4f09

#define VBE_ATTR_MODE_SUPPORTED 	(1 << 0)
#define VBE_ATTR_TTY 	(1 << 2)
#define VBE_ATTR_COLOR 	(1 << 3)
#define VBE_ATTR_GRAPHICS 	(1 << 4)
#define VBE_ATTR_NOT_VGA 	(1 << 5)
#define VBE_ATTR_NOT_WINDOWED 	(1 << 6)
#define VBE_ATTR_LINEAR 	(1 << 7)

#define VBE_MODEL_TEXT 	0
#define VBE_MODEL_CGA 	1
#define VBE_MODEL_HERCULES 	2
#define VBE_MODEL_PLANAR 	3
#define VBE_MODEL_PACKED 	4
#define VBE_MODEL_256 	5
#define VBE_MODEL_RGB 	6
#define VBE_MODEL_YUV 	7

/* structures for vbe 3.0 */

struct vbe_info_block {
	char vbe_signature[4];
	short vbe_version;
	unsigned short oem_string_off;
	unsigned short oem_string_seg;
	int capabilities;
	unsigned short video_mode_list_off;
	unsigned short video_mode_list_seg;
	short total_memory;
	short oem_software_rev;
	unsigned short oem_vendor_name_off;
	unsigned short oem_vendor_name_seg;
	unsigned short oem_product_name_off;
	unsigned short oem_product_name_seg;
	unsigned short oem_product_rev_off;
	unsigned short oem_product_rev_seg;
	char reserved[222];
	char oem_data[256];
} __attribute__ ((packed));

struct vbe_mode_info_block {
	unsigned short mode_attributes;
	unsigned char win_a_attributes;
	unsigned char win_b_attributes;
	unsigned short win_granularity;
	unsigned short win_size;
	unsigned short win_a_segment;
	unsigned short win_b_segment;
	unsigned short win_func_ptr_off;
	unsigned short win_func_ptr_seg;
	unsigned short bytes_per_scanline;

	unsigned short x_resolution;
	unsigned short y_resolution;
	unsigned char x_char_size;
	unsigned char y_char_size;
	unsigned char number_of_planes;
	unsigned char bits_per_pixel;
	unsigned char number_of_banks;
	unsigned char memory_model;
	unsigned char bank_size;
	unsigned char number_of_image_pages;
	unsigned char res1;

	unsigned char red_mask_size;
	unsigned char red_field_position;
	unsigned char green_mask_size;
	unsigned char green_field_position;
	unsigned char blue_mask_size;
	unsigned char blue_field_position;
	unsigned char rsvd_mask_size;
	unsigned char rsvd_field_position;
	unsigned char direct_color_mode_info;

	unsigned int phys_base_ptr;
	unsigned int offscreen_mem_offset;
	unsigned short offscreen_mem_size;

	unsigned short lin_bytes_per_scanline;
	unsigned char bnk_number_of_image_page;
	unsigned char lin_number_of_image_pages;
	unsigned char lin_red_mask_size;
	unsigned char lin_red_field_position;
	unsigned char lin_green_mask_size;
	unsigned char lin_green_field_position;
	unsigned char lin_blue_mask_size;
	unsigned char lin_blue_field_position;
	unsigned char lin_rsvd_mask_size;
	unsigned char lin_rsvd_field_position;
	unsigned int max_pixel_clock;
	unsigned char res2[189];
} __attribute__ ((packed));

struct vbe_palette_entry {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char align;
} __attribute__ ((packed));

#endif
