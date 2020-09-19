	lwt	r1, 1
loop:
	lw	r2, r1
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	srz	r2
	sry	r2
	srx	r2
	irb	r1, loop
	ujs	loop
	hlt	077
