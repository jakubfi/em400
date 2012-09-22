// MJC400_TIMER is a value (in miliseconds) from the
// following sets (different sources give different sets):
// - 2, 4, 8, 10, 20
// - 2, 10, 20, 40, 80
#define MJC400_TIMER	10 // miliseconds

void mjc400_timer_interrupt(int i);
int mjc400_timer_start();

