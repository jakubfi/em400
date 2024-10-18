
About em400
==========================================================================

EM400 is an emulator of Polish MERA-400 minicomputer system (built in late '70 and popular in the '80). Learn more at: http://mera400.pl
In its current state, EM400 can run original MERA-400 software.

Emulation features:

* MERA-400 CPU
* MX-16 CPU (original CPU with modifications)
* real CPU speed
* floating point arithmetic
* Elwro memory (32kword modules, ferromagnetic)
* Amepol MEGA memory (64kword modules, semiconductor)
* character channel (character-oriented I/O interface)
* Amepol MULTIX peripherial processor
* Winchester disk drive
* terminal connection over TCP or serial line
* built-in debugger

Things that are still waiting for implementation:

* 8" floppy disk drive (partially implemented)
* memory channel and MERA 9425 disk drive
* Terminals connected to MULTIX peripherial processor
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
* libemdas (deassembler library)
* libemcrk (CROOK memory structures)
* libemawp (floating point arithmetic)
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

EM400 is built with a debugger which currently is its primary user interface.

Usage:

```
em400 [option] ...
```

Where *options* are:

* **-h** - Display help
* **-c config** - Config file to use instead of the default one (*~/.em400/em400.ini*)
* **-p program** - Load program image into OS memory at address 0
* **-l component,component,...** - Enable logging for specified components. Available components: reg, mem, cpu, op, int, io, mx, px, cchar, cmem, term, wnch, flop, pnch, pnrd, crk5, em4h, all.
* **-L** -  Disable logging
* **-u ui** - User interface to use. Available UIs: curses (default), cmd (minimal, for remote control)
* **-F** - Use FPGA implementation of the CPU and external memory (experimental)
* **-O sec:key=value**  : Override configuration entry "key" in section [sec] with a specific value

