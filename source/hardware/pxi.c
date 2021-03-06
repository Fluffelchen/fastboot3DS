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
#include "hardware/pxi.h"
#ifdef ARM9
	#include "arm9/hardware/interrupt.h"
	#include "arm9/debug.h"
#elif ARM11
	#include "arm11/hardware/interrupt.h"
	#include "arm11/debug.h"
#endif
#include "ipc_handler.h"
#include "fb_assert.h"
#include "hardware/cache.h"



static void pxiIrqHandler(UNUSED u32 id);

void PXI_init(void)
{
	REG_PXI_SYNC = PXI_IRQ_ENABLE;
	REG_PXI_CNT = PXI_ENABLE_SEND_RECV_FIFO | PXI_EMPTY_FULL_ERROR | PXI_FLUSH_SEND_FIFO;

#ifdef ARM9
	REG_PXI_DATA_SENT = 9;
	while(REG_PXI_DATA_RECEIVED != 11);

	IRQ_registerHandler(IRQ_PXI_SYNC, pxiIrqHandler);
#elif ARM11
	while(REG_PXI_DATA_RECEIVED != 9);
	REG_PXI_DATA_SENT = 11;

	IRQ_registerHandler(IRQ_PXI_SYNC, 13, 0, true, pxiIrqHandler);
#endif
}

static void pxiIrqHandler(UNUSED u32 id)
{
	const u32 cmdCode = REG_PXI_RECV;
	const u32 inBufs = IPC_CMD_IN_BUFS_MASK(cmdCode);
	const u32 outBufs = IPC_CMD_OUT_BUFS_MASK(cmdCode);
	const u32 params = IPC_CMD_PARAMS_MASK(cmdCode);
	const u32 cmdBufSize = (inBufs * 2) + (outBufs * 2) + params;
	if(cmdBufSize > IPC_MAX_PARAMS || cmdBufSize != REG_PXI_DATA_RECEIVED)
	{
		panic();
	}

	u32 buf[IPC_MAX_PARAMS];
	for(u32 i = 0; i < cmdBufSize; i++) buf[i] = REG_PXI_RECV;
	if(REG_PXI_CNT & PXI_EMPTY_FULL_ERROR) panic();

	REG_PXI_SEND = IPC_handleCmd(IPC_CMD_ID_MASK(cmdCode), inBufs, outBufs, buf);
}

u32 PXI_sendCmd(u32 cmd, const u32 *const buf, u32 words)
{
	fb_assert(words <= IPC_MAX_PARAMS);


	const u32 inBufs = IPC_CMD_IN_BUFS_MASK(cmd);
	const u32 outBufs = IPC_CMD_OUT_BUFS_MASK(cmd);
	for(u32 i = 0; i < inBufs; i++)
	{
		const IpcBuffer *const inBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(inBuf->ptr && inBuf->size) flushDCacheRange(inBuf->ptr, inBuf->size);
	}
	for(u32 i = inBufs; i < inBufs + outBufs; i++)
	{
		const IpcBuffer *const outBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(outBuf->ptr && outBuf->size) invalidateDCacheRange(outBuf->ptr, outBuf->size);
	}

	while(REG_PXI_CNT & PXI_SEND_FIFO_FULL);
	REG_PXI_SEND = cmd;
	for(u32 i = 0; i < words; i++) REG_PXI_SEND = buf[i];
	if(REG_PXI_CNT & PXI_EMPTY_FULL_ERROR) panic();

	REG_PXI_SYNC = PXI_DATA_SENT(words) | PXI_TRIGGER_SYNC_IRQ;

	while(REG_PXI_CNT & PXI_RECV_FIFO_EMPTY);
	const u32 res = REG_PXI_RECV;

#ifdef ARM11
	// The CPU may do speculative prefetches of data after the first invalidation
	// so we need to do it again. Not sure if this is a ARMv6+ thing.
	for(u32 i = inBufs; i < inBufs + outBufs; i++)
	{
		const IpcBuffer *const outBuf = (IpcBuffer*)&buf[i * sizeof(IpcBuffer) / 4];
		if(outBuf->ptr && outBuf->size) invalidateDCacheRange(outBuf->ptr, outBuf->size);
	}
#endif

	return res;
}
