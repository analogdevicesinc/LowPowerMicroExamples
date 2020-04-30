MAX3263X Example -- SPI Master 2

Description
MAX3263X series micro controllers has different SPI masters. This sample source code exercizes SPI master2 to write data out.

The code has a Frequency of 8000000Hz
P5.0 acts as SCLK
P5.1 acts as MOSI
P5.3 acts as SS

There are two types of SPI writes provided by Maxim.  In this case we are using SPIM_trans. This means that the transmit FIFO register should be allowed to complete transmitting before control is returned from the SPI function (recommended when not using interrupts). 

Operation
This program transmits two bytes of 0X0A, from P5.1 at a rate of 8MHz continuously.

