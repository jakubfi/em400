; OPTS -c configs/iotester.ini

; Test if IN command writes "0" to the register if there is no answer

	.cpu	mera400

	.include cpu.inc

	uj	start

	.org	OS_START

	.include iotester.inc

start:
	lw	r4, 0xffff		; value should be overwritten with "0"
	lw	r3, CMD_ANS + 0\10	; request "no answer" response
	in	r4, r3
	.word	no, en, ok, pe
no:	hlt	077
en:	hlt	040
ok:	hlt	041
pe:	hlt	042

; XPCT r4 : 0
; XPCT ir : 0xec3f

