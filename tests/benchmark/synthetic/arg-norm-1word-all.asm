	lwt	r1, 1
loop:
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	md	1
	lw	r7, r3 + r4
	irb	r1, loop
	ujs	loop
	hlt	077
