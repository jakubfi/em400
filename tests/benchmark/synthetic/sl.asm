	lwt	r1, 1
loop:
	lw	r2, r1
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	slz	r2
	sly	r2
	slx	r2
	irb	r1, loop
	ujs	loop
	hlt	077
