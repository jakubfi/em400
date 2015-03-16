add_library(io
	io.c chan.c
#	cmem.c cmem_m9425.c
	cchar.c cchar_term.c
)

add_subdirectory(dev)

# vim: tabstop=4
