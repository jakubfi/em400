
About em400
==========================================================================

EM400 is an emulator of Polish MERA-400 minicomputer system (built in late '70 and popular in the '80). Learn more at: http://mera400.pl
In its current state, EM400 can run original MERA-400 software.

Emulation features:

* MERA-400 CPU
* MX-16 CPU (original CPU with modifications)
* real CPU speed
* floating point arithmetic
* Elwro memory (32kword modules, magnetic-core)
* character channel (character-oriented I/O interface)
* SP45DE 8" floppy disk drive
* Amepol MEGA memory (64kword modules, DRAM)
* Amepol MULTIX peripherial processor
* Amepol real time clock
* Winchester disk drive
* terminal connection over TCP or serial line

Things that are still waiting for implementation:

* memory channel and MERA 9425 disk drive
* Terminals connected to MULTIX peripherial processor
* 5.25" floppy disk drive

Things that may or may not be implemented:

* Amepol PLIX peripherial processor
* EC 6051 disk drive
* PT-305 tape drive


Requirements
==========================================================================

To build and run em400 you'll need:

* cmake
* a build tool (GNU make or ninja)
* bison and flex
* readline (used by cmd ui)
* Qt6 (used by qt ui)
* libemdas (deassembler library)
* libemcrk (CROOK memory structures)
* libemawp (floating point arithmetic)


Build instructions
==========================================================================

Do the following in the source directory:

```
mkdir build
cmake -B build
cmake --build build
cmake --install build
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
* **-u ui** - User interface to use. Available UIs: qt (default), cmd (minimal, for remote control)
* **-O sec:key=value**  : Override configuration entry "key" in section [sec] with a specific value

