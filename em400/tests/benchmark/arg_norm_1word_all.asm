.prog "benchmark/arg_norm_1word_all"

	lwt r1, 1
loop:
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	md 1
	lw r7, r3 + r4
	irb r1, loop
	hlt 077

.finprog
