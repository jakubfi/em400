; OPTS -c configs/meclo.ini

; Check MECLO-400 PROM:
;  * reads work?
;  * addressing wraps around?
;  * contents match?

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	CHAR_CHAN 7
	.const	MECLO_DEV 0
	.const	MECLO CHAR_CHAN\IO_CHAN | MECLO_DEV\IO_DEV
	.const	PROM_SIZE 256

	.org	OS_START

; ------------------------------------------------------------------------
; Compare memory contents
; ARGS: 
; r1 - buffer 1 address
; r2 - buffer 2 address
; r3 - word count
; RETURN:
; r1=0 - match, r1 !=0 - no match
memcmp: 
	.res	1
        
	cwt	r3, 0
	jes	.done
	awt	r1, -1
	awt	r2, -1

.loop:  
	lw	r4, [r1+r3]
	cl	r4, [r2+r3]
	blc	?E
	drb	r3, .loop
.done:
	lw	r1, r3
	uj	[memcmp]

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lwt	r1, 0			; byte counter
	lw	r7, prom_read		; byte address of the read buffer
	slz	r7
.next:
	in	r3, 1\0 + MECLO
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	041
.pe1:	hlt	042
.en1:	hlt	043
.ok1:
	rb	r3, r7+r1
	awt	r1, 1
	cw	r1, PROM_SIZE*2		; two full reads to verify MECLO addressing loops around
	jl	.next

	; verify first read
	lw	r1, prom_exp
	lw	r2, prom_read
	lw	r3, PROM_SIZE/2
	lj	memcmp

	cw	r1, 0
	jes	.second_read
	hlt	050

.second_read:
	; verify second read
	lw	r1, prom_exp
	lw	r2, prom_read + PROM_SIZE/2
	lw	r3, PROM_SIZE/2
	lj	memcmp

	cw	r1, 0
	jes	.fin
	hlt	051
.fin:
	hlt	077

prom_exp:
	.word 0x4f52, 0x4040, 0x406a, 0x4544, 0x434e, 0x6c40
	.word 0x4e71, 0x404e, 0x6441, 0x4d44, 0x404e, 0x4245
	.word 0x4e6c, 0x484e, 0x4641, 0x4f76, 0x4544, 0x4644
	.word 0x4d76, 0x5b4f, 0x7140, 0x4040, 0x6944, 0x4c48
	.word 0x4040, 0x794f, 0x7d48, 0x4040, 0x7547, 0x5446
	.word 0x4040, 0x5740, 0x4053, 0x4040, 0x574e, 0x7050
	.word 0x4c69, 0x4a4e, 0x7051, 0x4048, 0x4040, 0x5440
	.word 0x4068, 0x4040, 0x7440, 0x4f7d, 0x4344, 0x4c40
	.word 0x4f7f, 0x7f4e, 0x6c41, 0x487c, 0x4840, 0x405e
	.word 0x4e42, 0x414e, 0x7052, 0x4d66, 0x414e, 0x7440
	.word 0x4040, 0x424c, 0x5440, 0x4f7f, 0x7c40, 0x4040
	.word 0x4041, 0x6140, 0x4162, 0x4040, 0x7d40, 0x4048
	.word 0x4040, 0x404d, 0x4040, 0x4a40, 0x4044, 0x4040
	.word 0x4840, 0x4040, 0x4079, 0x4041, 0x4040, 0x4140
	.word 0x4041, 0x4640, 0x5c40, 0x4040, 0x4040, 0x4440
	.word 0x4240, 0x4040, 0x5040, 0x4040, 0x4040, 0x4040
	.word 0x4048, 0x4140, 0x4040, 0x4040, 0x5b41, 0x4c43
	.word 0x4058, 0x5c40, 0x4441, 0x4048, 0x4040, 0x4040
	.word 0x4043, 0x7f40, 0x4040, 0x4040, 0x4040, 0x4040
	.word 0x4040, 0x5e40, 0x405e, 0x4040, 0x5e40, 0x405e
	.word 0x4040, 0x5740, 0x4057, 0x4040, 0x5740, 0x4057
	.word 0x5751, 0x5800
prom_read:

; XPCT ir : 0xec3f
; XPCT alarm : 0
