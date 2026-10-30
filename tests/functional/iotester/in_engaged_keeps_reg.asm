; CONFIG configs/iotester.ini

; IN leaves the register unchanged if channel/device is engaged

	.cpu	mera400

	.include cpu.inc

	uj	start

	.org	OS_START

	.include iotester.inc

start:
	lw	r4, 0xffff		; must survive the failed IN
	lw	r3, CMD_ANS + 1\10	; request "engaged" response
	in	r4, r3
	.word	no, en, ok, pe
no:	hlt	040
en:	hlt	077
ok:	hlt	041
pe:	hlt	042

; XPCT r4 : 0xffff
