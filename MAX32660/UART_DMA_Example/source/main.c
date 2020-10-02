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
 * @file    main.c
 * @brief   Main for UART example.
 * @details This example loops back the TX to the RX on UART0. For this example
 *          you must connect a jumper across P0.4 to P0.5. UART_BAUD and the BUFF_SIZE
 *          can be changed in this example.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_sys.h"
#include "uart.h"
#include "board.h"
#include "wdt.h"

/***** Definitions *****/
#define UART_BAUD           115200	// UART Baud Rate
#define DATA_SIZE           512		// Amount of data to transmit. Not related to 512 clocks setting for the DMA timeout

/***** Globals *****/

volatile int read_flag;				// Global flags for indicating operations complete

int dma_rx, dma_tx;                 // Global DMA channels.
/***** Functions *****/

/******************************************************************************/
/** @brief callback function to be called after the DMA rx transaction occurs. **/
void dma_read_cb(int a, int b)
{
    read_flag = 0;
}

/******************************************************************************/
/** @brief DMA RX interrupt routine. Will call the read_cb function. **/
void DMA1_IRQHandler(void)
{
    DMA_Handler(dma_rx); //
}

/******************************************************************************/
int main(void)
{
    const sys_cfg_uart_t sys_uart_cfg = { // Set the configuration for the UART pins.
        MAP_A, // GPIO pin mapping
        UART_FLOW_DISABLE, // flow control
    };
    int error, i;
    uint8_t txdata[DATA_SIZE];
    uint8_t rxdata[DATA_SIZE];

    printf("\n\n***** UART Example *****\n");
    printf("\nConnect UART0A TX (P0.4) to UART0A RX (P0.5) for this example.\n\n");
    printf(" System freq \t: %d Hz\n", SystemCoreClock);
    printf(" UART freq \t: %d Hz\n", UART_BAUD);
    printf(" Loop back \t: %d bytes\n\n", DATA_SIZE);

    /***** Initialize the data buffers *****/
    for (i = 0; i < DATA_SIZE; i++) {
        txdata[i] = i;
    }
    memset(rxdata, 0x0, DATA_SIZE);

    /***** Setup the interrupt *****/
    NVIC_EnableIRQ(DMA1_IRQn);

    /***** Initialize the UART module *****/
    uart_cfg_t cfg;
    cfg.parity = UART_PARITY_DISABLE;	// No Parity bit
    cfg.size = UART_DATA_SIZE_8_BITS;	// 8 Bits
    cfg.stop = UART_STOP_1;				// Stop bit enabled
    cfg.flow = UART_FLOW_CTRL_EN;		// Flow Control enabled
    cfg.pol = UART_FLOW_POL_EN;			// Flow Control polarity
    cfg.baud = UART_BAUD;				// Set baud rate

    printf("Enabling UART0A.\n");
    error = UART_Init(MXC_UART_GET_UART(0), &cfg, &sys_uart_cfg); // Initialize the UART module

    // Enable DMA on UART Register and set the TX/RX Interrupt Thresholds.
    MXC_UART0->dma = MXC_F_UART_DMA_TDMA_EN | MXC_F_UART_DMA_RXDMA_EN
    		| (0x1 << MXC_F_UART_DMA_TXDMA_LEVEL_POS) | (0x1 << MXC_F_UART_DMA_RXDMA_LEVEL_POS);

    if (error != E_NO_ERROR) {
        printf("Error initializing UART %d. Exiting program...Check configurations before restarting.\n", error);
        while(1) {}
    }
    error = DMA_Init();
    if (error != E_NO_ERROR) {
    	printf("Error initializing DMA.\n");
    }

    /***** Initialize and Configure DMA Channels *****/
    dma_tx = DMA_AcquireChannel();
    DMA_ConfigChannel(  dma_tx,                 	//ch
                        DMA_PRIO_MEDHIGH,           //prio
                        DMA_REQSEL_UART0TX,         //reqsel
                        1,                          //reqwait_en
                        DMA_TIMEOUT_512_CLK,        //tosel
						DMA_PRESCALE_DIV256,        //pssel
                        DMA_WIDTH_BYTE,             //srcwd
                        1,                          //srcinc_en
                        DMA_WIDTH_BYTE,             //dstwd
                        0,                          //dstinc_en
                        1,                          //burst_size (bytes-1)
                        1,                          //chdis_inten
                        0                           //ctz_inten
        				);
    DMA_SetSrcDstCnt(dma_tx, txdata, 0, DATA_SIZE); // Set the Source & Dest addresses; Set the operation count.

    dma_rx = DMA_AcquireChannel();
    DMA_ConfigChannel(  dma_rx,                 //ch
                        DMA_PRIO_HIGH,           //prio
                        DMA_REQSEL_UART0RX,          //reqsel
                        1,                          //reqwait_en
                        DMA_TIMEOUT_512_CLK,          //tosel
						DMA_PRESCALE_DIV256,       //pssel
                        DMA_WIDTH_BYTE,             //srcwd
                        0,                          //srcinc_en
                        DMA_WIDTH_BYTE,             //dstwd
                        1,                          //dstinc_en
                        1,                          //burst_size (bytes-1)
                        1,                          //chdis_inten
                        0                           //ctz_inten
            			);
    DMA_SetSrcDstCnt(dma_rx, 0, rxdata, DATA_SIZE);
    DMA_EnableInterrupt(dma_rx);
    DMA_SetCallback(dma_rx, dma_read_cb); // Set the RX callback function to be used in the interrupt.

    read_flag = 1;

    /***** Start the transaction *****/
    DMA_Start(dma_rx); // Start the DMA RX channel
    DMA_Start(dma_tx); // Start the DMA TX Channel

    // Read should have completed
    // Will exit the loop if the transaction is successful or the DMA timeout occurs.
    while(read_flag != 0);

    printf("Transaction complete.\n");

    /***** Verify the data was written correctly *****/
    if ((error = memcmp(rxdata, txdata, DATA_SIZE)) != 0) {
        printf("Error verifying rxdata. Error #%d\n", error);
    } else {
        printf("Data verified as correct!\n");
    }

    while (1) {}
}
