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
*******************************************************************************
* @file MAX30205_Sensor.h
*
* @version 1.0
*
* Started: 10JUL19
*
* Updated: No current revisions
*/

#ifndef MAX30205_Sensor_H_
#define MAX30205_Sensor_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "i2c.h"

/***** I2C Declirations *****/
#define I2C_MASTER	    MXC_I2C1		//Set master to P0_2 and P0_3. Change to MXC_I2C0 to setup master as P0_8 (SCL) and P0_9 (SDA)
#define I2C_MASTER_IDX	0
#define I2C_SLAVE_IDX	1
#define SLAVE_ADDR	(0x90)				//MAX30205 Address with all address selection lines grounded (EVkit standard configuration)
#define I2C_TIMEOUT MXC_DELAY_MSEC(1)
#define BYTES_WRITTEN	2				//Quantitity of bytes sent from the MAX32660
#define	BYTES_READ		1				//Quantity of bytes sent from the MAX32660
#define	READ_ACKNOWLEDGE	1			//MAX32660 continues transmission after read or write, keeps MAX30205 selected
#define	WRITE_ACKNOWLEDGE	0			//MAX32660 ends transmission after read or write

i2c_req_t req;							//I2C Device structure
volatile int i2c_flag;
volatile int i2c_flag1;

/***** Device Register Addresses *****/
static uint8_t INTIALIZE_SLEEP[] = {0x01, 0x01};					//Config register address and sleep-mode configuration data 
static uint8_t CONFIGURATION_REGISTER_ADDR[] = {0x01, 0x00};		//Config register address and active mode setup
static uint8_t CONFIGURATION_ONESHOT[] = {0x01, 0x81};				//Config register address and one-shot configuration data
static uint8_t TEMPERATURE_REGISTER_ADDR[] = {0x00};				//Temperature register address


/**
 * @brief Eclipse Library for implementing the MAX30205 Human Body
 * Temperature Sensor. Library has functionality to read and write
 * to sensor registers via I2C communications. One-Shot functionality
 * is also enabled in example below.
 * 
 * 
 * @code
 *
 * //Include
 * #include <stdio.h>
 * #include <stdint.h>
 * #include <string.h>
 * #include "tmr_utils.h"
 * #include "mxc_delay.h"
 * #include "tmr.h"
 * #include "MAX30205_Sensor.h"
 * 
 * int main(void)
 * {
 * 
 * 	printf("This program uses MAX32660 P0_2 and P0_3 to\n");
 * 	printf("to read from a MAX30205 Temperature Sensor.\n");
 * 	printf("Readings will be displayed in both Celsius as well\n");
 * 	printf("as Fahrenheit onto the terminal window.\n"); 
 * 
 *    //Initialize Sensor Addresses and pins
 *     MAX30205_I2CSetup();
 * 
 *     //Set Sensor into Sleep Mode
 *    MAX30205_TempSenseSleep();
 * 
 *     //Main loop that continuously updates temperature on e-ink display
 *     while(1)
 *     {
 *     	//Send a One-shot signal
 *     	MAX30205_OneShotSense();
 * 
 *     	//Give time to make a new reading
 *     	TMR_Delay(MXC_TMR0, MSEC(50), NULL);
 * 
 *     	//Convert to Fahrenheit and display new value on screen
 *     	double Celsius = MAX30205_TempRead();
 *     	double Fahrenheit = MAX30205_CtoF(Celsius);
 *     	printf("The temperature is %f degrees Celsius\n",Celsius);
 *     	printf("The temperature is %f degrees Fahrenheit\n",Fahrenheit);
 *     	printf("\n\n\n\n\n\n\n\n\n");
 *     	TMR_Delay(MXC_TMR0, MSEC(2000), NULL);
 *     	}
 *     return (0);
 *  }
 *
 * @endcode
 */

/**** Functions ****/

/*
 * @brief	Initialize I2C protocol for MAX32660 microcontroller. Must be called before any read or write commands in project. Only needs to be called once in main function. Initialzies P0_2 (SCL) and P0_3 (SDA)
 */
void MAX30205_I2CSetup(void);

/*
 * @brief	Convert temperature reading from Celsius to Farenheit
 */
double MAX30205_CtoF(double Celsius);

/*
 * @brief	Place MAX30205 into sleep mode -- Sensor will stop continuous temperature measurments. Sensor waits for One-Shot signal
 */
void MAX30205_TempSenseSleep(void);

/*
 * @brief	Send a One-Shot signal to calculate a new temperature. Sensor must be placed into Sleep mode first
 */
void MAX30205_OneShotSense(void);

/*
 * @brief	Read value stored in temperature register
 */
double MAX30205_TempRead(void);

/*
 * @brief	Convert temperature register reading into degrees Celsius
 */
double MAX30205_TempCalc(uint8_t *data);

#endif /* MAX30205_Sensor_H_ */
