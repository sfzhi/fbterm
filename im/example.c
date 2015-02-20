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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imapi.h"
#include "keycode.h"

static char raw_mode = 1;
static unsigned cursorx, cursory;

static void im_active()
{
	if (raw_mode) {
		init_keycode_state();
	}
}

static void im_deactive()
{
	Rectangle rect = { 0, 0, 0, 0 };
	set_im_window(0, rect);
	set_im_window(1, rect);
}

static void process_raw_key(char *buf, unsigned len)
{
	unsigned i;
	for (i = 0; i < len; i++) {
		char down = !(buf[i] & 0x80);
		short code = buf[i] & 0x7f;

		if (!code) {
			if (i + 2 >= len) break;

			code = (buf[++i] & 0x7f) << 7;
			code |= buf[++i] & 0x7f;
			if (!(buf[i] & 0x80) || !(buf[i - 1] & 0x80)) continue;
		}

		unsigned short keysym = keycode_to_keysym(code, down);
		char *str = keysym_to_term_string(keysym, down);

		put_im_text(str, strlen(str));
	}
}

static void process_key(char *keys, unsigned len)
{
	if (raw_mode) {
		process_raw_key(keys, len);
	} else {
		put_im_text(keys, len);
	}
}

static void im_show(unsigned winid)
{
	static const char str[] = "a IM example";

	Rectangle rect;
	rect.x = cursorx + 10, rect.y = cursory + 10;
	rect.w = 40, rect.h = 20;

	set_im_window(0, rect);
	fill_rect(rect, Gray);

	rect.y += rect.h + 10;
	rect.w = 180;
	rect.h = 30;

	set_im_window(1, rect);
	fill_rect(rect, Gray);
	draw_text(rect.x + 5, rect.y + 5, Black, Gray, str, sizeof(str) - 1);
}

static void im_hide()
{
}

static void cursor_pos_changed(unsigned x, unsigned y)
{
	cursorx = x;
	cursory = y;
	im_show(-1);
}

static void update_fbterm_info(Info *info)
{
}

static ImCallbacks cbs = {
	im_active, // .active
	im_deactive, // .deactive
	im_show,	 // .show_ui
	im_hide, // .hide_ui
	process_key, // .send_key
	cursor_pos_changed, // .cursor_position
	update_fbterm_info, // .fbterm_info
	update_term_mode // .term_mode
};

int main()
{
	register_im_callbacks(cbs);
	connect_fbterm(raw_mode);
	while (check_im_message()) ;

	return 0;
}
