ADD_library(cpu
	instructions.c timer.c cpu.c interrupts.c iset.c registers.c alu.c
)

# POSIX: timer.c Needs IEEE Std 1003.1b-1993 for clock_gettime(2)
# POSIX: timer.c Needs IEEE Std 1003.1-2004 for sem_timedwait(2)
xopen(timer.c 600 "clock_gettime(2)" "sem_timedwait(2)")

# vim: tabstop=4
