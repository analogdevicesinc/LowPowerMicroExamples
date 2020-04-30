
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
*******************************************************************************/
//This code runs the traditional SPI masterB for the MAX326XX series micros
//This code is designed using SPIM2B. P5.0 acts as SCLK and P5.1 acts as MOSI and P5.3 acts as SS



/***** Includes *****/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_config.h"
#include "clkman.h"
#include "ioman.h"
#include "spim.h"
#include "mx25.h"
#include "led.h"
#include "nhd12832.h"
#include "tmr_utils.h"

/***** Definitions *****/
#define SPI_frequency           8000000    // 48 MHz maximum, 20 kHz minimum-- AKA SPI frequency  (Modify this parameter to change th SPI frequency
#define SPI_SPIM_WIDTH          SPIM_WIDTH_1
#define BUFF_SIZE               2

/***** Globals *****/

/***** Functions *****/

/******************************************************************************/
int main(void)
{
    int error, i, count;
    uint8_t txdata[BUFF_SIZE] = {0xA, 0xA};  //Modify this values to change the data write
    uint8_t rxdata[BUFF_SIZE];

    //The following lnes of code confirms USB-UART is working good
    printf("\n\n***** SPIM sample Master Example *****\n");
    printf(" System freq \t: %d Hz\n", SystemCoreClock);
    printf(" SPI freq \t: %d Hz\n", SPI_frequency);
    printf(" SPI data width : %d bits\n", (0x1 << SPI_SPIM_WIDTH));
    printf(" Write/Verify   : %d bytes\n\n", BUFF_SIZE);

    // Initialize the data buffers
    for(i = 0; i < BUFF_SIZE; i++) {
        txdata[i] = 0xA;
    }

    //There are the following steps to write data through SPI
    // Step1: Initialize the SPI


    // Initialize the SPIM using spim_cfg_t f. This structure can be found in spim.h.  It has three elements
    //1)  Mode:  SPIM mode selection, 0 to 3.
    //2)  Mask of active levels for the slave select signals, see #spim_ssel_t.
    //3)  Baud rate in Hz


    spim_cfg_t cfg;
    cfg.mode = 0;
    cfg.ssel_pol = 0;
    cfg.baud = SPI_frequency;

    // Structure type for SPIM System Configuration can be found in maxc_sys.h.
    sys_cfg_spim_t sys_cfg;

    //IO mapping for SPIM2 in this case we are using Map B secondary function of SPIM2 connect to
    // Pins 5.0 and 5.1

    ioman_map_t map;
    map = 1;


    // Tells how to structure th pin map for SPI

 //ioman_cfg_t IOMAN_SPIM2(ioman_map_t map, int io_en, int ss0, int ss1, int ss2, int sr0, int sr1, int quad, int fast);

    sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_SPIM2(map, 1,   1,  0,    0,    0,0,0, 1);
    sys_cfg.clk_scale = CLKMAN_SCALE_AUTO; //Enumeration type for selecting the clock scale for the system or peripheral module found in clkman.h


    // Initializing the SPI
    if((error = SPIM_Init(MXC_SPIM2, &cfg, &sys_cfg)) != E_NO_ERROR) {
        printf("Error initializing SPIM %d\n", error);
        while(1) {}
    } else {
        printf("SPIM Initialized\n");
    }

   //Writing data to SPI
   //Before writing data we have to Initialize the request structure

 /*   struct spim_req {
        uint8_t             ssel;       < Number of the Slave Select to use.
        uint8_t             deass;      < Set to de-assert slave select at the completions of the transaction.
        const uint8_t       *tx_data;   < Pointer to a buffer to transmit data from.
        uint8_t             *rx_data;   /**< Pointer to a buffer to store data received.
        spim_width_t        width;      /**< Number of data lines to use, see #spim_width_t.
        unsigned            len;        /**< Number of bytes to send from the \p tx_data buffer.
        unsigned            read_num;   /**< Number of bytes read and stored in \p rx_data buffer.
        unsigned            write_num;  /**< Number of bytes sent from the \p tx_data buffer, this will be filled by the driver after up to \p len bytes have been transmitted.
        spim_callback_fn    callback;   /**< Function pointer to a callback function if desired, NULL otherwise
    };*/

    spim_req_t spim_req;


    spim_req.ssel = 1;
    spim_req.deass = 1;
    spim_req.tx_data = txdata;
    spim_req.rx_data = rxdata;
    spim_req.width = SPIM_WIDTH_1;
    spim_req.len = 2;
    spim_req.write_num = 2;
    spim_req.read_num = 2;
    spim_req.callback = NULL;


   //Defining the SPIM clocks

    //int SPIM_Clocks(mxc_spim_regs_t *spim, uint32_t len, uint8_t ssel, uint8_t deass);
  // Defining the SPIM clocks
  SPIM_Clocks(MXC_SPIM2, 2, 1,1);


while(1)
{
	// There are two types of SPI writies provided by Maxim in this case we are using SPIM_trans which means Control of execution
	//has to send all the data to transmit FIFO before it returns from the SPI function (Recommended when not using interrupts)

	count  = SPIM_Trans(MXC_SPIM2, &spim_req);
	printf("Data printed %d\n", count);
	LED_On(0);
	        TMR_Delay(MXC_TMR0, MSEC(500));
	        LED_Off(0);
	        TMR_Delay(MXC_TMR0, MSEC(500));
}


}






