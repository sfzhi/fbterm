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

#include <string.h>
#include <locale.h>
#include <langinfo.h>
#include <iconv.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "io.h"

DEFINE_INSTANCE(IoDispatcher)

IoDispatcher::IoDispatcher()
{
}

IoDispatcher::~IoDispatcher()
{
}

const s8 *IoPipe::localCodec()
{
	static s8 local_codec[32];
	if (!local_codec[0]) {
		// The output character set of gettext is, by default, the value of nl_langinfo(CODESET),
		// which depends on the LC_CTYPE part of the current locale.
		setlocale(LC_CTYPE, "");

		s8 *locale = setlocale(LC_CTYPE, "");
		if (!locale || !strcmp(locale, "C")) {
			strcpy(local_codec, "UTF-8");
		} else {
			strncpy(local_codec, nl_langinfo(CODESET), sizeof(local_codec) - 1);
		}
	}

	return local_codec;
}

IoPipe::IoPipe()
{
	mFd = -1;
	mCodecRead = 0;
	mCodecWrite = 0;
	mBufLenRead = 0;
	mBufLenWrite = 0;
}

IoPipe::~IoPipe()
{
	if (mFd != -1) {
		IoDispatcher::instance()->removeIoSource(this, true);
		close(mFd);
	}

	if (mCodecRead) {
		iconv_close((iconv_t)mCodecRead);
	}

	if (mCodecWrite) {
		iconv_close((iconv_t)mCodecWrite);
	}
}

void IoPipe::setFd(s32 fd)
{
	if (fd == mFd) return;

	if (mFd != -1) {
		IoDispatcher::instance()->removeIoSource(this, true);
		close(mFd);
	}

	mFd = fd;

	if (mFd != -1) {
		fcntl(mFd, F_SETFD, fcntl(mFd, F_GETFD) | FD_CLOEXEC);
		fcntl(mFd, F_SETFL, fcntl(mFd, F_GETFL) | O_NONBLOCK);

		IoDispatcher::instance()->addIoSource(this, true);
	}
}


void IoPipe::setCodec(const s8 *up, const s8 *down)
{
	if (!up || !down) return;

	if (mCodecRead) {
		iconv_close((iconv_t)mCodecRead);
		mCodecRead = 0;
	}

	if (mCodecWrite) {
		iconv_close((iconv_t)mCodecWrite);
		mCodecWrite = 0;
	}

	if (!strcasecmp(up, down)) return;

	mCodecRead = iconv_open(up, down);
	if (mCodecRead == (void*)-1) mCodecRead = 0;

	mCodecWrite = iconv_open(down, up);
	if (mCodecWrite == (void*)-1) mCodecWrite = 0;
}

#define BUF_SIZE 10240

void IoPipe::ready(bool isread)
{
	if (!isread) return;

	s8 buf[BUF_SIZE];
	s32 len = read(mFd, buf + mBufLenRead, sizeof(buf) - mBufLenRead);

	if (!len) {
		ioError(true, 0); // end of file
	} else if (len == -1) {
		if (errno != EAGAIN && errno != EINTR) {
			ioError(true, errno);
		}
	} else if (len > 0) {
		if (mBufLenRead) {
			memcpy(buf, mBufRead, mBufLenRead);
			len += mBufLenRead;
			mBufLenRead = 0;
		}

		translate(true, buf, len);
	}
}

void IoPipe::write(s8 *buf, u32 len)
{
	translate(false, buf, len);
}

void IoPipe::translate(bool isread, s8 *buf, u32 len)
{
	if (!buf || !len) return;

	s8 *bufleft;
	u32 *buflen;
	iconv_t codec;

	if (isread) {
		bufleft = mBufRead;
		buflen = &mBufLenRead;
		codec = (iconv_t)mCodecRead;
	} else {
		bufleft = mBufWrite;
		buflen = &mBufLenWrite;
		codec = (iconv_t)mCodecWrite;
	}

	if (!codec) {
		if (isread) {
			readyRead(buf, len);
		} else {
			writeIo(buf, len);
		}

		return;
	}

	u32 outlen = BUF_SIZE;
	s8 outbuf[outlen];

	size_t nconv, left, total = len;
	s8 *outptr, *inptr = buf;

	while (total) {
		outptr = outbuf;
		left = outlen;

		nconv = iconv(codec, &inptr, &total, &outptr, &left);
		if (left < outlen) {
			if (isread) {
				readyRead(outbuf, outlen - left);
			} else {
				writeIo(outbuf, outlen - left);
			}
		}

		if ((ssize_t)nconv == -1) {
			if (errno == EILSEQ) {
				s8 c = '?';
				if (isread) {
					readyRead(&c, 1);
				} else {
					writeIo(&c, 1);
				}

				inptr++;
				total--;
			} else if (errno == EINVAL) {
				memcpy(bufleft, inptr, total);
				*buflen = total;
				total = 0;
			}
		}
	}
}

void IoPipe::writeIo(s8 *buf, u32 len)
{
	u32 num = 5;

	while (len) {
		s32 ret = ::write(mFd, buf, len);
		if (ret == -1) {
			if (errno == EAGAIN) {
				if (num--) {
					timespec tm = { 0, 200 * 1000000UL };
					nanosleep(&tm, 0);
				} else break;
			} else {
				if (errno != EINTR) {
					ioError(false, errno);
				}
				break;
			}
		} else if (ret > 0) {
			buf += ret;
			len -= ret;
		}
	}
}
