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

#ifndef CONFIG_H
#define CONFIG_H

#include "type.h"
#include "instance.h"

class Config {
	DECLARE_INSTANCE(Config)
public:
	void getOption(const s8 *key, s8 *val, u32 len);
	void getOption(const s8 *key, u32 &val);
	void getOption(const s8 *key, bool &val);
	bool parseArgs(s32 argc, s8 **argv);
	s8** getShellCommand() { return mShellCommand; }

private:
	void checkConfigFile(const s8 *name);
	void parseOption(s8 *str);
	void addEntry(const s8 *key, const s8 *val);

	struct OptionEntry {
		const s8 *key;
		const s8 *val;
		OptionEntry *next;
	};

	OptionEntry *getEntry(const s8 *key);

	s8 *mConfigBuf;
	OptionEntry *mConfigEntrys;
	s8 **mShellCommand;
};

#endif
