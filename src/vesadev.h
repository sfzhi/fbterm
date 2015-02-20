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

#ifndef VESADEV_H
#define VESADEV_H

#include "screen.h"

class VesaDev : public Screen {
private:
	friend class Screen;
	static void printModes();
	static VesaDev *initVesaDev(s16 mode);

	VesaDev();
	~VesaDev();

	virtual void setupOffset();
	virtual void setupPalette(bool restore);
	virtual void switchVc(bool enter);
	virtual const s8 *drvId();

	void initScrollType();
};
#endif
