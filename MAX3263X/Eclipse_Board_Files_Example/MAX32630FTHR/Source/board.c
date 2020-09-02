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

/* **** Includes **** */
#include <stdio.h>
#include "mxc_config.h"
#include "mxc_assert.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "spim.h"
#include "max14690.h"

/**
 * @ingroup board_evkit
 * @{
 */
/* **** Global Variables **** */

// LEDs
// Note: EvKit board uses 3.3v supply so these must be open-drain.
/**
 * Array that contains the GPIO configuration for the LED devices on the evaluation kit board. 
 * @note The Evaluation Kit board uses a 3.3V supply voltage, so these must be defined as open-drain. 
 */
const gpio_cfg_t led_pin[] = {
    { PORT_2, PIN_4, GPIO_FUNC_GPIO, GPIO_PAD_OPEN_DRAIN },
    { PORT_2, PIN_5, GPIO_FUNC_GPIO, GPIO_PAD_OPEN_DRAIN },
    { PORT_2, PIN_6, GPIO_FUNC_GPIO, GPIO_PAD_OPEN_DRAIN },
};
/**
 * Number of LEDs on the evaluation kit board.
 */
const unsigned int num_leds = (sizeof(led_pin) / sizeof(gpio_cfg_t));

// Pushbuttons
/**
 * Array that contains the GPIO configuration for Push Buttons on the evaluation kit board. 
 */
const gpio_cfg_t pb_pin[] = {
    { PORT_2, PIN_3, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP },
};

/**
 * Number of Push-buttons on the evaluation kit board.
 */
const unsigned int num_pbs = (sizeof(pb_pin) / sizeof(gpio_cfg_t));

// Console UART configuration
/**
 * Configuration structure that sets up the UART used for Console Input/Output as part of stdio.
 */
const uart_cfg_t console_uart_cfg = {
    .parity = UART_PARITY_DISABLE,
    .size = UART_DATA_SIZE_8_BITS,
    .extra_stop = 0,
    .cts = 0,
    .rts = 0,
    .baud = CONSOLE_BAUD,
};
/**
 * System configuration structure that sets the Clock and IO configruation for the UART used for the Console I/O as part of stdio.
 */
const sys_cfg_uart_t console_sys_cfg = {
    .clk_scale = CLKMAN_SCALE_DIV_4,
    .io_cfg = IOMAN_UART(CONSOLE_UART, IOMAN_MAP_A, IOMAN_MAP_UNUSED, IOMAN_MAP_UNUSED, 1, 0, 0)
};
const gpio_cfg_t console_uart_rx = { PORT_2, PIN_0, GPIO_FUNC_GPIO, GPIO_PAD_INPUT };       /**< GPIO configuration object for the UART Receive (RX) Pin for Console I/O. */
const gpio_cfg_t console_uart_tx = { PORT_2, PIN_1, GPIO_FUNC_GPIO, GPIO_PAD_INPUT };       /**< GPIO configuration object for the UART Transmit (TX) Pin for Console I/O. */


/**
 * MAX14690 PMIC configuration structure.
 */
const max14690_cfg_t max14690_cfg = {
  .ldo2mv = 3300, /**< 3.3v in mV, connected to VDDB */
  .ldo2mode = MAX14690_LDO_MPC1, /**< Enalbe LDO2 when +5v is present on VBUS */
  .ldo3mv = 3300,  /**< 3.3v is L3OUT -- optional */
  .ldo3mode = MAX14690_LDO_ENABLED /**< Enable the LDO. */
};
/**
 * I2C Master system configuration object for communicating with the MAX14690. 
 */
const sys_cfg_i2cm_t max14690_sys_cfg = {
        .clk_scale = CLKMAN_SCALE_DIV_1,
        .io_cfg = IOMAN_I2CM2(IOMAN_MAP_A, 1)
};
const gpio_cfg_t max14690_int   = { PORT_3, PIN_7, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP }; /**< GPIO configuration object for the INT pin interface to the MAX14690. */

/* **** File Scope Variables **** */

/* ************************************************************************** */
void mxc_assert(const char *expr, const char *file, int line)
{
    printf("MXC_ASSERT %s #%d: (%s)\n", file, line, expr);
    while (1);
}

/* ************************************************************************** */
int Board_Init(void)
{
    int err;

    if ((err = Console_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    if ((err = LED_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    if ((err = PB_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    /* Configure PMIC voltages */
    if ((err = MAX14690_Init(&max14690_cfg)) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }
     // Configure PMIC LDO output on VBUS detection 
    if ((err = MAX14690_InterruptInit()) != E_NO_ERROR) {
      MXC_ASSERT_FAIL();
      return err;
    }

    return E_NO_ERROR;
}

/* ************************************************************************** */
int Console_Init(void)
{
    int err;

    if ((err = UART_Init(MXC_UART_GET_UART(CONSOLE_UART), &console_uart_cfg, &console_sys_cfg)) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    return E_NO_ERROR;
}

/* ************************************************************************** */
int Console_PrepForSleep(void)
{
#if defined ( __GNUC__ )
    fflush(stdout);
#endif /* __GNUC__ */
    return UART_PrepForSleep(MXC_UART_GET_UART(CONSOLE_UART));
}

/**@} end of ingroup board_evkit */

