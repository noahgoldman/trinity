## Code for Trinity robot 2012-2013

# Scons

Scons can be used on linux to compile and upload the code while avoiding the ide

 - Either make a virtualenv or make sure python2 is installed
 - install pyserial
 - install scons

To use, type "scons" to compile, and "scons upload" to upload

When getting the error "error: attempt to use poisoned 'SIG_USART0_RECV'":

 - Add "#define __AVR_LIBC_DEPRECATED_ENABLE__ 1" to
    /usr/share/arduino/hardware/arduino/cores/arduino/Arduino.h
