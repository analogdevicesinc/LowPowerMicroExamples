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
 *
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   Hello World!
 * @details This example uses the UART to print to a terminal and flashes an LED.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "led.h"
#include "nhd12832.h"
#include "tmr_utils.h"
#include "i2cs.h"
#include "i2cm.h"
#include "Max30208_x.h"

/***** Definitions *****/
/***** Globals *****/
/***** Functions *****/
// *****************************************************************************
int main(void)
{
	int sample = 0;
	I2CSetup(); // setup Master I2C

   // Print to the OLED
   NHD12832_Init();
   NHD12832_ShowString((uint8_t*)"Max30208v3", 0, 4);


   while (1)
   {

	   printf("======SAMPLE: %d ======\n", sample);
	   sample += sample + 1;

      // Testing ID Part Register
      uint8_t ID_Reg[1];
      int err0 = read_PartID (&ID_Reg);
      printf("Read_PartID: 0x%X\t, Error = %d\n", ID_Reg[0], err0);

      // Testing FiFo Configuration1 Register
      int err1 = write_FiFoConfig1 ();
      printf("Write_FiFoConfig1 \t, Error = %d\n", err1);

      // Testing Setup Register
      int err2 = write_SetupRegister ();
      printf("Write_SetupRegister \t, Error = %d\n", err2);

      //Testing Status Register
      uint8_t status[1];
      int err3 = read_StatusRegister (&status);
      printf("Read_Status: 0x%X\t, Error = %d\n", status[0], err3);

      // Testing FiFo OverFlow Counter
      uint8_t counter[1];
      int err4 =  read_OverFlow (&counter);
      printf("Read_OVFCounter: 0x%X\t, Error = %d\n", counter[0], err4);

      //Test FiFo data
      uint16_t FiFo_data[1];
      int err5 = read_FiFoDataRegister (&FiFo_data);
      printf("Read_FiFoData: 0x%X\t, Error = %d\n", FiFo_data[0], err5);

      // Test Temperature read in c
      double temp= 0.00;
      temp = read_Temperature (&FiFo_data[0]);
      printf ("TEMPERATURE: %f C", temp);
      printf ("\n");

      TMR_Delay(MXC_TMR0, MSEC(50));
   }
}
