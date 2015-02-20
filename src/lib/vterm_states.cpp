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

#include "vterm.h"

#define ADDSAME(len) ((len) << 8)

const VTerm::Sequence VTerm::control_sequences[] = {
	{ 0,	0,	ESkeep },
	{ 7,	&VTerm::bell,	ESkeep },
	{ 8,	&VTerm::bs,		ESkeep },
	{ 9,	&VTerm::tab,	ESkeep },
	{ 0xA,	&VTerm::lf,	ESkeep },
	{ 0xB,	&VTerm::lf,	ESkeep },
	{ 0xC,	&VTerm::lf,	ESkeep },
	{ 0xD,	&VTerm::cr,	ESkeep },
	{ 0xE,	&VTerm::active_g1,	ESkeep },
	{ 0xF,	&VTerm::active_g0,	ESkeep },
	{ 0x18, 0,	ESnormal },
	{ 0x1A, 0,	ESnormal },
	{ 0x1B, 0,	ESesc },
	{ 0x7F, 0,	ESkeep },
	{ 0x9B, 0,	ESsquare },
	{ -1}
};

const VTerm::Sequence VTerm::escape_sequences[] = {
	{   0, 0, ESnormal },

	// ESnormal
	{ -1 },

	// ESesc
	{ '[', &VTerm::clear_param,	ESsquare },
	{ ']', &VTerm::clear_param,	ESnonstd },
	{ '%', 0,	ESpercent },
	{ '#', 0,	EShash },
	{ '(', &VTerm::current_is_g0,	EScharset },
	{ ')', &VTerm::current_is_g1,	EScharset },
	{ 'c', &VTerm::reset,		ESnormal },
	{ 'D', &VTerm::index_down,	ESnormal },
	{ 'E', &VTerm::next_line,	ESnormal },
	{ 'H', &VTerm::set_tab,		ESnormal },
	{ 'M', &VTerm::index_up,	ESnormal },
	{ 'Z', &VTerm::respond_id,	ESnormal },
	{ '7', &VTerm::save_cursor,	ESnormal },
	{ '8', &VTerm::restore_cursor,	ESnormal },
	{ '>', &VTerm::keypad_numeric,	ESnormal },
	{ '=', &VTerm::keypad_application,	ESnormal },
	{ -1 },

	// ESsquare
	{ '[', 0,	ESfunckey },
	{ '?', &VTerm::set_q_mode,	ESkeep },
	{ '0' | ADDSAME(9), &VTerm::param_digit,	ESkeep },
	{ ';', &VTerm::next_param,	ESkeep },
	{ '@', &VTerm::insert_char,	ESnormal },
	{ 'A', &VTerm::cursor_up,	ESnormal },
	{ 'B', &VTerm::cursor_down,	ESnormal },
	{ 'C', &VTerm::cursor_right,ESnormal },
	{ 'D', &VTerm::cursor_left,	ESnormal },
	{ 'E', &VTerm::cursor_down_cr, ESnormal },
	{ 'F', &VTerm::cursor_up_cr, ESnormal },
	{ 'G', &VTerm::cursor_position_col,	ESnormal },
	{ 'H', &VTerm::cursor_position,	ESnormal },
	{ 'J', &VTerm::erase_display,	ESnormal },
	{ 'K', &VTerm::erase_line,	ESnormal },
	{ 'L', &VTerm::insert_line, ESnormal },
	{ 'M', &VTerm::delete_line,	ESnormal },
	{ 'P', &VTerm::delete_char,	ESnormal },
	{ 'X', &VTerm::erase_char,	ESnormal },
	{ 'a', &VTerm::cursor_right,ESnormal },
	{ 'c', &VTerm::set_cursor_type,	ESnormal },
	{ 'd', &VTerm::cursor_position_row, ESnormal },
	{ 'e', &VTerm::cursor_down,	ESnormal },
	{ 'f', &VTerm::cursor_position,	ESnormal },
	{ 'g', &VTerm::clear_tab,	ESnormal },
	{ 'h', &VTerm::set_mode,	ESnormal },
	{ 'l', &VTerm::clear_mode,	ESnormal },
	{ 'm', &VTerm::set_display_attr,	ESnormal },
	{ 'n', &VTerm::status_report,	ESnormal },
	{ 'q', &VTerm::set_led, ESnormal },
	{ 'r', &VTerm::set_margins,	ESnormal },
	{ 's', &VTerm::save_cursor,	ESnormal },
	{ 'u', &VTerm::restore_cursor,	ESnormal },
	{ '`', &VTerm::cursor_position_col,	ESnormal },
	{ ']', &VTerm::linux_specific, ESnormal },
	{ '}', &VTerm::fbterm_specific, ESnormal },
	{ -1 },

	// ESnonstd
	{ '0' | ADDSAME(9), &VTerm::set_palette,    ESkeep },
	{ 'A' | ADDSAME(5), &VTerm::set_palette,    ESkeep },
	{ 'a' | ADDSAME(5), &VTerm::set_palette,    ESkeep },
	{ 'P', &VTerm::begin_set_palette, ESkeep },
	{ 'R', &VTerm::reset_palette, ESnormal },
	{ -1 },

	// ESpercent
	{ '@', &VTerm::clear_utf8,	ESnormal },
	{ 'G', &VTerm::set_utf8,	ESnormal },
	{ '8', &VTerm::set_utf8,	ESnormal },
	{ -1 },

	// EScharset
	{ '0', &VTerm::set_charset, ESnormal },
	{ 'B', &VTerm::set_charset, ESnormal },
	{ 'U', &VTerm::set_charset, ESnormal },
	{ 'K', &VTerm::set_charset, ESnormal },
	{ -1 },

	// EShash
	{ '8', &VTerm::screen_align,	ESnormal },
	{ -1 },

	// ESfunckey
	{ -1 },
};
