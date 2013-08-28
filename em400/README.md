
About em400
==========================================================================

EM400 is an emulator of Polish MERA-400 minicomputer system (built in late '70 and popular in the '80). Learn more at: http://mera400.pl

Project is still in development state, but most of the emulation works fine and EM400 can run original MERA-400 software. Things that work:

* CPU
* memory
* interrupts
* floating point arithmetics
* character channel (character-oriented I/O interface)
* memory channel (block-oriented I/O interface)
* MULTIX peripherial processor
* Winchester disk drive
* TCP terminal
* built-in debugger

Things that are still waiting for implementation:

* MERA 9425 disk drive
* Terminals (serial, console)
* 5.25" floppy disk drive
* real time clock

Things that may or may not be implemented:

* PLIX peripherial processor
* EC 6051 disk drive
* PT-305 tape drive
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

