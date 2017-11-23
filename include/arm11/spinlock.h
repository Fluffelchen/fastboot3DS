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



static inline void spinlockLock(u32 *lock)
{
	u32 tmp;
	__asm__ __volatile__("1: ldrex %0, [%1]\n"
	                     "   teq %0, #0\n"
	                     "   wfene\n"
	                     "   strexeq %0, %2, [%1]\n"
	                     "   teqeq %0, #0\n"
	                     "   bne 1b\n"
	                     "   mcr p15, 0, %0, c7, c10, 5"
	                     : "=&r" (tmp)
	                     : "r" (lock), "r" (1)
	                     : "cc", "memory");
}

static inline void spinlockUnlock(u32 *lock)
{
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 5\n" // DMB
	                     "str %0, [%1]\n"
	                     "mcr p15, 0, %0, c7, c10, 4\n" // DSB
	                     "sev"
	                     :
	                     : "r" (0), "r" (lock)
	                     : "memory");
}
