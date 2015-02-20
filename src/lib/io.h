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

#ifndef IO_H
#define IO_H

#include "type.h"
#include "instance.h"

class IoPipe {
public:
	IoPipe();
	virtual ~IoPipe();

	s32 fd() { return mFd; }
	void ready(bool isread);

	static const s8 *localCodec();

protected:
	void setFd(s32 fd);
	void setCodec(const s8 *up, const s8 *down);
	void write(s8 *buf, u32 len);

	virtual void readyRead(s8 *buf, u32 len) = 0;
	virtual void ioError(bool read, s32 err) {}

private:
	void translate(bool isread, s8 *buf, u32 len);
	void writeIo(s8 *buf, u32 len);

	s32 mFd;
	void *mCodecRead, *mCodecWrite;
	s8 mBufRead[16], mBufWrite[16];
	u32 mBufLenRead, mBufLenWrite;
};

class IoDispatcher {
	DECLARE_INSTANCE(IoDispatcher)
private:
	virtual void addIoSource(IoPipe *src, bool isread) = 0;
	virtual void removeIoSource(IoPipe *src, bool isread) = 0;

	friend class IoPipe;
};

#endif
