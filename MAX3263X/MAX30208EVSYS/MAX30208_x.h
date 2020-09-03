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
*	Also note that this project is setup using te MAX32630 EVkit in Maxim Toolchain in the Eclipse IDE.
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
* Updated: No current revisions
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

#ifndef Max30208_Sensor
#define Max30208_Sensor

/***** I2C Declirations *****/
#define I2C_MASTER          MXC_I2CM1 // Setup Master PinMap to P3_4 and P3_5 
#define I2C_MASTER_IDX      1

#define I2C_SLAVE           MXC_I2CS // Setup I2C Slave Port 
#define I2C_SLAVE_IDX       0
#define I2C_SLAVE_ADDR      0x50 //7-bit address, 8-bit address is 0xA0 write and 0xA1
#define I2C_SPEED           I2CS_SPEED_400KHZ

 /*****************************************************************************************************
 * I2CSetup
 * @brief: Setup and Initialize I2C protocol for MAX32630 microcontroller. 
 * @Note: Must be called before any read or write commands in project.
 * Only needs to be called once in main function. Initialzies P3_5 (SCL) and P3_4 (SDA)
 * -->Mapping the pins P3.4 and P3.5 for I2C operation (User Guide page 242)
 * -->Setup Clock_scale value for Slave + Master  ( Derived from main system )  
*******************************************************************************************************/
void I2CSetup (void);

 /*****************************************************************************************************
 * read_PartID
 * @brief: READ Part_ID Register from Max30208 
 * @param[out] data - PartID data from FIFO data register on successful read
 * @return 0 on success, non-zero on failure
 * Expecting print out Result: 0x30
******************************************************************************************************/
int read_PartID (uint8_t *data);

 /****************************************************************************************************************
 * read_OverFlow
 * @brief: READ FiFo_OverFlow Register from Max30208
 * @param[out] data - data from FIFO counter on successful read
 * @return 0 on success, non-zero on failure
****************************************************************************************************************/
int read_OverFlow (uint8_t *data);

 /***************************************************************************************************
 * read_StatusRegister
 * @brief: Read the Status Register (Max30208)
 * @param[out] data - data from status Register on successful read
 * @return 0 on success, non-zero on failure
 ***************************************************************************************************/
int read_StatusRegister (uint8_t *data);

 /************************************************************************************************************
 * write_SetupRegister
 * @brief: WRITE Setup Register (Address:0x14; Value: C1)  ; Default value = 0xC0, then  0xC0---> 0XC1
 * @return 0 on success, non-zero on failure
 * @Note:  (the last bit use for convert temp and we need to set it to one every time we want to read).
*************************************************************************************************************/
int write_SetupRegister (void);

/************************************************************************************************************
 * write_FiFoConfig1
 * @brief: WRITE Into FiFo Configuration_1 Register (Address:0x09; Value: 0x0F) ;
 * @return 0 on success, non-zero on failure
 * @Note: configure the Push and Pop on FiFo Register of Max30208
************************************************************************************************************/
int write_FiFoConfig1 (void);

/**************************************************************************************************************
 * read_FiFoDataRegister
 * @brief:	Reading 16-bits temperature Sample from FiFoData
 * @param[out] data -  2-data from FIFO Register on successful read
 * @return 0 on success, non-zero on failure
 * @Note:  READ FIFO DATA REG (2 BYTE)-->The Final result is 16-Bits data
 * -------->unint8_t FiFo_data[n] + unint8_t FiFo_data[n+1] = uint16_t result 
**************************************************************************************************************/
int read_FiFoDataRegister (uint16_t *data);

/***************************************************************************************************************
 * read_Temperature
 * @brief:	Convert unsigned 16-bits from Data FiFo register to Celsius unit
 * @param[in] data_fifo.
 * @return temperature in double type on successful call
***************************************************************************************************************/
double read_Temperature (uint16_t *data_fifo );


#endif /* Max30208_Sensor */
