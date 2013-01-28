Winchester Disk Controler is a software to control Winchester disk drive with Atmega162 microcontroller.
It doesn't read any data, its only purpose is to initialize disk drive and move read/write heads so data
can be read with another piece of circuitry (in this particular case Logic16 is used as a data sampler).

Microcontroler configuration:

* clocked by external crystal oscillator, 16MHz, no clock divider
* JTAG disabled

Where to connect ST-506 control port signals:

* all odd pins - GND
* 2 (HD SLCT 3) - 5V pull-up
* 4 (HD SLCT 2) - 5V pull-up
* 6 (WRITE GATE) - 5V pull-up
* 8 (SEEK COMPLETE) - uC PC0
* 10 (TRACK 0) - uC PC1
* 12 (WRITE FAULT) - NC
* 14 (HEAD SELECT 0) - uC PA7
* 16 (RESERVED) - NC
* 18 (HEAD SELECT 1) - uC PA6
* 20 (INDEX) - uC PC2 (also to a data sampler)
* 22 (READY) - uC PC3
* 24 (STEP) - uC PA5
* 26 (DRV SLCT 1) - uC PA4
* 28 (DRV SLCT 2) - uC PA3
* 30 (DRV SLCT 3) - uC PA2
* 32 (DRV SLCT 4) - uC PA1
* 34 (DIRECTION IN) - uC PA0

uC also receives commands from the user (or software) and sends responses on UART0
(async serial, 9600baud, 8N1) preferably connected to an FTDI232 USB chip:

* FTDI TX - uC PD0 (RXD0)
* FTDI RX - uC PD1 (TXD0)

