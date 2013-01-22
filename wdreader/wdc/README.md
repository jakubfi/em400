Winchester Disk Controler is a software to control Winchester disk drive with Atmega162 microcontroller.
It doesn't read any data, its only purpose is to initialize disk drive and move read/write heads so data
can be read with another piece of circuitry (in this particular case Logic16 is used as a data sampler).

Microcontroler configuration:

* clocked by internal RC oscillator, 8MHz (no clock divider)
* JTAG disabled

Disk-uC connections (connection is for reading data only):

* all odd pins - GND
* 2 - 5V pull-up
* 4 - 5V pull-up
* 6 - 5V pull-up
* 8 - PC0
* 10 - PC1
* 12 - NC
* 14 - PA7
* 16 - NC
* 18 - PA6
* 20 - PC2
* 22 - PC3
* 24 - PA5
* 26 - PA4
* 28 - PA3
* 30 - PA2
* 32 - PA1
* 34 - PA0

uC receives commands and sends responses on UART0 (async serial, 19200baud, 8N1) preferably connected to an FTDI232 USB chip.

* PD0 (RXD0)
* PD1 (TXD0)

