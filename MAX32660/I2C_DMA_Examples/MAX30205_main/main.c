/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
 ******************************************************************************/

/**
 * @file    	main.c
 * @version		1.0
 * Started:		24JUL2020
 *
 * @brief   	I2C/DMA Example using the MAX30205 Temperature Sensor Peripheral.
 * @details 	This example uses the I2C Master to read/write from/to the I2C Slave. For
 * 		this example you must connect P0.9 to SDA and P0.8 to SCL. You must also
 * 		connect the MAX30205 to the VDDIO and GND pins on the MAX32660.
 *
 * @notes	WIRING DIAGRAMS
 *          Below are the pinouts of the associated EVKits used in developing this program,
 *          and how they should be connected. Note the ned for pullup resistors on SDA and SCL.
 *
 *                      ###############                                 ###############
 *                      #  MAX32660   #                                 #   MAX30205  #
 *                      #    EVKIT    #                                 #    EVKIT    #
 *                      #             #                                 #             #
 *                      #             #                                 #             #
 *                      #             #                                 #  # # # x #  #
 *                      #        P0_8 # --> SCL*                        #  V S S   G  #
 *                      #        P0_9 # --> SDA*                        #  D D C   N  #
 *                      #             #                                 #  D A L   D  #
 *              GND <-- # GND  VDD_IO # --> VDD                         #             #
 *                      ###############                                 ###############
 *
 *              * - attach a pullup resistor to VDD, approximately 2.2k - 10k Ohms
 *              x - unused (MAX30205 EVKit has lateral pin orientation).
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "i2c.h"
#include "mxc_delay.h"
#include "dma.h"
#include "tmr_utils.h"

/***** Definitions *****/
#define I2C_MASTER	    MXC_I2C0
#define I2C_SLAVE_ADDR	(0x90)
#define I2C_TIMEOUT     MXC_DELAY_MSEC(1)
#define MAX30205_T_HYST_REG 0x02
#define MAX30205_CONFIG_REG 0x01
#define MAX30205_TEMP_REG 0x00

/***** Globals *****/
unsigned int dma_rx_fl;                     // Flag will indicate when a DMA RX Transfer has completed.
int dma_rx, dma_tx;                         // Global DMA channels.

unsigned long ONESHOT_WAIT_TIME = MXC_DELAY_MSEC(70); // Time needed for the MAX30205 to complete a new measurement.
                                                      // Please leave this as is.

unsigned long TEMPERATURE_LOOP_IDLE = MXC_DELAY_MSEC(1500); // Wait time at the end of the measurement loop in main. Configurable!

/***** Interrupts *****/
void DMA1_IRQHandler(void) {
	dma_rx_fl = 1; //Indicate rx transfer is finished.
	DMA_Handler(dma_rx);
	return;
}

/***** Functions *****/
/**
* @brief       DMA_I2CWrite. Blocking function for an I2C Write transaction using DMA.
*              Transaction will finish before function return.
*
* @param[in]   slave_addr: Address for the slave device to be written to using I2C.
* @param[in]   reg_addr: Register address on the slave device. *Note: This function is
*                         currently designed for only 8-bit addresses.
* @param[in]   *data: Pointer to an array of data to be written to the target register.
* @param[in]   data_size: Length of the data array to be written. Can be used to represent registers longer
*                          than 1 byte, multiple registers, etc.
*
* @return       void
* @pre          Expects I2C_Init and DMA enabled on the I2C registers before function call.
* 				Example code for preparing I2C module:
*
* @code         I2C_Init(I2C_MASTER, I2C_STD_MODE, &sys_i2c_cfg);
*               I2C_MASTER->dma |= MXC_F_I2C_DMA_TX_EN | MXC_F_I2C_DMA_RX_EN; // Enable DMA stream on the I2C Bus
*               I2C_MASTER->int_en0 = MXC_F_I2C_INT_EN0_STOP;
*               // For flag polling on the I2C bus.
*
*               // Set the TX and RX thresholds to 1. Avoids FIFO overflow/underflow
*               I2C_MASTER->tx_ctrl0 = (0x1 << MXC_F_I2C_TX_CTRL0_TX_THRESH_POS);
*               I2C_MASTER->rx_ctrl0 = (0x1 << MXC_F_I2C_RX_CTRL0_RX_THRESH_POS);
*               I2C_MASTER->ctrl |= MXC_F_I2C_CTRL_MST;	  //Set Master Control bit
*@endcode
* 				Also Expects DMA_Init() and a global dma_tx channel. Ensure channel comes from
*               DMA_AcquireChannel() and is configured with DMA_Config(args). See "dma.h" for more information.
*
* @post         Clears I2C interrupts.
****************************************************************************/

void DMA_I2CWrite(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, int data_size) {
	// Put data into array with register address (pass data_size-->0 if writing register pointer before read)
	uint8_t write_config[data_size + 1];
	write_config[0] = reg_addr;
	for (int i = 1; i < data_size + 1; i++) {write_config[i] = data[i - 1];}

	I2C_MASTER->int_fl0 = 0xFF; // Make sure interrupts are cleared.
	DMA_Stop(dma_tx);

	// Note: For Write transactions, the slave address should be loaded before starting the channel.
	I2C_MASTER->fifo = (slave_addr & ~(0x1)); //Load Slave write address
	DMA_SetSrcDstCnt(dma_tx, write_config, 0, data_size + 1);
	DMA_Start(dma_tx); // Start DMA Channel

	// The following 2 tasks must be done manually to start an I2C TX DMA stream:
	// 1) Load the slave address into the TX FIFO
	// 2) Send a Start bit

	while (I2C_MASTER->status & MXC_F_I2C_STATUS_TX_EMPTY); // Wait for address load
    I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_START; // Generate start bit
    I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_STOP;  // Generate stop bit
    while(!(I2C_MASTER->int_fl0 & MXC_F_I2C_INT_FL0_STOP)); // Wait for stop condition on the bus.

    DMA_Stop(dma_tx);
    return;
}

/***** Functions *****/
/**
* @brief       DMA_I2CRead. Blocking function for an I2C Read transaction using DMA.
*              Transaction will finish before function return.
*
* @param[in]   slave_addr: Address for the slave device to be read from using I2C.
* @param[in]   reg_addr: Register address on the slave device. *Note: This function is
*                         currently designed for only 8-bit addresses.
* @param[in]   *data: Pointer to an array for the Read data.
* @param[in]   data_size: Size of the data array to be read into. Allows for registers with multiple bytes.
*
* @return      void
* @pre         Expects the same initialization as the DMA_I2CWrite function above
*              Additionally expects a global dma_rx channel. Ensure channel comes from
*              DMA_AcquireChannel() and is configured SEPARATELY from dma_tx and using DMA_Config(args).
*              See "dma.h" for more information.
*
* @post        Clears I2C interrupts.
****************************************************************************/
void DMA_I2CRead(uint8_t slave_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_size) {
	// Initialize configuration array
	uint8_t read_config[data_size];
	for (int i = 0; i < data_size; i++) {read_config[i] = 0;}
	I2C_MASTER->rx_ctrl1 = (data_size); // Set the amount of bytes to receive.

	DMA_Stop(dma_rx);
	DMA_SetSrcDstCnt(dma_rx, 0, data, data_size);
	DMA_Start(dma_rx);

	DMA_I2CWrite(I2C_SLAVE_ADDR, reg_addr, read_config, 0);

	// The following 2 tasks must be done manually to start an I2C TX DMA stream:
	// 1) Load the slave address into the TX FIFO
	// 2) Send a Start bit

	I2C_MASTER->fifo = (slave_addr | (0x1)); // Load slave address for reading

	while (I2C_MASTER->status & MXC_F_I2C_STATUS_TX_EMPTY); // Wait for address load
	I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_START; // Generate start bit
	I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_STOP;  // Generate stop bit
	while((I2C_MASTER->master_ctrl & MXC_F_I2C_MASTER_CTRL_STOP));

	while(!(dma_rx_fl)); // Wait for the ISR to indicate transfer complete.
	dma_rx_fl = 0;
	I2C_MASTER->int_fl0 = 0xFF; // Make sure interrupts are cleared.

	DMA_Stop(dma_rx);
	return;
}

//Temperature conversion from Celsius to Fahrenheit
double temp_CtoF(double tempCelsius) {
	double tempFahrenheit = tempCelsius * 9;
	tempFahrenheit = tempFahrenheit / 5;
	tempFahrenheit += 32;
	return tempFahrenheit;
}

/*****************************************************************************/

int main(void)
{
    uint8_t rxdata[2] = {0x00, 0x00};   // Registers with 2-byte data
    const sys_cfg_i2c_t sys_i2c_cfg = NULL; // No system specific configuration needed.
    uint8_t ONESHOT_CONFIG[1] = {0x81}; // Configuration to transmit for a oneshot temperature reading.

    printf("\n***** DMA/I2C MAX30205 Example *****\n");
    printf("This example uses the I2C Master to read/write from/to the MAX30205 I2C Slave via DMA.\n");
    printf("For this example you must connect P0.9 to SDA and P0.8 to SCL. \n\n");

    //Setup the I2CM
    I2C_Shutdown(I2C_MASTER);
    I2C_Init(I2C_MASTER, I2C_STD_MODE, &sys_i2c_cfg);
    I2C_MASTER->dma |= MXC_F_I2C_DMA_TX_EN | MXC_F_I2C_DMA_RX_EN; // Enable DMA stream on the I2C Bus
    I2C_MASTER->int_en0 = MXC_F_I2C_INT_EN0_STOP;
                               // For flag polling on the I2C bus.

    // Set the TX and RX thresholds to 1. Avoids FIFO overflow/underflow
    I2C_MASTER->tx_ctrl0 = (0x1 << MXC_F_I2C_TX_CTRL0_TX_THRESH_POS);
    I2C_MASTER->rx_ctrl0 = (0x1 << MXC_F_I2C_RX_CTRL0_RX_THRESH_POS);
	I2C_MASTER->ctrl |= MXC_F_I2C_CTRL_MST;	  //Set Master Control bit

    //Initialize DMA Channels
    DMA_Init();
    dma_tx = DMA_AcquireChannel();
    DMA_ConfigChannel(  dma_tx,                 //ch
                    DMA_PRIO_HIGH,              //prio
                    DMA_REQSEL_I2C0TX,          //reqsel
                    1,                          //reqwait_en
                    DMA_TIMEOUT_4_CLK,          //tosel
                    DMA_PRESCALE_DISABLE,       //pssel
                    DMA_WIDTH_BYTE,             //srcwd
                    1,                          //srcinc_en
                    DMA_WIDTH_BYTE,             //dstwd
                    0,                          //dstinc_en
                    1,                          //burst_size (bytes-1)
                    1,                          //chdis_inten
                    0                           //ctz_inten
    				);

    dma_rx = DMA_AcquireChannel();
    DMA_ConfigChannel(  dma_rx,                 //ch
                    DMA_PRIO_MEDHIGH,           //prio
                    DMA_REQSEL_I2C0RX,          //reqsel
                    1,                          //reqwait_en
                    DMA_TIMEOUT_4_CLK,          //tosel
                    DMA_PRESCALE_DISABLE,       //pssel
                    DMA_WIDTH_BYTE,             //srcwd
                    0,                          //srcinc_en
                    DMA_WIDTH_BYTE,             //dstwd
                    1,                          //dstinc_en
                    1,                          //burst_size (bytes-1)
                    1,                          //chdis_inten
                    0                           //ctz_inten
        			);
    DMA_EnableInterrupt(dma_rx);
    NVIC_EnableIRQ(DMA1_IRQn);
    __enable_irq();


    /****** READING A PERIPHERAL REGISTER *****/
    // This section demonstrates a single register read from the MAX30205 Hysteresis temp register.
    printf("Master read, Slave write from MAX30205 T_HYST Register... \n");
    DMA_I2CRead(I2C_SLAVE_ADDR, MAX30205_T_HYST_REG, rxdata, 2);

    printf("Printing read data: ");
    printf("%d %d\n", rxdata[0], rxdata[1]);
    printf("\nExample complete.\n\n");

    /***** WRITING A REGISTER AND TAKING A TEMPERATURE MEASUREMENT *****/
    // This section demonstrates a single write and read from the MAX30205.
    // It writes a OneShot command to the configuration register,
    // waits for a new measurement, and then reads from the temperature register.
    printf("Master write, Slave read to MAX30205 Config Register... \n");

    DMA_I2CWrite(I2C_SLAVE_ADDR, MAX30205_CONFIG_REG, ONESHOT_CONFIG, 1);
    TMR_Delay(MXC_TMR0, ONESHOT_WAIT_TIME, NULL); // Wait for a new measurement

    printf("Reading the temperature register...\n"); // Read the new measurement
    DMA_I2CRead(I2C_SLAVE_ADDR, 0x00, rxdata, 2);

    volatile double temp_Celsius = (double)(rxdata[0]) + (double)(rxdata[1]) * pow(2.0, -8.0);
    volatile double temp_Fahrenheit = temp_CtoF(temp_Celsius);
    printf("Single Temperature Reading: \n\t %lf Celsius; %lf Fahrenheit\n", temp_Celsius, temp_Fahrenheit);
    printf("Example Complete!\n");

    /***** CONTINUOUS TEMPERATURE MEASUREMENTS *****/
    // This section takes continuous temperature measurements using the format of
    // the previous section. At the end of each loop, there is a global, configurable
    // wait period in milliseconds. The default wait period is 1500 milliseconds or 1.5 seconds.

    printf("Starting continuous temperature measurements...\n\n");
    while (1) {
    	// Take a new temperature measurement ~every 2 seconds
    	DMA_I2CWrite(I2C_SLAVE_ADDR, MAX30205_CONFIG_REG, ONESHOT_CONFIG, 1); // Send a command to take a reading
    	TMR_Delay(MXC_TMR0, ONESHOT_WAIT_TIME, NULL); // Give the sensor time to take a reading
    	DMA_I2CRead(I2C_SLAVE_ADDR, MAX30205_TEMP_REG, rxdata, 2);  // Read the temperature data

    	//Convert rxdata to temperature and print.
    	temp_Celsius = (double)(rxdata[0]) + (double)(rxdata[1]) * pow(2.0, -8.0);
    	temp_Fahrenheit = temp_CtoF(temp_Celsius);
    	printf("MAX30205 Die Temperature:\n\t %lf Celsius, %lf Fahrenheit\n", temp_Celsius, temp_Fahrenheit);

    	// Wait for a new measurement
    	TMR_Delay(MXC_TMR0, TEMPERATURE_LOOP_IDLE, NULL);
    }
    return 0;
}
