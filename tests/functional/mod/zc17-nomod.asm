; OPTS -c configs/minimal.cfg

; 17-bit byte addressing does not work on unmodified cpu

	.cpu	mx16

	.include cpu.inc
	.include io.inc

	.const	magic1 0x4455
	.const	magic2 0xfeba
	.const	addr 0x200
	.const	page 9

	uj	start

mask:	.word	IMASK_NOMEM
zeroreg:.res	7, 0

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

; no matter how we build the address, 'lb' will fail for addresses >= 0x8000

	md	1
	lb	r3, r7+r7

	lw	r4, r7
	md	1
	lb	r5, r7+r4

	lwt	r4, 0
	md	1+r7
	lb	r4, r7

	lwt	r1, 0
	lb	r1, ((page\3+addr)*2 + 1) & 0xFFFF

	hlt	077

stack:

; XPCT r1 : 0x0055
; XPCT r3 : 0x0055
; XPCT r4 : 0x0055
; XPCT r5 : 0x0055
; XPCT ir : 0xec3f
