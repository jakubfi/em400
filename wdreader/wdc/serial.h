#include <avr/io.h>

void serial_init(void);
void serial_tx_char(char c);
void serial_tx_string(char *data);
char serial_rx_char(void);
