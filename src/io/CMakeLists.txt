ADD_library(io
	io.c
	cmem.c cmem_m9425.c
	cchar.c cchar_term.c
	multix.c multix_winch.c multix_floppy.c multix_puncher.c multix_punchreader.c multix_term.c
	e4image.c
	term.c
)

# POSIX: term.c needs IEEE Std 1003.1g-2000 pselect(2)
# POSIX: term.c needs IEEE Std 1003.1b-1993 nanosleep(2) and (struct timespec)
# POSIX: term.c needs X/Open 5 fsync(2)
xopen(term.c 600 "pselect(2)" "nanosleep(2)" "fsync(2)")
strdup(e4image.c)

# vim: tabstop=4
