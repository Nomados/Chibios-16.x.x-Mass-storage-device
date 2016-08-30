*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M4 STM32F407.                            **
*****************************************************************************

** TARGET **

The demo runs on an Waveshare Open 207Z-407Z board.

** The Demo **

The demo currently just flashes a LED using a thread and uses
 USB-FS port is used as USB-MSD (mass storage device).
You need to insert SD card to make to work
** Build Procedure **

The demo has been tested by using the free Codesourcery GCC-based toolchain
and YAGARTO.
Just modify the TRGT line in the makefile in order to use different GCC ports.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
