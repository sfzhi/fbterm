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

#ifndef FBTERM_H
#define FBTERM_H

#include "type.h"
#include "instance.h"

class FbTerm {
	DECLARE_INSTANCE(FbTerm)
public:
	void run();
	void exit() {
		mRun = false;
	}
	void processSysKey(u32 key);
	void processSignal(u32 signo);
	void initChildProcess();

private:
	void init();

	bool mInit, mRun;
};

#endif
