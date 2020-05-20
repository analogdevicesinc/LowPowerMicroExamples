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
 *Based on the Adafruit Libraries:
 *Adafruit-GFX-Library - https://github.com/adafruit/Adafruit-GFX-Library
 *Adafruit_EPD library - https://github.com/adafruit/Adafruit_EPD
 *
 */

/*
* Copyright (c) 2012 Adafruit Industries.  All rights reserved. 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met: 
*
*  - Redistributions of source code must retain the above copyright notice, 
*   this list of conditions and the following disclaimer. 
*  - Redistributions in binary form must reproduce the above copyright notice, 
*   this list of conditions and the following disclaimer in the documentation 
*   and/or other materials provided with the distribution. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
* POSSIBILITY OF SUCH DAMAGE. 
********************************************************************************
*	NOTE: This is a working library for the "Wearable Temperature Sensor LP"
* 	project. This library is a working version specific to this project but can be modified
* 	for other project use cases. See full project description at:
* 	
* 		https://www.hackster.io/thomas-lyp/human-body-temperature-to-e-ink-display-part-1-8d2500
*	
*	Also note that this project is setup using te MAX32660 EVkit in Maxim Toolchain in the Eclipse IDE.
*	The screen used for this project is the Adafruit SSD1608, a 1.54" electronic ink display. Ordering information
*	can be found here:
*		https://www.adafruit.com/product/4196?gclid=EAIaIQobChMIlYfCsqaI5AIVEbvsCh2YJgD8EAQYASABEgImI_D_BwE
*		
*	The screen is 200 pixels x 200 pixels, which accounts for 40,000 pixels total. One byte of information contains 8 bits,
*	so sending 5,000 bytes will effectivly change all 40,000 pixels. Whenever a function uses X and Y coordinate system,
*	Assume bottom left corner is point (0,0), with x and y values ranging form 0 - 200. No points out of this range will
*	be accepted.
**********************************************************************************	
* @file SSD1608_Display.h
*
* @author Maxim Integrated - TTS
*
* @version 1.1
*
* Started: 05/20/2020
*
* Updated: Udated license
*/


#ifndef SSD1608_DISPLAY_H_
#define SSD1608_DISPLAY_H_

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tmr_utils.h"
#include "mxc_delay.h"
#include "tmr.h"
#include "uart.h"
#include "spi.h"
#include "gpio.h"
#include "board.h"

/**** Display Addresses ****/
#define SSD1608_SW_RESET 	0x12
#define SSD1608_WRITE_DUMMY 0x3A
#define SSD1608_WRITE_GATELINE 0x3B
#define SSD1608_WRITE_RAM  0x24
#define SSD1608_READ_RAM   0x25
#define SSD1608_SET_RAMXPOS 0x44
#define SSD1608_SET_RAMYPOS 0x45
#define SSD1608_DATA_MODE 0x11
#define SSD1608_DRIVER_CONTROL 0x01
#define SSD1608_WRITE_VCOM  0x2C
#define SSD1608_WRITE_LUT 0x32
#define SSD1608_SET_RAMXCOUNT 0x4E
#define SSD1608_SET_RAMYCOUNT 0x4F
#define SSD1608_DISP_CTRL2 0x22
#define SSD1608_MASTER_ACTIVATE 0x20

/***** Enable Pin Decelerations *****/
#define 	DC_SEL			PIN_8		//Data/Communications
 #define 	RST				PIN_9		//Reset
 #define 	CS				PIN_10		//Chip Select
 #define 	EN				PIN_11		//Enable
 #define  	GPIO_PORT       PORT_0

/***** General Definitions *****/
#define ARRAY_SIZE	5000		//Total array size to modify screen

/***** SPI Config *****/
 #define SPI0_A
 #define SPI 			SPI0A
 #define SPI_IRQ 		SPI0_IRQn
 #define SPI_SPEED      500000  // Bit Rate

/***** Variables *****/
volatile int spi_flag;
uint8_t buffer1[5000];		//Screen Update Buffer

/***** Enable GPIO Structure Decelerations *****/
gpio_cfg_t	cs;
gpio_cfg_t	dc;
gpio_cfg_t	en;
gpio_cfg_t	rst;

/**
 * @brief Eclipse Library for implementing the
 * Adafruit SSD1608 electronic ink display. The 
 * display is 1.54" big, and resolution dimensions 
 * are 200 pixels x 200 pixles. All enables must
 * be connected for screen to function properley,
 * and all enable toggles are built into library
 * functions. 
 * 
 * If user would like to display pre-determined
 * design to electronic ink display, a LUT can be added in a 
 * seperate .h file using the tutorial found under "Modification" section:
 *
 * 		https://www.hackster.io/thomas-lyp/human-body-temperature-to-e-ink-display-part-1-8d2500 
 * 		*NOTE - When using LUT and importing data from GIMP, make sure to use function
 *		"BitMapTransfer" to send data. GIMP exports data with bits in reverse order
 * 
 * @code
 *
 * //Includes
 * #include <stdio.h>
 * #include <stdint.h>
 * #include "SSD1608_Display.h"
 *
 * int main(void)
 * {
 *	printf("Program Start\n");
 *	SPIinit();
 *
 *	//Initialize Enables
 *	pinInit ();
 *	printf("Pins initialized\n");
 *
 *	ClearBuffer();
 *
 *	// Draw Checkerboard
 *	for(int i=0;i<200;i+=4)
 *	{
 *		WriteLine(i,0,i,200,0);
 *		printf("Writing line from 0,0 to %i , 200\n", i);
 *	}
 *	for(int j=0;j<200;j+=4)
 *	{
 *		WriteLine(200,j,0,j,0);
 *		printf("Line from 200 , 200 to %d , 0\n", j);
 *  }
 *	displayScreen();
 *
 *	ClearBuffer();
 *
 *	// Draw diagonal lines
 *	for(int i=0;i<200;i+=4)
 *		{
 *			WriteLine(i,0,200,200,0);
 *			printf("Writing line from 0,0 to %i , 200\n", i);
 *		}
 *		for(int j=0;j<200;j+=4)
 * 		{
 *			WriteLine(0,200,j,0,0);
 *			printf("Line from 200 , 200 to %d , 0\n", j);
 *		}
 *
 *	displayScreen();
 *
 *
 *	ClearBuffer();
 *
 *	// Draw a Starfish
 *	WriteLine(180,10,140,95,0);
 *	WriteLine(140,95,180,150,0);
 *	WriteLine(180,150,140,130,0);
 *	WriteLine(140,130,100,190,0);
 *	WriteLine(60,130,100,190,0);
 *	WriteLine(20,150,60,130,0);
 *	WriteLine(60,95,20,150,0);
 *	WriteLine(20,10,60,95,0);
 *	WriteLine(100,70,20,10,0);
 *	WriteLine(180,10,100,70,0);
 *
 *	drawCircle (100,140,10,0);
 *	drawCircle(90,165,2,0);
 *	drawCircle(90,165,1,0);
 *	drawCircle(90,165,3,0);
 *	drawCircle(110,165,1,0);
 *	drawCircle(110,165,2,0);
 *	drawCircle(110,165,3,0);
 *	drawCircle(110,165,6,0);
 *	drawCircle(90,165,6,0);
 *
 *	displayScreen();
 *
 *	return (0);
 * }
 *
 * @endcode
 */



/***** Functions *****/

/**
 * @brief	Initialize SPI protocol on MAX32660. Initial setup uses P0_6 (SCK), P0_5 (MOSI), and P0_4 (MISO)
 */
void SPIinit(void);

/**
 * @brief	Clears out buffer1 array holding new screen data
 */
void ClearBuffer(void);

/**
 * @brief	Initialize Enable Pins
 */
 void pinInit(void);

/**
 * @brief	Master Refresh command -- Transitions display
 */
void updateScreen (void);

/**
 * @brief	Sends new screen information by reversing every 8 bits (Gimp exports bitmaps least significant bit first, but display accepts most significant first)
 */
void BitMapTransfer(uint8_t *Design, int len);

/**
 * @brief	Sends all of buffer1 data to screen, and then refreshes display
 */
void displayScreen(void);

/**
 * @brief	Function used to edit screen buffer array and directly write in pixels.
 */
void drawPixel(int16_t x, int16_t y, int color);

/**
 * @brief	Draw a line between two points
 */
void WriteLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, int color);

/**
 * @brief	Draw a Circle based on center and radius
 */
void drawCircle (uint8_t x0, uint8_t y0,uint8_t r, uint8_t color);

/**
 * @brief	Boot-up the e-ink display. Runs basic startup commands based on display module's data sheet
 */
void powerUp(void);

/**
 * @brief	Set display's RAM address pointer
 */
void setRAM (uint16_t x, uint16_t y);

/**
 * @brief	Sends address to configure RAM
 */
void writeRAM();

#endif /* SSD1608_DISPLAY_H_ */
