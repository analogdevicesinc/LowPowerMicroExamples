MAX3262X Example -- Modifying the System Clock

Description:
MAX3262X series of micro controllers has different clock rate choices for the system clock. The sample source code herein sets the clock at 4MHz with divider 8. The code sets the clock frequency to 4MHz.

Operation:
This code was developed in a such a way that initially the micro runs at its standard clock rate.  Once the SW1 is pressed the micro enters 4MHz mode. A string is shown on the LCD display “NHD12832_ShowString((uint8_t*)"4 Mhz clock", 0, 4);”.

See the oscilloscope screen shots in this directory showing before and after pics of the microprocessor clock signal.
