; CONFIG configs/iotester.ini

; IN leaves the register unchanged if there is no answer

	.cpu	mera400

	.include cpu.inc

	uj	start

	.org	OS_START

	.include iotester.inc

start:
	lw	r4, 0xffff		; must survive the failed IN
	lw	r3, CMD_ANS + 0\10	; request "no answer" response
	in	r4, r3
	.word	no, en, ok, pe
no:	hlt	077
en:	hlt	040
ok:	hlt	041
pe:	hlt	042

; XPCT r4 : 0xffff
