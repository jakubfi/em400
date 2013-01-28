#include <avr/io.h>

void serial_init(void);
inline void serial_tx_char(char c);
void serial_tx_string(char *data);
inline char serial_rx_char(void);
unsigned char serial_rx_string(char *buf, unsigned char buf_size, char terminator);

