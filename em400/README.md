
About em400
==========================================================================

EM400 is an attempt to create emulation of Polish MERA-400 minicomputer system,
built in late '70 and popular in the '80. Learn more at: http://mera400.pl

Project is in highly development state, but some emulation parts work fine...ish:

* CPU
* memory
* interrupts
* basic I/O layer
* character channel (character-oriented I/O interface)
* memory channel (block-oriented I/O interface)
* built-in debugger

Things that are still waiting for implementation:

* MULTIX peripherial processor
* PLIX peripherial processor
* Winchester
* Terminals (TCP, serial, console)
* floating point arithmetics
* real time clock

Things that may or may not be implemented:

* MERA 9425 disk drive
* EC 6051 disk drive
* PT-305 tape drive
* 5.25" floppy disk drive
* MEGA memory


Requirements
==========================================================================

To build and run em400 you need:

* a Linux box
* cmake
* GNU make
* ncurses (used by debugger)
* readline (used by debugger)


Build instructions
==========================================================================

Do the following in the directory where this README lives:

```
	cmake .
	make
```

Running
==========================================================================

Emulator is by default built with debugger, which currently is its primary (and only) user interface.
Just run:

```
	build/em400
```

and emulate away.

