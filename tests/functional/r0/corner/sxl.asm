	; sets X
	lwt	r0, 1
	sxl	r0
	rpc	r1

	; clears X
	lw	r0, ?X
	sxl	r0
	rpc	r2

	hlt	077

; XPCT r1 : 0x0081
; XPCT r2 : 0
