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

#ifndef INPUT_KEY_H
#define INPUT_KEY_H

#include <linux/keyboard.h>

enum Keys {
	AC_START = K(KT_LATIN, 0x80),
	SHIFT_PAGEUP = AC_START,
	SHIFT_PAGEDOWN,
	SHIFT_LEFT,
	SHIFT_RIGHT,
	CTRL_SPACE,
	CTRL_ALT_1,
	CTRL_ALT_2,
	CTRL_ALT_3,
	CTRL_ALT_4,
	CTRL_ALT_5,
	CTRL_ALT_6,
	CTRL_ALT_7,
	CTRL_ALT_8,
	CTRL_ALT_9,
	CTRL_ALT_0,
	CTRL_ALT_C,
	CTRL_ALT_D,
	CTRL_ALT_E,
	CTRL_ALT_F1,
	CTRL_ALT_F2,
	CTRL_ALT_F3,
	CTRL_ALT_F4,
	CTRL_ALT_F5,
	CTRL_ALT_F6,
	CTRL_ALT_K,
	AC_END = CTRL_ALT_K
};

#endif
