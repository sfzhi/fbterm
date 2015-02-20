/*
 *   Copyright © 2008-2010 dragchan <zgchan317@gmail.com>
 *   This file is part of FbTerm.
 *   based on GTerm by Timothy Miller <tim@techsource.com>
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
#include <stdio.h>
#include "vterm.h"

void VTerm::cr()
{
	move_cursor(0, cursor_y);
}

void VTerm::lf()
{
	if (cursor_y < scroll_bot) move_cursor(cursor_x, cursor_y + 1);
	else scroll_region(scroll_top, scroll_bot, 1);

	if (mode_flags.crlf) cr();
}

void VTerm::bs()
{
	if (cursor_x) move_cursor(cursor_x - 1, cursor_y);
}

void VTerm::bell()
{
	request(Bell);
}

void VTerm::tab()
{
	u16 x = 0;

	for (u16 i=cursor_x + 1; i < width; i++) {
		s8 a = tab_stops[i/8];
		if (a && (a & (1 << (i % 8)))) {
			x = i;
			break;
		}
	}

	if (!x) x = (cursor_x + 8) & -8;

	if (x < width) move_cursor(x, cursor_y);
	else move_cursor(width - 1, cursor_y);
}

void VTerm::set_tab()
{
	tab_stops[cursor_x / 8] |=  (1 << (cursor_x % 8));
}

void VTerm::clear_tab()
{
	if (param[0] == 3) {
		memset(tab_stops, 0, max_width / 8 + 1);
	} else if (param[0] == 0) {
		tab_stops[cursor_x / 8] &= ~(1 << (cursor_x % 8));
	}
}

void VTerm::param_digit()
{
	param[npar] = param[npar] * 10 + cur_char - '0';
}

void VTerm::next_param()
{
	if (npar < NPAR - 1) npar++;
}

void VTerm::clear_param()
{
	q_mode = 0;
	palette_mode = 0;
	npar = 0;
	memset(param, 0, sizeof(param));
}

void VTerm::save_cursor()
{
	s_g0_charset = g0_charset;
	s_g1_charset = g1_charset;
	s_g0_is_active = g0_is_active;

	s_cursor_x = cursor_x;
	s_cursor_y = cursor_y;
	s_char_attr = char_attr;
}

void VTerm::restore_cursor()
{
	g0_charset = s_g0_charset;
	g1_charset = s_g1_charset;
	g0_is_active = s_g0_is_active;
	charset = (g0_is_active ? g0_charset : g1_charset);

	char_attr = s_char_attr;
	move_cursor(s_cursor_x, s_cursor_y);
}

void VTerm::next_line()
{
	lf();
	cr();
}

void VTerm::index_down()
{
	lf();
}

void VTerm::index_up()
{
	if (cursor_y > scroll_top) move_cursor(cursor_x, cursor_y - 1);
	else scroll_region(scroll_top, scroll_bot, -1);
}

void VTerm::cursor_left()
{
	u16 n, x;

	n = param[0];
	if (n < 1) n = 1;

	if (cursor_x < n) x = 0;
	else x = cursor_x - n;

	move_cursor(x, cursor_y);
}

void VTerm::cursor_right()
{
	u16 n, x;

	n = param[0];
	if (n < 1) n = 1;

	x = cursor_x + n;
	if (x >= width) x = width - 1;

	move_cursor(x, cursor_y);
}

void VTerm::cursor_up()
{
	u16 n, y;

	n = param[0];
	if (n < 1) n = 1;

	if (cursor_y < n) y = 0;
	else y = cursor_y - n;

	move_cursor(cursor_x, y);
}

void VTerm::cursor_down()
{
	u16 n, y;

	n = param[0];
	if (n < 1) n = 1;

	y = cursor_y + n;
	if (y >= height) y = height - 1;

	move_cursor(cursor_x, y);
}

void VTerm::cursor_up_cr()
{
	u16 n, y;

	n = param[0];
	if (n < 1) n = 1;

	if (cursor_y < n) y = 0;
	else y = cursor_y - n;

	move_cursor(0, y);
}

void VTerm::cursor_down_cr()
{
	u16 n, y;

	n = param[0];
	if (n < 1) n = 1;

	y = cursor_y + n;
	if (y >= height) y = height - 1;

	move_cursor(0, y);
}

void VTerm::cursor_position()
{
	u16 x, y;

	x = param[1];
	if (x < 1) x = 1;

	y = param[0];
	if (y < 1) y = 1;

	move_cursor(x - 1, y - 1 + (mode_flags.cursor_relative ? scroll_top : 0));
}

void VTerm::cursor_position_col()
{
	u16 x = param[0];
	if (x < 1) x = 1;

	move_cursor(x-1, cursor_y);
}

void VTerm::cursor_position_row()
{
	u16 y = param[0];
	if (y < 1) y = 1;

	move_cursor(cursor_x, y-1);
}

void VTerm::insert_char()
{
	u16 n, mx;

	n = param[0];
	if (n < 1) n = 1;

	mx = width - cursor_x;

	if (n >= mx) clear_area(cursor_x, cursor_y, width - 1, cursor_y);
	else shift_text(cursor_y, cursor_x, width - 1, n);
}

void VTerm::delete_char()
{
	u16 n, mx;
	n = param[0];
	if (n < 1) n = 1;

	mx = width - cursor_x;

	if (n >= mx) clear_area(cursor_x, cursor_y, width - 1, cursor_y);
	else shift_text(cursor_y, cursor_x, width - 1, -n);
}

void VTerm::erase_char()
{
	u16 n, mx;

	n = param[0];
	if (n < 1) n = 1;

	mx = width - cursor_x;
	if (n > mx) n = mx;

	clear_area(cursor_x, cursor_y, cursor_x + n - 1, cursor_y);
}

void VTerm::insert_line()
{
	u16 n, mx;

	n = param[0];
	if (n < 1) n = 1;

	mx = scroll_bot - cursor_y + 1;

	if (n >= mx) clear_area(0, cursor_y, width - 1, scroll_bot);
	else scroll_region(cursor_y, scroll_bot, -n);
}

void VTerm::delete_line()
{
	u16 n, mx;

	n = param[0];
	if (n < 1) n = 1;

	mx = scroll_bot - cursor_y + 1;

	if (n >= mx) clear_area(0, cursor_y, width - 1, scroll_bot);
	else scroll_region(cursor_y, scroll_bot, n);
}

void VTerm::erase_line()
{
	switch (param[0]) {
	case 0:
		clear_area(cursor_x, cursor_y, width - 1, cursor_y);
		break;
	case 1:
		clear_area(0, cursor_y, cursor_x, cursor_y);
		break;
	case 2:
		clear_area(0, cursor_y, width - 1, cursor_y);
		break;
	}
}

void VTerm::erase_display()
{
	switch (param[0]) {
	case 0:
		clear_area(cursor_x, cursor_y, width - 1, cursor_y);
		if (cursor_y < height - 1) clear_area(0, cursor_y + 1, width - 1, height - 1);
		break;
	case 1:
		clear_area(0, cursor_y, cursor_x, cursor_y);
		if (cursor_y > 0) clear_area(0, 0, width - 1, cursor_y - 1);
		break;
	case 2:
		clear_area(0, 0, width - 1, height - 1);
		break;
	}
}

void VTerm::screen_align()
{
	for (u16 y = 0; y < height; y++) {
		u32 yp = linenumbers[y] * max_width;
		changed_line(y, 0, width - 1);

		for (u16 x = 0; x < width; x++) {
			text[yp + x] = 'E';
			attrs[yp + x] = normal_char_attr();
		}
	}
}

void VTerm::set_margins()
{
	u16 t, b;

	t = param[0];
	if (t < 1) t = 1;

	b = param[1];
	if (b < 1 || b > height) b = height;

	if (pending_scroll) update();

	scroll_top = t - 1;
	scroll_bot = b - 1;
	if (cursor_y < scroll_top) move_cursor(cursor_x, scroll_top);
	if (cursor_y > scroll_bot) move_cursor(cursor_x, scroll_bot);
}

void VTerm::respond_id()
{
	sendBack("\e[?6c"); // response 'I'm a VT102'
}

void VTerm::status_report()
{
	if (param[0] == 5) { // device status report
		sendBack("\e[0n"); // response 'Terminal OK'
	} else if (param[0] == 6) { // cursor position report
		s8 str[32];
		snprintf(str, sizeof(str), "\e[%d;%dR", cursor_y + 1, cursor_x + 1);
		sendBack(str);
	}
}

void VTerm::keypad_numeric()
{
	mode_flags.applic_keypad = false;
	modeChanged(ApplicKeypad);
}

void VTerm::keypad_application()
{
	mode_flags.applic_keypad = true;
	modeChanged(ApplicKeypad);
}

void VTerm::enable_mode(bool enable)
{
	switch (param[0] + 1000 * q_mode) {
	case 3:
		mode_flags.display_ctrl = enable;
		break;
	case 4:
		mode_flags.insert_mode = enable;
		break;
	case 20: // auto echo cr with lf
		mode_flags.crlf = enable;
		modeChanged(CRWithLF);
		break;
	case 1001 :
		mode_flags.cursorkey_esco = enable;
		modeChanged(CursorKeyEscO);
		break;
	case 1003 :
		mode_flags.col_132 = enable;
		break;
	case 1005 :
		mode_flags.inverse_screen = enable;
		for (u16 i = 0; i < height; i++) {
			changed_line(i, 0, width - 1);
		}
		break;
	case 1006 :
		mode_flags.cursor_relative = enable;
		break;
	case 1007 :
		mode_flags.auto_wrap = enable;
		break;
	case 1008 :
		mode_flags.autorepeat_key = enable;
		modeChanged(AutoRepeatKey);
		break;
	case 1009 :
		mode_flags.mouse_report = (enable ? MouseX10 : MouseNone);
		modeChanged(MouseReport);
		break;
	case 1025 :
		mode_flags.cursor_visible = enable;
		modeChanged(CursorVisible);
		break;
	case 2000 :
		mode_flags.mouse_report = (enable ? MouseX11 : MouseNone);
		modeChanged(MouseReport);
		break;
	default:
		break;
	}
}

void VTerm::set_mode()
{
	enable_mode(true);
}

void VTerm::clear_mode()
{
	enable_mode(false);
}

void VTerm::set_display_attr()
{
	for (u16 n = 0; n <= npar; n++) {
		switch (param[n]) {
		case 0:
			char_attr = default_char_attr;
			break;
		case 1:
			char_attr.intensity = 2;
			break;
		case 2:
			char_attr.intensity = 0;
			break;
		case 3:
			char_attr.italic = true;
			break;
		case 4:
			char_attr.underline = true;
			break;
		case 5:
			char_attr.blink = true;
			break;
		case 7:
			char_attr.reverse = true;
			break;
		case 10:
			charset = (g0_is_active ? g0_charset : g1_charset);
			mode_flags.display_ctrl = false;
			mode_flags.toggle_meta = false;
			break;
		case 11:
			charset = IbmpcMap;
			mode_flags.display_ctrl = true;
			mode_flags.toggle_meta = false;
			break;
		case 12:
			charset = IbmpcMap;
			mode_flags.display_ctrl = true;
			mode_flags.toggle_meta = true;
			break;
		case 21:
		case 22:
			char_attr.intensity = 1;
			break;
		case 23:
			char_attr.italic = false;
			break;
		case 24:
			char_attr.underline = false;
			break;
		case 25:
			char_attr.blink = false;
			break;
		case 27:
			char_attr.reverse = false;
			break;
		case 30 ... 37:
			char_attr.fcolor = param[n] % 10;
			break;
		case 38:
			char_attr.fcolor = cur_fcolor;
			char_attr.underline = true;
			break;
		case 39:
			char_attr.fcolor = cur_fcolor;
			char_attr.underline = false;
			break;
		case 40 ... 47:
			char_attr.bcolor = param[n] % 10;
			break;
		case 49:
			char_attr.bcolor = cur_bcolor;
			break;
		default :
			break;
		}
	}
}

void VTerm::set_q_mode()
{
	q_mode = 1;
}

void VTerm::set_cursor_type()
{
	if (q_mode) {
		mode_flags.cursor_shape = param[0];
		modeChanged(CursorShape);
	} else if (!param[0]) {
		respond_id();
	}
}

void VTerm::set_utf8()
{
	utf8 = true;
}

void VTerm::clear_utf8()
{
	utf8 = false;
}

void VTerm::set_charset()
{
	CharsetMap &m = (g0_is_current ? g0_charset : g1_charset);

	switch (cur_char) {
	case '0':
		m = Lat1Map;
		break;
	case 'B':
		m = GrafMap;
		break;
	case 'U':
		m = IbmpcMap;
		break;
	case 'K':
		m = UserMap;
		break;
	default:
		break;
	}
}

void VTerm::active_g0()
{
	charset = g0_charset;
	g0_is_active = true;
	mode_flags.display_ctrl = false;
}

void VTerm::active_g1()
{
	charset = g1_charset;
	g0_is_active = false;
	mode_flags.display_ctrl = true;
}

void VTerm::current_is_g0()
{
	g0_is_current = true;
}

void VTerm::current_is_g1()
{
	g0_is_current = false;
}

void VTerm::linux_specific()
{
	switch (param[0]) {
	case 1:
		if (param[1] < 8) cur_underline_color = param[1];
		break;
	case 2:
		if (param[1] < 8) cur_halfbright_color = param[1];
		break;
	case 8:
		cur_fcolor = char_attr.fcolor;
		cur_bcolor = char_attr.bcolor;
		break;
	case 9:
		request(Blank, param[1]);
		break;
	case 10:
		request(BellFrequencySet, param[1]);
		break;
	case 11:
		request(BellDurationSet,  param[1]);
		break;
	case 12:
		request(VcSwitch, param[1]);
		break;
	case 13:
		request(Unblank);
		break;
	case 14:
		request(VesaPowerIntervalSet,  param[1]);
		break;
	default:
		break;
	}
}

void VTerm::begin_set_palette()
{
	if (palette_mode || npar) esc_state = ESnormal;
	else palette_mode = true;
}

void VTerm::set_palette()
{
	if (palette_mode) {
		param[npar++] = (cur_char >= 'a' ? (cur_char - 'a' + 10) : (cur_char >= 'A' ? (cur_char - 'A' + 10) : (cur_char - '0')));

		if (npar != 7) return;

		u32 val = 0;
		for (u32 i = 0; i < 7; i++) {
			val <<= 4;
			val |= param[i];
		}

		request(PaletteSet, val);
	}

	esc_state = ESnormal;
}

void VTerm::reset_palette()
{
	request(PaletteClear);
}

void VTerm::set_led()
{
	request(param[0] ? LedSet : LedClear,  param[0]);
}

void VTerm::fbterm_specific()
{
	switch (param[0]) {
	case 1:
		if (npar == 1) char_attr.fcolor = param[1];
		break;

	case 2:
		if (npar == 1) char_attr.bcolor = param[1];
		break;

	case 3:
		if (npar == 4) request(PaletteSet, ((param[1] & 0xff) << 24) | ((param[2] & 0xff) << 16) | ((param[3] & 0xff) << 8) | (param[4] & 0xff));
		break;

	default:
		break;
	}
}
