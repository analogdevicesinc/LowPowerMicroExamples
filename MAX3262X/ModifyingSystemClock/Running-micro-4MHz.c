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
//This is a sample code running at 4MHz clock.
//Reason:  A customer complained that running the Micro at 4MHz system clock with a divider of 8 is causing issues. This code tells tehm our micro works fine at that low frequencies.
//Please make sure to set up the baud rate as 650 on tera term to see the characters 



/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "board.h"
#include "nhd12832.h"
#include "tmr_utils.h"
#include "led.h"
#include "lp.h"
#include "gpio.h"


//The idea of this program is to run the micro initally at 96MHz(default clock) but once the SW1 is pressed we want it to get switched to low clock 4MHz
// So we are configuring GPIO
/* **** Definitions **** */
#define LP0_WAKE_GPIO_PORT	5
#define LP0_WAKE_GPIO_PIN	PIN_4

gpio_cfg_t gpioLP0;

/* ************************************************************************** */
int main(void)
{

    int count = 0;
    double i;
    const limit = 5000;
    // Initialize the OLED. 
    //configure GPIO pin as input with pullup - use for LP0 wakeup
       gpioLP0.port = LP0_WAKE_GPIO_PORT;
       gpioLP0.mask = LP0_WAKE_GPIO_PIN;
       gpioLP0.func = GPIO_FUNC_GPIO;
       gpioLP0.pad = GPIO_PAD_INPUT_PULLUP;
       GPIO_Config(&gpioLP0);

    NHD12832_Init();
    // Enter 4MHz when SW1 is pushed
           while(!PB_Get(SW1))
            {
        	   NHD12832_ShowString((uint8_t*)"96 Mhz clock", 0, 5);
        	           LED_On(0);
        	         
        	           for(i = 0; i<= limit; i++)
        	           {

        	           }

        	           LED_Off(0);
        	           for(i = 0; i<= limit; i++)
        	           {

        	           }

        	           
        	           printf("count = %d\n", count++);
            }

//We are using the following tyoes to set this can be found in clkman.h


//typedef enum {
//    /** Clock select for 96MHz oscillator */
//    CLKMAN_SYSTEM_SOURCE_96MHZ,
//    /** Clock select for 4MHz oscillator */
//    CLKMAN_SYSTEM_SOURCE_4MHZ
//}
//clkman_system_source_select_t;

///**
// * @brief Defines clock scales for the system clock.
// * @note 4MHz System source can only be divided down by a maximum factor of 8.
 //*/
//typedef enum {
//    CLKMAN_SYSTEM_SCALE_DIV_1,     /** Clock scale for dividing system by 1  */
//    CLKMAN_SYSTEM_SCALE_DIV_2,     /** Clock scale for dividing system by 2  */
//    CLKMAN_SYSTEM_SCALE_DIV_4,     /** Clock scale for dividing system by 4  */
//    CLKMAN_SYSTEM_SCALE_DIV_8,     /** Clock scale for dividing system by 8  */
//    CLKMAN_SYSTEM_SCALE_DIV_16     /** Clock scale for dividing system by 16  */
//} clkman_system_scale_t;


          // Set the System clock to the 4MHz oscillator
    CLKMAN_SetSystemClock(CLKMAN_SYSTEM_SOURCE_4MHZ, CLKMAN_SYSTEM_SCALE_DIV_8);

    while(1) {



        NHD12832_ShowString((uint8_t*)"4 Mhz clock", 0, 4);
        LED_On(0);
       
        for(i = 0; i<= limit; i++)
        {

        }
        LED_Off(0);
        
        for(i = 0; i<= limit; i++)
        {

        }
        printf("count = %d\n", count++);
    }
    
}

