; mem and memmap: read back, unmapped page gaps, address wrap.

	.include cpu.inc
	.include io.inc

; PRECMD memw 0 100 1 2 0xffff

	uj	start
err:	hlt	040

	.org	OS_START
start:
	lw	r1, 1\MEM_SEGMENT | 2\MEM_PAGE
	ou	r1, 0\MEM_MODULE | 2\MEM_FRAME | MEM_CFG
	.word	err, err, ok, err
ok:
	hlt	077

; XPCT_CMD memmap 0 : "OK: 0x0003"
; XPCT_CMD memmap 1 : "OK: 0x0004"
; XPCT_CMD memmap 2 : "OK: 0x0000"

; XPCT_CMD mem 0 100 3 : "OK: 0x0001 0x0002 0xffff"
; XPCT_CMD memw 0 0 0x1234 : "OK: 1 words written"
; XPCT_CMD memw 1 0x2000 0x5a5a : "OK: 1 words written"
; XPCT_CMD mem 0 0xffff 2 : "OK: ? 0x1234"
; XPCT_CMD mem 1 0x1fff 2 : "OK: ? 0x5a5a"
; XPCT_CMD mem 1 0x0ffe 0x1003 : "OK: ? ? 0x5a5a"
; XPCT_CMD mem 2 0 1 : "OK: ?"
