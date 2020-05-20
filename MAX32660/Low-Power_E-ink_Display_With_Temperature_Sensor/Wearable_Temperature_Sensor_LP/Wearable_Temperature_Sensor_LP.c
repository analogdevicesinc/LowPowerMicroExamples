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
* @file Wearable_Temperature_Sensor_LP.c
*
* @version 1.0
*
* Started: 10JUL19
*
* Updated: No current revisions
*/

#include "Wearable_Temperature_Sensor_LP.h"
#include "SSD1608_Display.h"
#include "SSD1608_Display_LUT.h"

/***** LUT Arrays holding buffer update information. Each array holds a bitmap for each number *****/
uint8_t ONE [];
uint8_t TWO [];
uint8_t THREE [];
uint8_t FOUR [];
uint8_t FIVE [];
uint8_t SIX [];
uint8_t SEVEN [];
uint8_t EIGHT [];
uint8_t NINE [];
uint8_t ZERO [];
uint8_t logo [];			//Holds data for Maxim Logo bitmap
uint8_t screen [];			//Holds data for background image of temperature sensor display screen

/*
 * @brief	Checks for RTC alarm flags and then clears them once set
 */
void alarmHandler(void)
{

    int flags = RTC_GetFlags();
    alarmed = 1;

    if (flags & MXC_F_RTC_CTRL_ALSF) {
        RTC_ClearFlags(MXC_F_RTC_CTRL_ALSF);
    }

    if (flags & MXC_F_RTC_CTRL_ALDF) {
        RTC_ClearFlags(MXC_F_RTC_CTRL_ALDF);
    }
}

/*
 * @brief	Initialize RTC and prepare UART for deep-sleep
 * @param[(in)] <waitForTrigger> { Integer to decide whether system should wait for trigger or not }
 */
void setTrigger(int waitForTrigger)
{
    alarmed = 0;
    sys_cfg_rtc_t sys_cfg;
    sys_cfg.tmr = MXC_TMR0;
    while(RTC_Init(MXC_RTC, 0, 0, &sys_cfg) == E_BUSY);
    while(RTC_SetTimeofdayAlarm(MXC_RTC, DELAY_IN_SEC) == E_BUSY);
    while(RTC_EnableRTCE(MXC_RTC) == E_BUSY);
    if(waitForTrigger)
    {
        while(!alarmed);
    }

    // Wait for serial transactions to complete.
    while(UART_PrepForSleep(MXC_UART_GET_UART(CONSOLE_UART)) != E_NO_ERROR);
}


/*
 * @brief	Function that increments global variable (buttonPressed) everytime that the button on MAX32660 is pressed.
 * @param[(in)] <pb> { void pointer to button  on MAX32660 microcontroller}
 */
void buttonHandler(void *pb)
{
    buttonPressed++;
}

/*
 * @brief	Provides the Look Up Table data for a specified number so that it can be added into buffer1
 * @note       { Called internally within #BufferUpdate }
 * @param[(in)] <number> { Value from temperature digit that is going to be dispalyed onto screen }
 * @return     { pointer to Look Up Table array that holds bitmap for input value }
 */
uint8_t *NumberFinder(int number)
{
	switch(number)
	{
		case 0:
			return(ZERO);
			break;
		case 1:
			return(ONE);
			break;
		case 2:
			return(TWO);
			break;
		case 3:
			return(THREE);
			break;
		case 4:
			return(FOUR);
			break;
		case 5:
			return(FIVE);
			break;
		case 6:
			return(SIX);
			break;
		case 7:
			return(SEVEN);
			break;
		case 8:
			return(EIGHT);
			break;
		case 9:
			return(NINE);
			break;
		default:
			return(ZERO);
			break;
	}
}


/*
 * @brief	Splits given number into each significant tens place (e.g hundreds, tens, ones, etc.)
 * @param[(in)] <temp> { Temperature read from MAX30205 sensor and converted using #MAX30205_CtoF }
 */
void TempValues (double temp)
{
	int hold = temp/100;
	int rem = trunc(hold);
	val[0] = rem;				//Hold hundreds place
	hold = trunc(temp / 10);
	rem = (hold % 10);
	val[1]=rem;					//Hold tens place value
	hold = trunc(temp);
	rem = hold % 10;
	val[2]=rem;					//Hold ones place value
	hold = trunc(temp*10);
	rem = hold % 10;
	val[3]=rem;					//Hold tenths place value
	hold = trunc(temp*100);
	rem = hold % 10;
	val[4]=rem;					//Hold hundreths place value
}

int flag = 0;	//Initializing flag
/*
 * @brief	Updates buffer based on pre-calculated template and pre-calculated numbers (LUT)
 * @note       { Must first call #TempValues in order to find each digit that must be updated in buffer1. Each digit being updated on screen is within Y range of
 * from y=80 and y = 115. Each digit is 4 bytes long on X axis, which is 32 pixels. (hundres digit from x= 1 - x = 3; tens digit from x = 4 - x = 6; etc.)}	
 * @param[(in)] <pos> { Array of integer values of each ten's place in temperature (e.g hundreds, tens, ones, etc) }
 */
void BufferUpdate (uint8_t *pos)
{
	if (flag == 0)
	{
		for (int z = 0;z<ARRAY_SIZE;z++)
		{
			buffer1[z] = screen[z];
		}
		flag = 1;
	}
	uint8_t num;
	uint8_t *p;
	int big1=0, big2=0, big3=0, big4=0, big5=0;
	for(int y=DIGIT_UPDATE_Y_START;y<DIGIT_UPDATE_Y_END;y++)
	{
		for(int x=DIGIT_UPDATE_X_START;x<DIGIT_UPDATE_X_END;x++)
		{
			int index = x + (y * 25);
			if( y>=80 && y<115)
			{
				if(x>HUNDREDS_DIGIT_START && x<HUNDREDS_DIGIT_END && pos[0] != 0)
				{
					num = pos[0];
					p = NumberFinder(num);
					buffer1[index]= p[big1];
					big1++;
				}
				else if(x>TENS_DIGIT_START && x<TENS_DIGIT_END)
				{
					num = pos[1];
					p = NumberFinder(num);
					buffer1[index]= p[big2];
					big2++;
				}
				else if(x>ONES_DIGIT_START && x<ONES_DIGIT_END)
				{
					num = pos[2];
					p = NumberFinder(num);
					buffer1[index]=p[big3];
					big3++;
				}
				else if(x>TENTHS_DIGIT_START && x<TENTHS_DIGIT_END)
				{
					num = pos[3];
					p = NumberFinder(num);
					buffer1[index]=p[big4];
					big4++;
				}
				else if(x>HUNDRETHS_DIGIT_START && x<HUNDRETHS_DIGIT_END)
				{
					num = pos[4];
					p = NumberFinder(num);
					buffer1[index]= p[big5];
					big5++;
				}
				else
				{
						buffer1[index]= screen[index];
				}
			}
			else
			{
				buffer1[index]= screen[index];
			}
		}
	}
}

/*
 * @brief	Send a start screen to the display as temperature sensor is being configured. Sends data from "logo" array in LUT
 */
void StartScreen(void)
{
	  powerUp();
	  setRAM(0, 0);
	  writeRAM();

	  GPIO_OutSet(&dc);
	  BitMapTransfer(logo, ARRAY_SIZE);		//Send logo screen information to display

	  GPIO_OutSet(&cs);
	  updateScreen();
}

