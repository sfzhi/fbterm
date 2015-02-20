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


#ifndef VTERM_H
#define VTERM_H

#include "type.h"

class VTerm {
public:
	struct CharAttr {
		typedef enum { Single = 0, DoubleLeft, DoubleRight } CharType;

		bool operator != (const CharAttr a) {
			return fcolor != a.fcolor || bcolor != a.bcolor || intensity != a.intensity
				|| italic != a.italic || underline != a.underline || blink != a.blink || reverse != a.reverse;
		}

		u16 fcolor : 8;
		u16 bcolor : 8;
		u16 intensity : 2; // 0 = half-bright, 1 = normal, 2 = bold
		u16 italic : 1;
		u16 underline : 1;
		u16 blink : 1;
		u16 reverse : 1;
		u16 type : 2;
	};

	typedef enum {
		MouseNone, MouseX11, MouseX10,
	} MouseReportType;

	typedef enum {
		CurDefault, CurNone, CurUnderline, CurLowerThird, CurLowerHalf, CurTwoThirds, CurBlock,
	} CursorType;

	typedef enum {
		CursorVisible = 1,
		CursorShape = 2,
		MouseReport = 4,
		CursorKeyEscO = 8,
		AutoRepeatKey = 16,
		ApplicKeypad = 32,
		CRWithLF = 64,
		AllModes = 0xff,
	} ModeType;

	typedef enum {
		Bell, BellFrequencySet, BellDurationSet,
		PaletteSet, PaletteClear,
		Blank, Unblank,
		LedSet, LedClear,
		VcSwitch, VesaPowerIntervalSet,
	} RequestType;

	VTerm(u16 w = 0, u16 h = 0);
	virtual ~VTerm();

	u16 w() { return width; }
	u16 h() { return height; }
	void historyDisplay(bool absolute, s32 num);
	u16 mode(ModeType type);
	void resize(u16 w, u16 h);
	void input(const u8 *buf, u32 count);
	void expose(u16 x, u16 y, u16 w, u16 h);
	void inverse(u16 sx, u16 sy, u16 ex, u16 ey);

	u16 charCode(u16 x, u16 y) { return text[get_line(y) * max_width + x]; }
	CharAttr charAttr(u16 x, u16 y) { return attrs[get_line(y) * max_width + x]; }

	static s32 charWidth(u32 ucs);

protected:
	virtual void drawChars(CharAttr attr, u16 x, u16 y, u16 w, u16 num, u16 *chars, bool *dws) = 0;
	virtual bool moveChars(u16 sx, u16 sy, u16 dx, u16 dy, u16 w, u16 h) { return false; }
	virtual void drawCursor(CharAttr attr, u16 x, u16 y, u16 c) {}
	virtual void sendBack(const s8 *data) {}
	virtual void modeChanged(ModeType type) {}
	virtual void historyChanged(u32 cur, u32 total) {}
	virtual void request(RequestType type, u32 val = 0) {}
	virtual void requestUpdate(u16 x, u16 y, u16 w, u16 h);

private:
	// utility functions
	void do_normal_char();
	void do_control_char();
	void scroll_region(u16 start_y, u16 end_y, s16 num);	// does clear
	void shift_text(u16 y, u16 start_x, u16 end_x, s16 num); // ditto
	void clear_area(u16 start_x, u16 start_y, u16 end_x, u16 end_y);
	void changed_line(u16 y, u16 start_x, u16 end_x);
	void move_cursor(u16 x, u16 y);
	void update();
	void draw_cursor();
	u16 get_line(u16 y);
	u16 total_history_lines() { return history_full ? history_lines : history_save_line; }

	// terminal actions
	void set_q_mode();
	void clear_param();
	void param_digit();
	void next_param();

	// non-printing characters
	void cr(), lf(), bell(), tab(), bs();

	// escape sequence actions
	void reset();
	void keypad_numeric();
	void keypad_application();
	void save_cursor();
	void restore_cursor();
	void set_tab();
	void clear_tab();
	void index_down();
	void index_up();
	void next_line();
	void cursor_left();
	void cursor_right();
	void cursor_up();
	void cursor_down();
	void cursor_up_cr();
	void cursor_down_cr();
	void cursor_position();
	void cursor_position_col();
	void cursor_position_row();
	void insert_char();
	void delete_char();
	void erase_char();
	void insert_line();
	void delete_line();
	void erase_line();
	void erase_display();
	void screen_align();
	void set_margins();
	void respond_id();
	void status_report();
	void set_mode();
	void clear_mode();
	void enable_mode(bool);
	void set_display_attr();
	void set_utf8();
	void clear_utf8();
	void active_g0();
	void active_g1();
	void current_is_g0();
	void current_is_g1();
	void set_charset();
	u32 translate_char(u32);
	void set_cursor_type();
	void linux_specific();
	void begin_set_palette();
	void set_palette();
	void reset_palette();
	void set_led();
	void fbterm_specific();

	CharAttr normal_char_attr();
	CharAttr erase_char_attr();

	//history
	void history_scroll(u16 num);

	static void init_state();
	static u16 init_history_lines();
	static u8 init_default_color(bool foreground);
	static bool init_ambiguous_wide();

	typedef enum {
		ESnormal = 0, ESesc, ESsquare, ESnonstd, ESpercent, EScharset, EShash, ESfunckey, ESkeep
	} EscapeState;

	EscapeState esc_state;

	typedef void (VTerm::*ActionFunc)();
	struct Sequence {
		u16 code;
		ActionFunc action;
		EscapeState next;
	};

	static const Sequence control_sequences[], escape_sequences[];

	#define NR_STATES ESkeep
	#define MAX_CONTROL_CODE 256
	#define MAX_ESCAPE_CODE 128

	static u8 control_map[MAX_CONTROL_CODE], escape_map[NR_STATES][MAX_ESCAPE_CODE];

	//utf8 parse
	u16 utf8_count;
	u32 cur_char;

	//treat cjk ambiguous width characters as wide
	static bool ambiguous_wide;

	//charset

	typedef enum { Lat1Map = 0, GrafMap, IbmpcMap, UserMap } CharsetMap;

	bool utf8;
	bool g0_is_current;
	bool g0_is_active, s_g0_is_active;
	CharsetMap charset;
	CharsetMap g0_charset, g1_charset;
	CharsetMap s_g0_charset, s_g1_charset;

	// terminal info
	u16 *text;
	CharAttr *attrs;
	s8 *tab_stops;
	u16 *linenumbers;
	u16 *dirty_startx, *dirty_endx;
	u16 width, height, max_width, max_height;
	u16 scroll_top, scroll_bot;
	s32 pending_scroll; // >0 means scroll up

	// terminal state
	struct ModeFlag {
		ModeFlag();

		u16 toggle_meta : 1;
		u16 inverse_screen : 1;

		u16 display_ctrl : 1;
		u16 crlf : 1;
		u16 auto_wrap : 1;
		u16 insert_mode : 1;
		u16 cursor_visible : 1;
		u16 cursor_relative : 1;
		u16 col_132 : 1;

		u16 applic_keypad : 1;
		u16 autorepeat_key : 1;
		u16 cursorkey_esco : 1;
		u16 mouse_report : 2;
		u16 cursor_shape : 3;
	} mode_flags;

	u16 cursor_x, cursor_y, s_cursor_x, s_cursor_y;
	CharAttr char_attr, s_char_attr;

	static CharAttr default_char_attr;
	u8 cur_fcolor, cur_bcolor;
	s8 cur_underline_color, cur_halfbright_color;

	// action parameters
	#define NPAR 16
	u16 npar, param[NPAR];
	bool q_mode, palette_mode;

	//history
	static u16 history_lines;
	bool history_full;
	u32 history_save_line, visual_start_line;
};

#endif
