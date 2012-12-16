
About em400
==========================================================================

EM400 is an attempt to create emulation of Polish MERA-400 minicomputer system,
built in late '70 and popular in the '80. Learn more at: http://mera400.pl

Currently, project is in highly development state, that means emulation is:

* incomplete
* inaccurate
* unoptimized

Basically, there is nothing interesting you can do with it at the moment. :-)


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

And have fun.

