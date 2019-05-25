	.cpu mera400
again:
	lw	r1, 0x1fff
loop:
	tw	r2, r1
	pw	r2, r1
	drb	r1, loop
	ujs	again
