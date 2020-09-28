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
********************************************************************************/

/**
 * @file    	main.c
 * @version 	1.0
 * Started:		13JUL2020
 *
 * @brief   	I2C (Loopback) Example
 * @details 	This example uses the I2C Master to read/write from/to the I2C Slave. For
 * 		        this example you must connect P0.3 to P0.9 (SDA) and P0.2 to P0.8 (SCL). The Master
 * 		        will use P0.8 and P0.9. The Slave will use P0.2 and P0.3.
 *
 * @notes		WIRING DIAGRAMS
 *              Below are the pinouts of the associated EVKits used in developing this program,
 *              and how they should be connected. Note the ned for pullup resistors on SDA and SCL.
 *
 *                      ###############
 *                      #  MAX32660   #
 *                      #             #
 *                      #             #
 *              SDA*<-- # P0_3        #
 *              SCL*<-- # P0_2        #
 *                      #        P0_8 # --> SCL*
 *                      #        P0_9 # --> SDA*
 *                      #             #
 *              GND <-- # GND  VDD_IO # --> VDD
 *                      ###############
 *
 *              * - attach a pullup resistor to VDD, approximately 2.2k - 10k Ohms
 *              x - unused (MAX30205 EVKit has lateral pin orientation).
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_config.h"
#include "i2c.h"
#include "mxc_delay.h"
#include "dma.h"
#include "board.h"

/***** Definitions *****/
#define I2C_MASTER	    MXC_I2C0
#define I2C_MASTER_IDX	0

#define I2C_SLAVE	    MXC_I2C1
#define I2C_SLAVE_IDX	1
#define I2C_SLAVE_ADDR	(0x51<<1)
#define I2C_TIMEOUT     MXC_DELAY_MSEC(1)
#define MAX_SIZE  100 	// Configurable example size. Please set in the range between 1 and 256 only.

/***** Globals *****/
i2c_req_t slave_req, master_req;
static uint8_t txdata[MAX_SIZE];
static uint8_t rxdata[MAX_SIZE];
volatile unsigned int i2c_flag;
int dma_tx, dma_rx;

/***** Functions *****/
//Slave interrupt handler
void I2C1_IRQHandler(void)
{
    I2C_Handler(I2C_SLAVE);
    return;
}

//Prints out human-friendly format to read txdata and rxdata
void print_data(void)
{
    int i;
    printf("txdata: ");
    for(i = 0; i < MAX_SIZE; ++i) {
        printf("%d\t", txdata[i]);
    }

    printf("\nrxdata: ");
    for(i = 0; i < MAX_SIZE; ++i) {
        printf("%d\t", rxdata[i]);
    }
    printf("\n");
    return;
}

//Compare txdata and rxdata to see if they are the same
void verify(void)
{
    int i, fails = 0;
    for(i = 0; i < MAX_SIZE; ++i) {
        if(txdata[i] != rxdata[i]) {
            ++fails;
	    }
    }
    if(fails > 0) {
        printf("Fail.\n");
    } else {
        printf("Pass.\n");
    }

    return;
}

// *****************************************************************************
int main(void)
{
    int i = 0;
    const sys_cfg_i2c_t sys_i2c_cfg = NULL; /* No system specific configuration needed. */

    printf("\n***** I2C Loopback Example *****\n");
    printf("This example uses the I2C Master to read/write from/to the I2C Slave with DMA. For\n");
    printf("this example you must connect P0.3 to P0.9 (SDA) and P0.2 to P0.8 (SCL). The\n");
    printf("Master will use P0.8 and P0.9. The Slave will use P0.2 and P0.3. \n\n");

    //Setup the I2CM
    I2C_Shutdown(I2C_MASTER);
    I2C_Init(I2C_MASTER, I2C_STD_MODE, &sys_i2c_cfg);
    MXC_I2C0->dma |= MXC_F_I2C_DMA_TX_EN | MXC_F_I2C_DMA_RX_EN; // Enable DMA stream on the I2C Bus
    I2C_MASTER->ctrl |= MXC_F_I2C_CTRL_MST;				    // Set Master Control bit

    // Set the FIFO thresholds to trigger the DMA Bursts
    I2C_MASTER->tx_ctrl0 = (0x1 << MXC_F_I2C_TX_CTRL0_TX_THRESH_POS);
    I2C_MASTER->rx_ctrl0 = (0x1 << MXC_F_I2C_RX_CTRL0_RX_THRESH_POS);

    //Setup the I2CS
    I2C_Shutdown(I2C_SLAVE);
	I2C_Init(I2C_SLAVE, I2C_STD_MODE, &sys_i2c_cfg);
    NVIC_EnableIRQ(I2C1_IRQn);

	//Initialize DMA Channels
    DMA_Init();
    dma_tx = DMA_AcquireChannel();
    DMA_ConfigChannel(  dma_tx,                 //ch
                    DMA_PRIO_HIGH,              //prio
                    DMA_REQSEL_I2C0TX,          //reqsel
                    0,                          //reqwait_en
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
    DMA_SetSrcDstCnt(dma_tx, txdata, 0, MAX_SIZE);

    dma_rx = DMA_AcquireChannel();
    DMA_ConfigChannel(  dma_rx,                 //ch
                    DMA_PRIO_HIGH,              //prio
                    DMA_REQSEL_I2C0RX,          //reqsel
                    0,                          //reqwait_en
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
    DMA_SetSrcDstCnt(dma_rx, 0, rxdata, MAX_SIZE);
    __enable_irq();


    /************ MASTER WRITE SLAVE READ *************/
    //Initialize test data
    for(i = 0; i < MAX_SIZE; i++) {
        txdata[i] = i;
        rxdata[i] = 0;
    }

    //Prepare SlaveAsync
    slave_req.addr = I2C_SLAVE_ADDR; // Sets up a request to be used in both examples like an external slave device.
    slave_req.tx_data = txdata;      // Ignores rxdata on write transactions, and ignores txdata on read transactions.
    slave_req.tx_len = MAX_SIZE;
    slave_req.rx_data = rxdata;
    slave_req.rx_len = MAX_SIZE;
    slave_req.restart = 0;

    I2C_SlaveAsync(MXC_I2C1, &slave_req);

    printf("Master write, Slave read ... \n");
    printf("New test data:\n");
    print_data();

    // The following 2 tasks must be done manually to start an I2C TX DMA stream:
    // 1) Load the slave address into the TX FIFO
    // 2) Send a Start bit

    // Note: For WRITE transactions, the slave address should be loaded before starting the channel.
    I2C_MASTER->fifo = (I2C_SLAVE_ADDR & ~(0x1)); // Load the FIFO with Slave Write Address before starting a TX DMA stream.
    DMA_Start(dma_tx); // Start the DMA Stream

    I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_START; // Generate start bit: Triggers I2C DMA Stream!
    I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_STOP; // Generate Stop bit manually.
    while(I2C_MASTER->master_ctrl & MXC_F_I2C_MASTER_CTRL_STOP); //Wait for the stop condition before moving on

    __disable_irq();
    DMA_Stop(dma_tx);			// Stop the TX DMA stream.
    DMA_ReleaseChannel(dma_tx); // Properly release the DMA channel.

    I2C_AbortAsync(&slave_req);

    printf("Checking test data:\n");
    print_data();
    verify();

    printf("\nExample complete.\n\n");

    /************ SLAVE WRITE, MASTER READ *************/

    // Re-Initialize data
    for(i = 0; i < MAX_SIZE; i++) {
            rxdata[i] = 0;
    }
    slave_req.tx_data = txdata; // Reset the pointers for SlaveAsync request; previous transaction modifies them.
    slave_req.rx_data = rxdata;
    I2C_MASTER->rx_ctrl1 = MAX_SIZE; // Set the size of the receive transaction in bytes. Only holds 0-256; 0 represents 256 bytes as well.

    printf("Slave write, Master read ... \n");
    printf("New test data:\n");
    print_data();
    DMA_Start(dma_rx);
    __enable_irq();

    I2C_SlaveAsync(MXC_I2C1, &slave_req);

    printf("Master reads data from Slave.\n");

    // For READ transactions, the FIFO can be loaded with the address right before starting.
    I2C_MASTER->fifo = I2C_SLAVE_ADDR | 0x1; // Load slave address for reading
    I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_START; // Generate a Start bit to initiate the transaction

    I2C_MASTER->master_ctrl |= MXC_F_I2C_MASTER_CTRL_STOP;
    while(I2C_MASTER->master_ctrl & MXC_F_I2C_MASTER_CTRL_STOP); // Wait for stop condition.
    __disable_irq();

    // Release the DMA Module
    DMA_Stop(dma_rx);
    DMA_ReleaseChannel(dma_rx);

    // Verify the data transfer
    print_data();
    verify();
    printf("\nExample complete.\n");

    while(1);
    return 0;
}
