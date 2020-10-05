
	.cpu	mera400

	.include cpu.inc
	.include io.inc

	uj	start

imask:	.word	IMASK_ALL
imask_zero:
	.word	0

dummy:	lip
xpower:	hlt 077 ujs -2
xparity:hlt 076 ujs -2
xnomem:	hlt 075 ujs -2
x2cpu:	hlt 074 ujs -2
xpower2:hlt 073 ujs -2
xill:	hlt 072 ujs -2

stack:	.res	11*4, 0xdead

	.org	INTV
	.word	xpower, xparity, xnomem, x2cpu, xpower2, dummy, xill
	.res	25, dummy
	.org	EXLV
	.word	exlproc
	.org	STACKP
	.word	stack
	.org	OS_START

	.include kz.asm
	.include stdio.asm
	.include crc.asm
	#include "stdio_macros.h"

; ------------------------------------------------------------------------
exlproc:
	lj	crc16_cont
	lip

; ------------------------------------------------------------------------

	.const	CH	15
	.const	PC	CH\IO_CHAN | 0\IO_DEV
uzdat_list:
	.word	PC, -1

;
; OPIS TESTU:
;
; pole	dł.	opis
; --------------------------------------------
; N	1	długość testu w słowach
; ARGS	1	ilość danych wejściowych w słowach
; RES	1	ilość danych wyjściowych w słowach
; PROG	N	program testu
;
; dane wejściowe i wyjściowe są tuż za ciałem testu
;
; WYKONANIE TESTU:
;
; zapis operacji: 1 - uruchom załadowany test, 2 - załaduj następny test
; zapis ARGS danych wejściowych
; odczyt RES danych wyjściowych
;

; ------------------------------------------------------------------------
testlen:.res	1
argcnt:	.res	1
rescnt:	.res	1
op:	.res	1

	.const	TEST_END 0x0200

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	kz_init(CH, uzdat_list)
	im	imask

.test_load:
	readw(testlen, PC, 1)
	readw(argcnt, PC, 1)
	readw(rescnt, PC, 1)
	readw(testproc, PC, [testlen])

.loop:
	read(op, PC, 1)
	lw	r1, TEST_END
	cw	r1, [op]
	je	.test_load

	lwt	r1, 0
	cw	r1, [argcnt]
	jes	.noargs

	md	[testlen]
	readw(testproc, PC, [argcnt])
.noargs:
	lj	crc16_init

	im	imask_zero
	fi	imask_zero
	md	[testlen]
	lw	r7, testproc

	rj	r4, testproc

	im	imask

	md	[testlen]
	writew(testproc, PC, [rescnt])

	uj	.loop

testproc:
