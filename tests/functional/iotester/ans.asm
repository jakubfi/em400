; OPTS -c configs/iotester.ini

; Test if I/O channel properly sends all possible answers (NO, EN, OK, PE)

	.cpu	mera400

	.include cpu.inc

	uj	start

	.org	OS_START

	.include iotester.inc

; ------------------------------------------------
; r1 - currently tested response number
; r2 - received response number
start:
	lwt	r1, 0		; start tests with response 0: 'NO'

next:
	lwt	r2, 0		; clear received answer register
	lw	r3, r1		; load response number
	shc	r3, 11		; shift into place
	or	r3, CMD_ANS	; insert command
	in	r4, r3		; request specific response (r4 is unused here)
	.word	a0, a1, a2, a3
a3:	awt	r2, 1		; set received answer register according to response
a2:	awt	r2, 1
a1:	awt	r2, 1
a0:
	cw	r1, r2		; response as requested?
	jn	err
	cw	r1, 3		; is it the last test?
	jes	done
	awt	r1, 1		; next answer test
	ujs	next

err:	hlt	040
done:	hlt	077

; XPCT ir : 0xec3f

