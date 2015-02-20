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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/stat.h>
#include "config.h"
#include "fbconfig.h"

#define MAX_CONFIG_FILE_SIZE 10240

DEFINE_INSTANCE_DEFAULT(Config)

Config::Config()
{
	mShellCommand = 0;
	mConfigBuf = 0;
	mConfigEntrys = 0;

	const s8 *home = getenv("HOME");
	if (!home) {
		if (getuid()) return;
		home = "/root";
	}

	s8 name[64];
	snprintf(name, sizeof(name), "%s/%s", home, ".fbtermrc");

	checkConfigFile(name);

	struct stat cstat;
	if (stat(name, &cstat) == -1) return;
	if (cstat.st_size > MAX_CONFIG_FILE_SIZE) return;

	s32 fd = open(name, O_RDONLY);
	if (fd == -1) return;

	mConfigBuf = new char[cstat.st_size + 1];
	mConfigBuf[cstat.st_size] = 0;

	s32 ret = read(fd, mConfigBuf, cstat.st_size);
	close(fd);

	s8 *end, *start = mConfigBuf;
	do {
		end = strchr(start, '\n');
		if (end) *end = 0;
		parseOption(start);
		if (end) start = end + 1;
	} while (end && *start);
}

Config::~Config()
{
	if (mConfigBuf) delete[] mConfigBuf;

	OptionEntry *next, *cur = mConfigEntrys;
	while (cur) {
		next = cur->next;
		delete cur;
		cur = next;
	}
}

void Config::getOption(const s8 *key, s8 *val, u32 len)
{
	if (!val) return;
	*val = 0;

	OptionEntry *entry = getEntry(key);
	if (!entry || !entry->val) return;

	u32 val_len = strlen(entry->val);
	if (--len > val_len) len = val_len;
	memcpy(val, entry->val, len);
	val[len] = 0;
}

void Config::getOption(const s8 *key, u32 &val)
{
	OptionEntry *entry = getEntry(key);
	if (!entry || !entry->val) return;

	s8 *tail;
	s32 a = strtol(entry->val, &tail, 10);

	if (!*tail) val = a;
}

void Config::getOption(const s8 *key, bool &val)
{
	OptionEntry *entry = getEntry(key);
	if (!entry || !entry->val) return;

	if (!strcmp(entry->val, "yes")) val = true;
	else if (!strcmp(entry->val, "no")) val = false;
}

Config::OptionEntry *Config::getEntry(const s8 *key)
{
	if (!key) return 0;

	OptionEntry *entry = mConfigEntrys;
	while (entry) {
		if (!strcmp(key, entry->key)) break;
		entry = entry->next;
	}

	return entry;
}

void Config::addEntry(const s8 *key, const s8 *val)
{
	OptionEntry *entry = new OptionEntry;
	entry->key = key;
	entry->val = val;
	entry->next = mConfigEntrys;
	mConfigEntrys = entry;
}

void Config::parseOption(s8 *str)
{
	s8 *cur = str, *end = str + strlen(str) - 1;
	while (*cur == ' ') cur++;

	if (!*cur || *cur == '#') return;

	s8 *key = cur;
	while (*cur && *cur != '=') cur++;
	if (!*cur) return;
	*cur = 0;

	s8 *val = ++cur;
	cur = end;
	while (*cur == ' ') cur--;
	if (cur < val) return;
	*(cur + 1) = 0;

	addEntry(key, val);
}

void Config::checkConfigFile(const s8 *name)
{
	static const s8 defaultConfig[] =
		"# Configuration for FbTerm\n"
		"\n"
		"# Lines starting with '#' are ignored.\n"
		"# Note that end-of-line comments are NOT supported, comments must be on a line of their own.\n"
		"\n\n"
		"# font family names/pixelsize used by fbterm, multiple font family names must be seperated by ','\n"
		"# and using a fixed width font as the first is strongly recommended\n"
		"font-names=mono\n"
		"font-size=12\n"
		"\n"
		"# force font width (and/or height), usually for non-fixed width fonts\n"
		"# legal value format: n (fw_new = n), +n (fw_new = fw_old + n), -n (fw_new = fw_old - n)\n"
		"#font-width=\n"
		"#font-height=\n"
		"\n"
		"# default color of foreground/background text\n"
		"# available colors: 0 = black, 1 = red, 2 = green, 3 = brown, 4 = blue, 5 = magenta, 6 = cyan, 7 = white\n"
		"color-foreground=7\n"
		"color-background=0\n"
		"\n"
		"# max scroll-back history lines of every window, value must be [0 - 65535], 0 means disable it\n"
		"history-lines=1000\n"
		"\n"
		"# up to 5 additional text encodings, multiple encodings must be seperated by ','\n"
		"# run 'iconv --list' to get available encodings.\n"
		"text-encodings=\n"
		"\n"
		"# cursor shape: 0 = underline, 1 = block\n"
		"# cursor flash interval in milliseconds, 0 means disable flashing\n"
		"cursor-shape=0\n"
		"cursor-interval=500\n"
		"\n"
		"# additional ascii chars considered as part of a word while auto-selecting text, except ' ', 0-9, a-z, A-Z\n"
		"word-chars=._-\n"
		"\n"
		"# change the clockwise orientation angle of screen display\n"
		"# available values: 0 = 0 degree, 1 = 90 degrees, 2 = 180 degrees, 3 = 270 degrees\n"
		"screen-rotate=0\n"
		"\n"
		"# specify the favorite input method program to run\n"
		"input-method=\n"
		"\n"
		"# treat ambiguous width characters as wide\n"
		"#ambiguous-wide=yes\n"
		;

	struct stat cstat;
	if (stat(name, &cstat) != -1) return;

	s32 fd = open(name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) return;

	s32 ret = write(fd, defaultConfig, sizeof(defaultConfig) - 1);
	close(fd);
}

bool Config::parseArgs(s32 argc, s8 **argv)
{
	static const option options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "version", no_argument, 0, 'V' },
		{ "verbose", no_argument, 0, 'v' },
		{ "font-names", required_argument, 0, 'n' },
		{ "font-size", required_argument, 0, 's' },
		{ "color-foreground", required_argument, 0, 'f' },
		{ "color-background", required_argument, 0, 'b' },
		{ "text-encodings", required_argument, 0, 'e' },
		{ "screen-rotate", required_argument, 0, 'r' },
		{ "input-method", required_argument, 0, 'i' },
		{ "cursor-shape", required_argument, 0, 0 },
		{ "cursor-interval", required_argument, 0, 1 },
		{ "font-width", required_argument, 0, 2 },
		{ "font-height", required_argument, 0, 4 },
		{ "ambiguous-wide", no_argument, 0, 'a' },
#ifdef ENABLE_VESA
		{ "vesa-mode", required_argument, 0, 3 },
#endif
		{ 0, 0, 0, 0 }
	};

	s32 index;
	while ((index = getopt_long(argc, argv, "Vvhn:s:f:b:e:r:i:a", options, 0)) != -1) {
		switch (index) {
		case 'V':
			printf("FbTerm version " VERSION "\n");
			return false;

		case 'h':
		case '?':
			printf(
				"Usage: fbterm [options] [--] [command [arguments]]\n"
				"A fast framebuffer/vesa based terminal emulator for linux\n"
				"\n"
				"  -h, --help                      display this help and exit\n"
				"  -V, --version                   display FbTerm version and exit\n"
				"  -v, --verbose                   display extra information\n"
				"  -n, --font-names=TEXT           specify font family names\n"
				"  -s, --font-size=NUM             specify font pixel size\n"
				"      --font-width=NUM            force font width\n"
				"      --font-height=NUM           force font height\n"
				"  -f, --color-foreground=NUM      specify foreground color\n"
				"  -b, --color-background=NUM      specify background color\n"
				"  -e, --text-encodings=TEXT       specify additional text encodings\n"
				"  -r, --screen-rotate=NUM         specify orientation of screen display\n"
				"  -a, --ambiguous-wide            treat ambiguous width characters as wide\n"
				"  -i, --input-method=TEXT         specify input method program\n"
				"      --cursor-shape=NUM          specify default cursor shape\n"
				"      --cursor-interval=NUM       specify cursor flash interval\n"
#ifdef ENABLE_VESA
				"      --vesa-mode=NUM             force VESA video mode\n"
				"                  list            display available VESA video modes\n"
#endif
				"\n"
				"See comments in ~/.fbtermrc for details of these options.\n"
			);
			return false;

		default:
			for (const option *opt = options; opt->name; opt++) {
				if (opt->val != index) continue;

				const s8 *val  = (opt->has_arg ? optarg : "yes");
				OptionEntry *entry = getEntry(opt->name);

				if (entry) entry->val = val;
				else addEntry(opt->name, val);

				break;
			}
			break;
		}
	}

	if (argv[optind]) mShellCommand = argv + optind;
	return true;
}
