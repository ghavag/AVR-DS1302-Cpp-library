# DS1302 timekeeper IC C++ library for AVR/Microchip ATmega microcontrollers

The original code was found at https://playground.arduino.cc/Main/DS1302/ and
written by a arduino.cc user called Krodal. I refactored the code to be a C++
class for AVR/Microchip ATmega microcontrollers and decoupled it from the
Arduino library to make it usable in non-arduino projects.

I tested the library on an Arduino Nano (ATmega328p). However I am using only
the Arduino hardware and the bootloader to burn the example code (see below)
into the controller. The actual code does not depend on the Arduino library.


## Features

Each part of the date/time value is stored in it's own register. There is a
register for hours, minutes, day of month and so on. You can either access those
registers individually by using the `read()` and `write()` methods or access all
date/time registers at once using the methods `clock_burst_read()` and
`clock_burst_read()`. For usage see `example.cpp`. Also note the comments in
`ds1302.h`.


### Not supported features

The DS1302 IC has some other features, which are not supported by the library
yet.

In addition to the date/time registers the DS1302 has a 31 bytes RAM that can be
used to store arbitrary data. Comments in the original source code are stating
that using the internal EEPROM of the ATmega are better anyway.

The DS1302 also has a trickle charger. Comments in the original source code
are stating:
```
// Trickle charge
// --------------
// The DS1302 has a build-in trickle charger.
// That can be used for example with a lithium battery
// or a supercap.
// Using the trickle charger has not been implemented
// in this code.
```


## Use the example code

Enter the directory `example`. If you are using a different controller than
ATmega328p edit `Makefile` to try out the example code. Run `make` to compile
and `make flash` to burn the example program to the controller using the Ardunio
bootloader. Issue `make clean` to cleanup output files produced by the compiler.

You can use `picocom` (e. g. run `picocom -b 9600 /dev/ttyUSB0`) to see the
output produced by the controller.

The output looks like follows:
```
Read date using read() method: YYYY/MM/DD = 2019/9/11

Demonstration of the write() method
	Read the following time: 13:29:47
	Set the time to 12:34:56...
	...and read it again: 12:34:56

=== Starting date and time reading loop ===
Time = 13:29:47, Date(day of month) = 11, Month = 9, Day(day of week) = 1, Year = 2019
Time = 13:29:48, Date(day of month) = 11, Month = 9, Day(day of week) = 1, Year = 2019
Time = 13:29:50, Date(day of month) = 11, Month = 9, Day(day of week) = 1, Year = 2019
Time = 13:29:51, Date(day of month) = 11, Month = 9, Day(day of week) = 1, Year = 2019
Time = 13:29:52, Date(day of month) = 11, Month = 9, Day(day of week) = 1, Year = 2019
...
```
