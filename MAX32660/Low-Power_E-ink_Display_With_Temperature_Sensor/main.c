/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
* 
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
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
/*
*	NOTE: This is a working library for the "Wearable Temperature Sensor LP"
* 	project. This library is a working version specific to this project but can be modified
* 	for other project use cases. See full project description at:
* 	
* 		https://www.hackster.io/thomas-lyp/human-body-temperature-to-e-ink-display-part-1-8d2500
*	
*	Also note that this project is setup using te MAX32660 EVkit with the Maxim Toolchain in the Eclipse IDE.
*	The screen used for this project is the Adafruit SSD1608, a 1.54" electronic ink display. Ordering information
*	can be found here:
*	
*		https://www.adafruit.com/product/4196?gclid=EAIaIQobChMIlYfCsqaI5AIVEbvsCh2YJgD8EAQYASABEgImI_D_BwE
*		
*	The screen is 200 pixels x 200 pixels, which accounts for 40,000 pixels total. One byte of information contains 8 bits,
*	so sending 5,000 bytes will effectivly change all 40,000 pixels. Whenever a function uses X and Y coordinate system,
*	Assume bottom left corner is point (0,0), with x and y values ranging form 0 - 200. No points out of this range will
*	be accepted.
*******************************************************************************
* @file main.c
*
* @version 1.1
*
* Started: 10JUL19
*
* Updated: Legal Headers. 
*/
 //Includes
 #include <stdio.h>
 #include <stdint.h>
 #include <string.h>
 #include "tmr_utils.h"
 #include "mxc_delay.h"
 #include "tmr.h"
 #include "pb.h"
 #include "NVIC_table.h"
 #include "MAX30205_Sensor.h"
 #include "SSD1608_Display.h"
 #include "Wearable_Temperature_Sensor_LP.h"
 
 //Globals
 extern uint8_t val[5];
 extern uint8_t buttonPressed;
 int main(void)
 {
 	printf("Initialization Begin\n");
 	//Initialize SPI
 	SPIinit();
 
   //Initialize GPIO Pins
 	 pinInit();
 
   StartScreen();
  
 	NVIC_SetVector(RTC_IRQn, alarmHandler);
 	SYS_ClockDisable(SYS_PERIPH_CLOCK_UART0);
 
   //Initialize Sensor Addresses and pins
    MAX30205_I2CSetup();
 
   //Set Sensor into Sleep Mode
   MAX30205_TempSenseSleep();
 
    //Button Setup for entering/exiting sleep mode
    PB_RegisterCallback(0, buttonHandler);
    TMR_Delay(MXC_TMR0, MSEC(3000), NULL);
 
    //Main loop that continuously updates temperature on e-ink display
    while(1)
    {
    	printf("Returned to Active Mode\n");
    	MAX30205_OneShotSense();
 
    	//Give time to make a new reading
    	TMR_Delay(MXC_TMR0, MSEC(50), NULL);
 
    	//Convert to Fahrenheit and display new value on screen
    	double Celsius = MAX30205_TempRead();
    	double Fahrenheit = MAX30205_CtoF(Celsius);
    	TempValues(Fahrenheit);
    	BufferUpdate(val);
    	displayScreen();
 
    	//User-requested low-power mode
    	if (buttonPressed == 1)
    	{
    		//Set trigger to wake up 32660 on RTC
 			 LP_EnableRTCAlarmWakeup();
 			 setTrigger(0);
 
 			 //pinSleep();
 
 			 //Prepare micro for deep-sleep and then enter low-power mode
 			 LP_DisableBandGap();
 			 LP_DisableVCorePORSignal();
 			 LP_EnableRamRetReg();
 			 LP_DisableBlockDetect();
 			 LP_EnableFastWk();
 			 LP_EnterDeepSleepMode();
    	}
 
    	//Exit low-power mode
    	else if (buttonPressed >= 2){
    		buttonPressed = 0;
 		}
 
    	//Stay in active-mode
    	else{
   		TMR_Delay(MXC_TMR0, MSEC(6500), NULL);
    	}
    }
    return 0;
 }

