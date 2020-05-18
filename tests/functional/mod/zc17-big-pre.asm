; OPTS -c configs/mod.ini

; 17-bit byte addressing works only when:
;  * cpu_mod is on (MX-16 CPU)
;  * modifications are enabled with CRON
;  * one of the following happened:
;    * carry on pre-mod, no b-mod
;    * carry on b-mod

	.cpu	mx16

	.include cpu.inc
	.include io.inc

	.const	magic1 0x4455
	.const	magic2 0xfeba
	.const	addr 0x200
	.const	page 9

	uj	start

mask:	.word	IMASK_NOMEM
zeroreg:.word	0, 0, 0, 0, 0, 0, 0

nomem_proc:
	hlt	040

	.org	OS_START

start:	la	zeroreg
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM

	lw	r1, page\MEM_PAGE | 0\MEM_SEGMENT
	ou	r1, 3\MEM_FRAME | 0\MEM_MODULE | MEM_CFG
	.word	err, err, ok, err

err:	hlt	040

ok:	im	mask
	lw	r1, magic1
	rw	r1, (page\3+addr) & 0b0111111111111111
	lw	r1, magic2
	rw	r1, page\3+addr
	lw	r7, page\3+addr

	cron			; enable cpu modification

	lw	r1, 1 + ((page\3+addr) >> 1)
	md	(page\3+addr) >> 1
	lb	r3, r7+r1	; this should work (17th bit overflown at B-mod)

	hlt	077

stack:

; XPCT r3 : 0x00ba
; XPCT ir : 0xec3f
