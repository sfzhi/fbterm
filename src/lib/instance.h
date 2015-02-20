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

#ifndef INSTANCE_H
#define INSTANCE_H

#define DECLARE_INSTANCE(classname) \
public: \
	static classname *instance(); \
	static void uninstance(); \
protected: \
	classname(); \
	virtual ~classname(); \
private: \
	static classname *createInstance(); \
	static classname *mp##classname;

#define DEFINE_INSTANCE(classname) \
classname *classname::mp##classname = 0; \
 \
classname *classname::instance() \
{ \
	if (!mp##classname) { \
		mp##classname = createInstance(); \
	} \
	 \
	return mp##classname; \
} \
 \
void classname::uninstance() \
{ \
	if (mp##classname) { \
		delete mp##classname; \
		mp##classname = 0; \
	} \
}

#define DEFINE_INSTANCE_DEFAULT(classname) \
DEFINE_INSTANCE(classname) \
 \
classname *classname::createInstance() \
{ \
	return new classname(); \
}

#endif
