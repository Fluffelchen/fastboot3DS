#include "mem_map.h"

.arm
.cpu arm946e-s
.fpu softvfp

.global undefInstrHandler
.global prefetchAbortHandler
.global dataAbortHandler
.global irqHandler

.type undefInstrHandler STT_FUNC
.type prefetchAbortHandler STT_FUNC
.type dataAbortHandler STT_FUNC
.type exceptionHandler STT_FUNC
.type irqHandler STT_FUNC

.extern deinitCpu
.extern guruMeditation
.extern irqHandlerTable

.section ".text"



undefInstrHandler:
	msr cpsr_f, #(0<<29)        @ Abuse conditional flags in cpsr for temporary exception type storage
	b exceptionHandler
prefetchAbortHandler:
	msr cpsr_f, #(1<<29)
	b exceptionHandler
dataAbortHandler:
	msr cpsr_f, #(2<<29)
exceptionHandler:
	mov sp, #A9_EXC_STACK_END
	stmfd sp!, {r0-r14}^        @ Save all user/system mode regs except pc
	mrs r5, cpsr
	lsr r5, r5, #29             @ Get back the exception type from cpsr
	mrs r1, spsr                @ Get saved cpsr
	stmfd sp!, {r1, lr}         @ Save spsr and lr (pc) on exception stack
	mov r6, sp
	msr cpsr_c, #0xDF           @ Disable all interrupts, system mode
	bl deinitCpu
	mov r0, r5
	mov sp, r6
	mov r1, r6
	b guruMeditation            @ r0 = exception type, r1 = reg dump ptr {cpsr, pc (unmodified), r0-r14}
.pool


irqHandler:
	stmfd sp!, {r0-r3, r12, lr}
	ldr r12, =0x10001000        //REG_IRQ_IE
	ldmia r12, {r1, r2}
	and r1, r1, r2
	mov r3, #0x80000000
irqHandler_find_first_lp:
	clz r0, r1
	bics r1, r1, r3, lsr r0
	bne irqHandler_find_first_lp
	mov r1, r3, lsr r0
	str r1, [r12, #4]           @ REG_IRQ_IF
	rsb r2, r0, #31             @ r2 = 31 - r0
	ldr r1, =irqHandlerTable
	ldr r0, [r1, r2, lsl #2]
	cmp r0, #0
	blxne r0
	ldmfd sp!, {r0-r3, r12, lr}
	subs pc, lr, #4
.pool
