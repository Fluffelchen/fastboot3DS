#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types.h"
#include "fatfs/ff.h"


#define FS_MAX_DRIVES   (FF_VOLUMES)
#define FS_DRIVE_NAMES  FF_VOLUME_STRS

#define FS_MAX_FILES    (5)

#define FS_MAX_DIRS     (3)


typedef enum
{
	FS_DRIVE_SDMC = 0,
	FS_DRIVE_TWLN = 1,
	FS_DRIVE_TWLP = 2,
	FS_DRIVE_NAND = 3
} FsDrive;

typedef enum
{
	FS_OPEN_READ          = FA_READ,
	FS_OPEN_WRITE         = FA_WRITE,
	FS_OPEN_EXISTING      = FA_OPEN_EXISTING,
	FS_CREATE_NEW         = FA_CREATE_NEW,
	FS_CREATE_ALWAYS      = FA_CREATE_ALWAYS,
	FS_OPEN_ALWAYS        = FA_OPEN_ALWAYS,
	FS_OPEN_APPEND        = FA_OPEN_APPEND
} FsOpenMode;

typedef FILINFO FsFileInfo;



s32 fMount(FsDrive drive);
s32 fUnmount(FsDrive drive);
s32 fGetFree(FsDrive drive, u64 *size);
s32 fOpen(const char *const path, FsOpenMode mode);
s32 fRead(s32 handle, void *const buf, u32 size);
s32 fWrite(s32 handle, const void *const buf, u32 size);
s32 fSync(s32 handle);
s32 fLseek(s32 handle, u32 offset);
u32 fTell(s32 handle);
u32 fSize(s32 handle);
s32 fClose(s32 handle);
s32 fExpand(s32 handle, u32 size);
s32 fStat(const char *const path, FsFileInfo *fi);
s32 fOpenDir(const char *const path);
s32 fReadDir(s32 handle, FsFileInfo *fi, u32 num);
s32 fCloseDir(s32 handle);
s32 fMkdir(const char *const path);
s32 fRename(const char *const old, const char *const new);
s32 fUnlink(const char *const path);
