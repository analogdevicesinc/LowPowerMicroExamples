MAX3263X Example -- SPI Master 1

Description:
MAX3263X series of Maxim low-power microcontrollers have multiple SPI masters. This sample source code exercizes SPI master1 to write data out.

The code has a Frequency of 8000000Hz
P1.0 acts as SCLK
P1.1 acts as MOSI
P1.3 acts as SS

There are two types of SPI writes provided by Maxim.  In this case we are using SPIM_trans. This means that the transmit FIFO register should be allowed to complete transmitting before control is returned from the SPI function (recommended when not using interrupts). 

Operation:
This program transmits two bytes of 0X0A, from P1.1 at a rate of 8MHz continuously.
