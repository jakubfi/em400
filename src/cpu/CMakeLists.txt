ADD_library(cpu
	instructions.c timer.c cpu.c interrupts.c iset.c registers.c alu.c
)

# POSIX: timer.c Needs IEEE Std 1003.1b-1993 for clock_gettime(2)
# POSIX: timer.c Needs IEEE Std 1003.1-2004 for sem_timedwait(2)
set_property(
	SOURCE timer.c
	APPEND
	PROPERTY COMPILE_DEFINITIONS
	_XOPEN_SOURCE=600
)

# vim: tabstop=4
