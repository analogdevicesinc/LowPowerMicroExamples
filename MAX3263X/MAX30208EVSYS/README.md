# Max3263x Example -- I2C Master 1

**Description:**

This firmware example uses the [MAX32630EVKIT's](https://www.maximintegrated.com/en/products/microcontrollers/MAX32630-EVKIT.html) I2C_Master1 to configure and read the temperature from the [MAX30208](https://www.maximintegrated.com/en/products/sensors/MAX30208.html) temperature sensor on the [MAX30208EVSYS board](https://www.maximintegrated.com/en/products/sensors/MAX30208EVSYS.html).

The code requires the frequency to be 400KHZ;
SDA = P3_4;
SCL = P3_5;

**Operation:**

MAX32630EVKIT's I2C_Master1 sends a clock signal and data to WRITE or READ from MAX30208 registers.

