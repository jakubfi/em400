
About em400
==========================================================================

EM400 is an emulator of Polish MERA-400 minicomputer system (built in late '70 and popular in the '80). Learn more at: http://mera400.pl

Project is still in development state, but most of the emulation works fine and EM400 can run original MERA-400 software.

Emulation features:

* MERA-400 CPU
* MX-16 CPU (original CPU with modifications)
* floating point arithmetics
* Elwro memory (32kword modules, ferromagnetic)
* Amepol MEGA memory (64kword modules, semiconductor)
* character channel (character-oriented I/O interface)
* memory channel (block-oriented I/O interface)
* Amepol MULTIX peripherial processor
* Winchester disk drive
* TCP terminal
* built-in debugger

Things that are still waiting for implementation:

* MERA 9425 disk drive
* Terminals (serial, console)
* 5.25" floppy disk drive
* Amepol real time clock

Things that may or may not be implemented:

* Amepol PLIX peripherial processor
* EC 6051 disk drive
* PT-305 tape drive


Requirements
==========================================================================

To build and run em400 you'll need:

* cmake
* GNU make
* bison and flex
* libemdas
* libemcrk
* libemawp
* ncurses (used by debugger)
* readline (used by debugger)


Build instructions
==========================================================================

Do the following in the source directory:

```
mkdir build
cd build
cmake ..
make
make install
```

Running
==========================================================================

Emulator is by default built with debugger, which currently is its primary (and only) user interface.

Usage:

```
em400 [option] ...
```

Where *options* are:

* **-h** - display help
* **-c config** - use given config file instead of default one (*~/.em400/em400.cfg*)
* **-p program** - load program image into OS memory
* **-l levels** - enable logging with given levels. Syntax for describing levels is: `component=level[,component=level[,..]]`. Available components are: reg, mem, cpu, op, int, io, mx, px, cchar, cmem, term, wnch, flop, pnch, pnrd, crk5, em4h, all. Logging level is 0-9.
* **-L** -  disable logging
* **-k value** - set keys to given value

Debuger-only options:

* **-s** - use simple debugger interface

