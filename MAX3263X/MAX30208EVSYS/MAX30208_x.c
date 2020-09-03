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
/*	NOTE: This is a working library for the Max30208  Digital Temperature Sensor project. 
*	
*	Also note that this project is setup using t MAX32630 EVkit in Maxim Toolchain in the Eclipse IDE.
*	The MAX30208 sensor values are being read from the MAX30208 EVKit.

*******************************************************************************
* @brief Eclipse Library for implementing the MAX30208 Digital
* Temperature Sensor. Library has functionality to read and write
* to sensor registers via I2C communications.

* @file MAX30208.h
*
* @version 1.0
*
* Started: 8 JUL 2020
*
* Updated: 
*/

#include "Max30208_x.h"

/***** Device Register Address *****/
static uint8_t ID_ADDR[] = {0xFF}; // IDpart register Address = 0xff

static uint8_t Status_Register[] = {0x00}; // Status register Address = 0x00
static uint8_t FiFo_OverFlow[] = {0x06};   // FiFo OverFlow register Address = 0x06
static uint8_t FIFoConfig1[] = {0x09, 0x0F}; // FiFo_Config1 Register Address: 0x09 Value 0x0F

static uint8_t SETUP_ADDR[] = {0x14, 0xC1};		// setup_reg_add = 0x14 --> value = 0xC1
static uint8_t TEMP_ADDR[] = {0x08};			// FIFO_reg_add = 0x08 --> value = 2-Byte from Reg


/*
 * @brief: Setup and Initialize Configuration of Master and Salve 
 */
void I2CSetup (void)
{
	// Setup the I2CM
		    sys_cfg_i2cm_t i2cm_sys_cfg;
		    ioman_cfg_t io_cfg = IOMAN_I2CM1(IOMAN_MAP_A, 1); // Setup The pin map on EVKIT
		    i2cm_sys_cfg.clk_scale = CLKMAN_SCALE_DIV_1; // clock scale for master
		    i2cm_sys_cfg.io_cfg = io_cfg;
		    I2CM_Shutdown(I2C_MASTER);
		    I2CM_Init(I2C_MASTER, &i2cm_sys_cfg, I2C_SPEED);
	// Setup the I2CS
		    sys_cfg_i2cs_t i2cs_sys_cfg;
		    ioman_cfg_t i2cs_io_cfg = IOMAN_I2CS(I2C_SLAVE_IDX, 1);
		    i2cs_sys_cfg.clk_scale = CLKMAN_SCALE_DIV_1;
		    i2cs_sys_cfg.io_cfg = i2cs_io_cfg;
		    I2CS_Init(I2C_SLAVE, &i2cs_sys_cfg, I2C_SPEED, I2C_SLAVE_ADDR, 0);
}

/*
 * @brief: Read the Id_Part Register (Max30208)
 */
int  read_PartID (uint8_t *data)
{
	int numberOfByte = 0;
	int error = 0;
	uint8_t ID[1]={0};
 	if((numberOfByte = I2CM_Write(I2C_MASTER, I2C_SLAVE_ADDR, NULL, 0, ID_ADDR, 1)) != 1)//#Byte Transaction= 1
	{
	   error = numberOfByte;
	   return error;
	}
	if((error = I2CM_Read(I2C_MASTER, I2C_SLAVE_ADDR, NULL , 0 , ID, 1)) != 1)//#Byte Transaction= 1
	{
		error = numberOfByte ;
		return error;
	}
	else
	{
		*data = ID[0]; return error = 0;}
	}

/*
 * @brief: Read the OverFlow Register (Max30208)
 */
int  read_OverFlow (uint8_t *data)
{
	int numberOfByte = 0;
	int error = 0;
	uint8_t value[1];
 	if((numberOfByte = I2CM_Write(I2C_MASTER, I2C_SLAVE_ADDR, NULL, 0, FiFo_OverFlow, 1)) != 1)//#Byte Transaction= 1
	{
	   error = numberOfByte;
	   return error;
	}
	if((numberOfByte = I2CM_Read(I2C_MASTER, I2C_SLAVE_ADDR, NULL , 0 , value, 1)) != 1)//#Byte Transaction= 1
	{
		error = numberOfByte ;
		return error;
	}
	else
	{
		*data = value[0]; return error = 0;
	}
}

/*
 * @brief: Read the Status Register (Max30208)
 */
int read_StatusRegister (uint8_t *data)
{
		int numberOfByte = 0;
		int error = 0;
		uint8_t status[1];
	 	if((numberOfByte = I2CM_Write(I2C_MASTER, I2C_SLAVE_ADDR, NULL, 0, Status_Register, 1)) != 1)//#Byte Transaction= 1
		{
		   error = numberOfByte;
		   return error;
		}
		if((numberOfByte = I2CM_Read(I2C_MASTER, I2C_SLAVE_ADDR, NULL , 0 , status, 1)) != 1)//#Byte Transaction= 1
		{
			error = numberOfByte;
			return error;
		}
		else
		{
			*data = status[0];  return error = 0;
		}
}

/*
 * @brief: WRITE Setup Register Max30208-->Default value = 0xC0, then  0xC0---> 0XC1
 */
int write_SetupRegister (void)
{	
	int numberOfByte = 0;
	int error = 0;
	if((numberOfByte = I2CM_Write(I2C_MASTER, I2C_SLAVE_ADDR, NULL, 0, SETUP_ADDR, 2)) != 2) return error = numberOfByte;
	else return error = 0;
}

/*
 * @brief: WRITE Into FiFo Configuration_1 Register (Address:0x09; Value: 0x0F) ;
 */
int write_FiFoConfig1 (void)
{
	int numberOfByte = 0;
	int error = 0;
	if((numberOfByte = I2CM_Write(I2C_MASTER, I2C_SLAVE_ADDR, NULL, 0, FIFoConfig1, 2)) != 2) return error = numberOfByte;
	else return error = 0;
}

/*
 * @brief:	Reading 16-bits temperature Sample from FiFoData
 */
int read_FiFoDataRegister (uint16_t *data)
{
	int numberOfByte = 0;
	int error = 0;
	uint8_t temp[3];
	if((numberOfByte = I2CM_Write(I2C_MASTER, I2C_SLAVE_ADDR, NULL, 0, TEMP_ADDR, 1)) != 1)//#Byte Transaction= 1
	{
		 error = numberOfByte;
		 return error;
	}
	if ((numberOfByte = I2CM_Read(I2C_MASTER, I2C_SLAVE_ADDR, NULL, 0, temp, 3)) != 3)//#Byte Transaction= 3
	{
		 error = numberOfByte;
		 return error;
	}
	else
	{
	uint16_t combine = ((uint16_t)temp[1] << 8) | temp[2]; // COMBINE TO 16 BIT
	*data = combine;
	return error = 0;
	}
}

/*
 * @brief:	Convert unsigned 16-bits from Data FiFo register to Celsius unit
 */
double read_Temperature (uint16_t *data_fifo )
{
	double final = 0.000;
	final = (double)*data_fifo*0.005;
	return final;
}

