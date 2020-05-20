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
* @file SSD1608_Display.c
*
* @author Maxim Integrated - TTS
*
* @version 1.1
*
* Started: 05/20/2020
*
* Updated: Udated license
* 
*/

#include "SSD1608_Display.h"

//Screen Setup Data -- DO NOT EDIT --
unsigned char LUT_DATA[30]= {
 	  0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 0x66, 0x69,
 	  0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 0x00, 0x00, 0x00, 0x00,
 	  0xF8, 0xB4, 0x13, 0x51, 0x35, 0x51, 0x51, 0x19, 0x01, 0x00
 };


/*
 * @brief	SPI Callback function
 */
void spi_cb(void *req, int error)
{
    spi_flag = error;
}

/*
 * @brief	Initialize SPI protocol on MAX32660. Initial setup uses P0_6 (SCK), P0_5 (MOSI), and P0_4 (MISO)
 */
 void SPIinit(void)
 {
	 while (UART_Busy(MXC_UART_GET_UART(CONSOLE_UART)));
	 Console_Shutdown();

	    	// Configure the peripheral
	 if (SPI_Init(SPI, 0, SPI_SPEED) != 0)
	 {
		 Console_Init();
	     printf("Error configuring SPI\n");
	 }
}

/*
 * @brief	Clears out buffer1 array holding screen data. Sets all bits to '1' (White)
 */
void ClearBuffer(void)
{
  memset(buffer1,0xFF,ARRAY_SIZE);
  return;
}

/*
 * @brief	Initialize Enable Pins on MAX32660. All pinouts are described in GPIO pin definitions in SSD1608_Dispaly.h
 */
void pinInit (void)
{

	cs.port = GPIO_PORT;
	cs.mask = CS;
	cs.func = GPIO_FUNC_OUT;
	cs.pad = GPIO_PAD_NONE;
	GPIO_Config(&cs);

	dc.port = GPIO_PORT;
	dc.mask = DC_SEL;
	dc.func = GPIO_FUNC_OUT;
	dc.pad = GPIO_PAD_NONE;
	GPIO_Config(&dc);

	rst.port = GPIO_PORT;
	rst.mask = RST;
	rst.func = GPIO_FUNC_OUT;
	rst.pad = GPIO_PAD_NONE;
	GPIO_Config(&rst);

	en.port = GPIO_PORT;
	en.mask = EN;
	en.func = GPIO_FUNC_OUT;
	en.pad = GPIO_PAD_NONE;
	GPIO_Config(&en);

	//Set SSD1608 pins to unselected
	GPIO_OutClr(&en);
	GPIO_OutClr(&dc);
	GPIO_OutSet(&cs);
}


/*
 * @brief	Sends out data via SPI protocol with SPI0A pins. Data is sent from MAX32660 microcontroller to SSD1608 display
 * @param[(in)] <info> { Array pointer to data that shall be sent to electronic display via SPI protocol }
 * @param[(in)] <len> { Number of bytes to send }
 */
void SPItransfer(uint8_t *info, uint16_t len)
{
    spi_req_t req;
	req.tx_data = info;			//Array pointer to data being sent
	req.rx_data = NULL;			//Only send data, do not revieve
	req.len = len;				//Length of data being sent (How many bytes to transfer)
	req.bits = 8;				//One byte of information
	req.width = SPI17Y_WIDTH_1;  // NOT applicable to SPI1A and SPI1B, value ignored
	req.ssel = 0;                // NOT applicable to SPI1A and SPI1B, value ignored
	req.deass = 0;               // NOT applicable to SPI1A and SPI1B, value ignored
	req.tx_num = 0;
	req.rx_num = 0;
	req.callback = spi_cb;
	spi_flag =1;

	SPI_MasterTrans(SPI, &req);
}

/*
 * @brief	Sends reset signal to display driver chip. Used during PowerUp function.
 * @note       { Electronic Paper Display (EPD) command is used to configure display module }
 */
void hardwareReset(void)
{
  GPIO_OutSet(&rst);
  TMR_Delay(MXC_TMR0, MSEC(10), NULL);

  GPIO_OutClr(&rst);
  TMR_Delay(MXC_TMR0, MSEC(10), NULL);

  GPIO_OutSet(&rst);
  TMR_Delay(MXC_TMR0, MSEC(10), NULL);
}



/*
 * @brief	Sends specified array data from MAX32660 to electronic screen via SPI
 * @note       { Electronic Paper Display (EPD) command is used to configure display module }
 * 
 * @param[(in)] <buf> { Array of data to send }
 * @param[(in)] <len> { Length of data array }
 */
void EPD_data(uint8_t *buf, uint16_t len)
{
  GPIO_OutSet(&dc);

  SPItransfer(buf, len);

  GPIO_OutSet(&cs);
  GPIO_OutClr(&dc);
}

/*
 * @brief	Sends out the provided address via SPI protocol.
 * @note       { Electronic Paper Display (EPD) command is used to configure display module }
 * @param[(in)] <address> { Address information to send via SPI }
 * @param[(in)] <flag> { 1 de-selects module. 0 keeps module selected. (Continue or end transmission) }
 */
void EPD_command2(uint8_t address, int flag)
{
  uint8_t trans[1];
  trans[0] = address;
  GPIO_OutClr(&dc);
  GPIO_OutClr(&cs);

  SPItransfer(trans,1);

  if (flag == 1)
  {
	  GPIO_OutSet(&cs);
  }
}

/*
 * @brief	Used for configuring registers in display module.
 * @note       { Electronic Paper Display (EPD) command is used to configure display module }
 * 
 * @param[(in)] <address> { Address of desired register }
 * @param[(in)] <buf> { Array of buffer data which is used to configure register }
 * @param[(in)] <len> { Number of bytes to send from the selected buf array }
 */
void EPD_command1(uint8_t address, uint8_t *buf, uint16_t len)
{
  EPD_command2(address,0);
  EPD_data(buf, len);
}
/*
 * @brief	Boot-up the e-ink display. Runs basic startup commands based on display module's data sheet
 */

void powerUp(void)
{
  GPIO_OutSet(&en);
  TMR_Delay(MXC_TMR0, MSEC(200), NULL);
  uint8_t buf[5];
  hardwareReset();
  TMR_Delay(MXC_TMR0, MSEC(500), NULL);

  EPD_command2(SSD1608_SW_RESET, 1);

  TMR_Delay(MXC_TMR0, MSEC(500), NULL);

  buf[0] = 0xc7;
  buf[1] = 0x00;
  buf[2] = 0x00;
  EPD_command1(SSD1608_DRIVER_CONTROL, buf, 3);   //0x01

  buf[0] = 0x1B;
  EPD_command1(SSD1608_WRITE_DUMMY, buf, 1);    //0x3a

  buf[0] = 0x0B;
  EPD_command1(SSD1608_WRITE_GATELINE, buf, 1);   //0x3b

  buf[0] = 0x03;
  EPD_command1(SSD1608_DATA_MODE, buf, 1);    //0x11

  buf[0] = 0x00;
  buf[1] = 0x18;
  EPD_command1(SSD1608_SET_RAMXPOS, buf, 2);    //0x44

  buf[0] = 0x00;
  buf[1] = 0x00;
  buf[2] = 0xc7;
  buf[3] = 0x00;
  EPD_command1(SSD1608_SET_RAMYPOS, buf, 4);  //0x45

  buf[0] = 0x70;
  EPD_command1(SSD1608_WRITE_VCOM, buf, 1);   //0x2c

  EPD_command1(SSD1608_WRITE_LUT, LUT_DATA, 30);  //0x32
  GPIO_OutSet(&cs);
}

/*
 * @brief	Set display's RAM address pointer
 * @param[(in)] <x> { X address counter value }
 * @param[(in)] <y> { Y address counter value }
 */
void setRAM (uint16_t x, uint16_t y)
{
  uint8_t buf[2];
  buf[0] = x;
  EPD_command1(SSD1608_SET_RAMXCOUNT, buf, 1);

  buf[0] = y >> 8;
  buf[1] = y;
  EPD_command1(SSD1608_SET_RAMYCOUNT, buf, 2);
}

/*
 * @brief	Sends address to configure RAM
 */
void writeRAM()
{
  EPD_command2(SSD1608_WRITE_RAM, 0);
}

/*
 * @brief	Refreshes screen based on data most recent data sent from buffer1
 * 
 * @note       { Make sure data has been sent to screen via #BitMapTransfer -- Function already built into #dispalyScreen }
 */
void updateScreen (void)
{
  uint8_t buf[1];
  buf[0]= 0xC7;
  EPD_command1(SSD1608_DISP_CTRL2, buf, 1);

  EPD_command2(SSD1608_MASTER_ACTIVATE, 1);
  TMR_Delay(MXC_TMR0, MSEC(2000), NULL);
  GPIO_OutClr(&en);
}

/*
 * @brief	Sends new screen information by reversing every 8 bits (Gimp exports bitmaps least significant bit first, but display accepts most significant first)
 * @param[(in)] <logo> { Data Array with bits reversed before being sent via SPI }
 * @param[(in)] <len> { Number of bytes to send from data array }
 */
void BitMapTransfer(uint8_t *Design, int len)
{
	uint8_t buf[1];
	for (int i = 0; i<len;i++)
	{
		buf[0] = (((Design[i]>>7 & 0x01) + (Design[i]>>5 & 0x02) + (Design[i]>>3 & 0x04) + (Design[i]>>1 & 0x08) + (Design[i]<<1 & 0x10) + (Design[i]<<3 & 0x20) +(Design[i]<<5 & 0x40) + (Design[i]<<7 & 0x80)));
		SPItransfer(buf, 1);
	}
}

/*
 * @brief	Sends all of buffer1 data to screen, and then refreshes display
 */
void displayScreen(void)
{
  powerUp();
  setRAM(0, 0);
  writeRAM();

  GPIO_OutSet(&dc);
  BitMapTransfer(buffer1, ARRAY_SIZE);

  GPIO_OutSet(&cs);
  updateScreen();
}

/* @brief	Function used to edit screen buffer array and directly write edit individual pixels.
 * @param[(in)] <x> { Desired X position of Pixel }
 * @param[(in)] <y> { Desired Y position of Pixel }
 * @param[(in)] <color> { Desired color value (White = 1 ; Black = 0) }
 */

void drawPixel(int16_t x, int16_t y, int color)
{
  x = round (x);
  y = round (y);

  if((x < 0) || (y < 0) || (x >= 200) || (y >= 200)) return;		//Out of Range

  int value = x + (y * 200);		//Find index for specific pixel
  int rem = value % 8;				//Which bit to modify with index (0-7)
  int index = trunc(value/8);		//Finds the position in buffer1 to modify
  int shift = 7-rem;
  if(color ==1)
  {
	  int gate1 = (0xFF & (color << shift));
	  gate1 = (0xFF ^ gate1);
	  buffer1[index] = (buffer1[index] & gate1);
  }
  else
  {
	  int next;
	  int gate2 =0;
	  for(int i=0;i<8;i++)
	  {
		  if(i == shift)
		  {
			  next = 0;
		  }
		  else
		  {
			  next = 1;
		  }
		  gate2 = (gate2 << 1) | next;
	  }
	  buffer1[index] = (buffer1[index] & gate2);
  }
}

/*
 * @brief	Draw a line between two designated points. Function will update values in buffer1
 * @param[(in)] <x0> { X position of first point in line }
 * @param[(in)] <y0> { Y position of first point in line }
 * @param[(in)] <x1> { X position of second point in line }
 * @param[(in)] <y1> { Y position of second point in line }
 * @param[(in)] <color> { Line color (0 = black ; 1 = White) }
 */
void WriteLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, int color)
{
	int16_t steep = abs(y1 - y0) - abs(x1 - x0);
	int16_t hold;
	if (steep >= 1)
	{
		hold = x0;
		x0 = y0;
		y0 = hold;
		hold = x1;
        x1 = y1;
        y1 = hold;
	 }

	    if (x0 > x1) {
			hold = x0;
			x0 = x1;
			x1 = hold;
			hold = y1;
	        y1 = y0;
	        y0 = hold;
	    }

	    int16_t dx, dy;
	    dx = x1 - x0;
	    dy = abs(y1 - y0);

	    int16_t err = dx / 2;
	    int16_t ystep;

	    if (y0 < y1) {
	        ystep = 1;
	    } else {
	        ystep = -1;
	    }

	    for (; x0<=x1; x0++) {
	        if (steep >= 1) {
	            drawPixel(y0, x0, color);
	        }
	        else {
	            drawPixel(x0, y0, color);
	        }
	        err -= dy;
	        if (err < 0) {
	            y0 += ystep;
	            err += dx;
	        }
	    }
}

/*
 * @brief	Draw a Circle based on center and radius. Function will update buffer1
 * @param[(in)] <x0> { X component of circle center }
 * @param[(in)] <y0> { Y component of circle center }
 * @param[(in)] <r> { radius of circle }
 * @param[(in)] <color> { Circle color (0 = black ; 1 = White) }
 */
void drawCircle (uint8_t x0, uint8_t y0,uint8_t r, uint8_t color)
{
		int16_t f = 1 - r;
	    int16_t ddF_x = 1;
	    int16_t ddF_y = -2 * r;
	    int16_t x = 0;
	    int16_t y = r;


	    drawPixel(x0  , y0+r, color);
	    drawPixel(x0  , y0-r, color);
	    drawPixel(x0+r, y0  , color);
	    drawPixel(x0-r, y0  , color);

	    while (x<y) {
	        if (f >= 0) {
	            y--;
	            ddF_y += 2;
	            f += ddF_y;
	        }
	        x++;
	        ddF_x += 2;
	        f += ddF_x;

	        drawPixel(x0 + x, y0 + y, color);
	        drawPixel(x0 - x, y0 + y, color);
	        drawPixel(x0 + x, y0 - y, color);
	        drawPixel(x0 - x, y0 - y, color);
	        drawPixel(x0 + y, y0 + x, color);
	        drawPixel(x0 - y, y0 + x, color);
	        drawPixel(x0 + y, y0 - x, color);
	        drawPixel(x0 - y, y0 - x, color);
	    }
}
