ADD_library(mem
	mem.c mem_mega.c mem_elwro.c
)

# POSIX: mem.c needs X/Open 6 (pthread_spinlock_t)
xopen(mem.c 600 pthread_spinlock_t)

# vim: tabstop=4
