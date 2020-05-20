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
/*	NOTE: This is a working library for the "Wearable Temperature Sensor LP"
* 	project. An additional library, which offers similiar functionality
*	and Mbed support can be found at:
*		
*		https://os.mbed.com/components/MAX30205-Human-Body-Temperature-Sensor/
*
*	This library is a working version specific to this project.
*	
*	Also note that this project is setup using te MAX32660 EVkit in Maxim Toolchain in the Eclipse IDE.
*	The MAX30205 sensor values are being read from the MAX30205 Ev-Kit.
*******************************************************************************/
/*
* @file MAX30205_Sensor.c
*
* @version 1.0
*
* Started: 10JUL19
*
* Updated: No current revisions
*/

#include "MAX30205_Sensor.h"

/**
 * @brief	Initialize I2C protocol for MAX32660 microcontroller. Must be called before any read or write commands in project. Only needs to be called once in main function. Initialzies P0_2 (SCL) and P0_3 (SDA)
 */
void MAX30205_I2CSetup(void){
	const sys_cfg_i2c_t sys_i2c_cfg = NULL; /* No system specific configuration needed. */

	//Setup the I2CM
	I2C_Shutdown(I2C_MASTER);
	I2C_Init(I2C_MASTER, I2C_STD_MODE, &sys_i2c_cfg);
	NVIC_EnableIRQ(I2C1_IRQn);
}

/**
 * @brief	convert temperature reading from Celsius to Farenheit using F = C*(9/5)+32
 * @param[(in)] <Celsius> { temeprature reading in degress Celsius }
 *
 * @note       { Make sure to call #MAX30205_TempRead before calling this function. }
 * @return	temperature reading in degrees Farenheit
 */
double MAX30205_CtoF(double Celsius){
	double temp;
	temp = Celsius*9;
	temp = temp/5;
	temp += 32;
	return(temp);
}



/**
 * @brief	Place MAX30205 into sleep mode -- Sensor will stop continuous temperature measurments. Sensor waits for One-Shot signal
 * 
 * @note       { Make sure #MAX30205_I2CSETUP has been called at start of program before reading or writing any data. }
 */
void MAX30205_TempSenseSleep(void){
	if(I2C_MasterWrite(I2C_MASTER, SLAVE_ADDR, INTIALIZE_SLEEP, BYTES_WRITTEN, WRITE_ACKNOWLEDGE) != BYTES_WRITTEN){
		printf("ERROR WRITING CONFIGURATION REGISTER AND ENTERING SLEEP MODE\n");
	}		
}

/**
 * @brief	Send a One-Shot signal to calculate a new temperature. Sensor must be placed into Sleep mode first.
 * 
 * @note       { Make sure #MAX30205_I2CSETUP has been called before reading or writing any data. Also, make sure #MAX30205_TempSenseSleep has been called to put device into sleep mode}
 */
void MAX30205_OneShotSense(void){
	int error;
	if((error = I2C_MasterWrite(I2C_MASTER, SLAVE_ADDR, CONFIGURATION_ONESHOT, BYTES_WRITTEN, WRITE_ACKNOWLEDGE)) != BYTES_WRITTEN){
		printf("ERROR WRITING CONFIGURATION REGISTER\n");
	}
}

/**
 * @brief	Read value stored in temperature register
 *
 * @note       { Make sure #MAX30205_I2CSETUP has been called before reading or writing any data. }
 *
 * @return	Value of temperature in degrees Farenheit
 */
double MAX30205_TempRead(void){
	int error;
	int WRITE_SIZE = 2;
	uint8_t data[1];
	//Read Temperature
	if((error = I2C_MasterWrite(I2C_MASTER, SLAVE_ADDR, TEMPERATURE_REGISTER_ADDR, BYTES_READ, READ_ACKNOWLEDGE)) != BYTES_READ){
		printf("ERROR READING ACCESSING TEMPERATURE REGISTER\n");
	}
	if ((error = I2C_MasterRead(I2C_MASTER, SLAVE_ADDR, data, BYTES_WRITTEN, WRITE_ACKNOWLEDGE)) != BYTES_WRITTEN){
		printf("ERROR READING TEMPERATURE DATA\n");
	}
	double f = MAX30205_TempCalc(data);
	return (f);
}

/**
 * @brief	Convert temperature register reading into degrees Celsius
 *
 * @param 	[(in)] <data> { data array holding temperature register reading }
 * @note       { Make sure #MAX30205_TempRead has been called before calculating temperature }
 *
 * @return	Temperature reading in degrees Farenheit
 */
double MAX30205_TempCalc(uint8_t *data){
	double Celsius = data[0];
	int val;
	double power = -1;
	for(int j=7;j>0;j--){
		val = (data[1]>>j) & 1;
		Celsius += val*(pow(2,power));
		power--;
	}
	return(Celsius);
}

