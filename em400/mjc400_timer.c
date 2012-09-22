#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include "mjc400_timer.h"
#include "mjc400_regs.h"

// -----------------------------------------------------------------------
void mjc400_timer_interrupt(int i)
{
	// TODO: reentrancy
	RZ_5sb;
}

// -----------------------------------------------------------------------
int mjc400_timer_start()
{
	if (signal(SIGALRM, mjc400_timer_interrupt) == SIG_ERR) {
		return 1;
	}

	struct itimerval it_val;

	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = MJC400_TIMER * 1000;
	it_val.it_interval = it_val.it_value;

	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		return 1;
	}
	return 0;
}
