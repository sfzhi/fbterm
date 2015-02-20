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

#ifndef INPUT_H
#define INPUT_H

#include "io.h"
#include "instance.h"

class TtyInput : public IoPipe {
	DECLARE_INSTANCE(TtyInput)
public:
	void switchVc(bool enter);
	void setRawMode(bool raw, bool force = false);
	void showInfo(bool verbose);

private:
	virtual void readyRead(s8 *buf, u32 len);
	void setupSysKey(bool restore);
	void processRawKeys(s8* buf, u32 len);

	bool mRawMode;
};

#endif
