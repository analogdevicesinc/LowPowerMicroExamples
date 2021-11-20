/**
 * @file    main.c
 * @brief   USB CDC-ACM example
 * @details Configures the MAX32620FTHR as a USB-UART adapter for a PC. After loading the firmware, connect a USB
 *          cable from the FTHR board to a host PC. A new CDC-ACM COM port will appear on the PC. A Daplink debugger 
 *          adapter can also be connected from the board to the PC, which allows for debug messages to be sent through 
 *          UART1 of the FTHR. Open serial port terminals on the PC for both the CDC-ACM COM port and the Daplink. 
 *          Typing chars in one terminal window should echo on the other.
 * 
 *          For a Windows PC, the device driver install file is the .inf file in the \driver folder. Before 
 *          plugging in the adapter, right-click on this file and choose "install." This will associate the 
 *          Maxim FTHR board with Microsoft's CDC-ACM device driver. Then, connect the FTHR to the PC through USB.
 *          1. LED1 (green) should turn on once enumeration is complete.
 *          2. The CDC-ACM COM driver should appear in the Device Manager's list of COM ports as 
 *             "Maxim USB-to-UART Adapter (COMx)". Open a serial terminal to this COM port.
 *          3. Open another serial terminal to the console UART of the MAX32620FTHR connected to the Daplink 
 *             adapter, which is UART1 (TX = P2.1, RX = P2.0).
 *          4. COM settings of the serial port terminals:  Speed = 115200, Data = 8-bit, Parity = none, stop bits = 1, 
 *             Flow Control = none.
 *          5. Typing characters in one serial terminal will echo in the other.
 * 
 * @version 1.0.0
 * @notes   This firmware differs slightly from the one in the Low Power ARM Micro SDK (Win) in two ways:  It is 
 *          designed to work with the MAX32620FTHR and this comes as a VSCode project. The SDK is located here: 
 *          https://www.maximintegrated.com/content/maximintegrated/en/design/software-description.html/swpart=SFW0001500A
 *          
 *          Compiled with Maxim's Low Power ARM Micro Toolchain (Windows) ver 1.2.0 downloaded from:
 *          https://www.maximintegrated.com/en/design/software-description.html/swpart=SFW0001500A
 * 
 *          VSCode version 1.62.2 (x64 Windows)
 *          C\C++ Plugin V1.7.1
 *          VSCode-Maxim V1.2.0 https://github.com/MaximIntegratedTechSupport/VSCode-Maxim
 *
 *          Version History:
 *          version 1.0.0 First release.
 */

/* ******************************************************************************
 * Copyright (C) 2021 Maxim Integrated Products, Inc., All Rights Reserved.
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
 *
 **************************************************************************** */

#include <stdio.h>
#include <stddef.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "pwrman_regs.h"
#include "board.h"
#include "lp.h"
#include "led.h"
#include "uart.h"
#include "usb.h"
#include "usb_event.h"
#include "enumerate.h"
#include "cdc_acm.h"
#include "descriptors.h"

/* **** Definitions **** */
#define AppVersion "1.0.0"

#if (CONSOLE_UART == 1)
#define MXC_UARTn           MXC_UART1
#define UARTn_IRQn          UART1_IRQn
#define UARTn_IRQHandler    UART1_IRQHandler
#else
#define MXC_UARTn           MXC_UART0
#define UARTn_IRQn          UART0_IRQn
#define UARTn_IRQHandler    UART0_IRQHandler
#endif

#define EVENT_ENUM_COMP     MAXUSB_NUM_EVENTS
#define EVENT_REMOTE_WAKE   (EVENT_ENUM_COMP + 1)

/* **** Global Data **** */
volatile int configured;
volatile int suspended;
volatile unsigned int event_flags;
int remote_wake_en;

/* **** Function Prototypes **** */
static int setconfig_callback(usb_setup_pkt *sud, void *cbdata);
static int setfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int clrfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int event_callback(maxusb_event_t evt, void *data);
static void usb_app_sleep(void);
static void usb_app_wakeup(void);
static int configure_uart(void);
static int usb_read_callback(void);
static void uart_read_callback(uart_req_t *req, int err);
static void echo_usb(void);
static void echo_uart(void);

/* **** File Scope Variables **** */

/* This EP assignment must match the Configuration Descriptor */
static const acm_cfg_t acm_cfg = {
  1,                  /* EP OUT */
  MXC_USB_MAX_PACKET, /* OUT max packet size */
  2,                  /* EP IN */
  MXC_USB_MAX_PACKET, /* IN max packet size */
  3,                  /* EP Notify */
  MXC_USB_MAX_PACKET, /* Notify max packet size */
};

static uart_req_t uart_req;
static uint8_t uart_rx_data[MXC_UART_FIFO_DEPTH + 1];
static uint8_t uart_tx_data[MXC_UART_FIFO_DEPTH];
static volatile int usb_read_complete;
static volatile int uart_read_complete;

/* ************************************************************************** */
int main(void)
{
    /* Initialize state */
    configured = 0;
    suspended = 0;
    event_flags = 0;
    remote_wake_en = 0;

    /* Enable the USB clock and power */
    SYS_USB_Enable(1);

    /* Initialize the usb module */
    if (usb_init(NULL) != 0) {
        printf("usb_init() failed\n");
        while (1);
    }

    /* Initialize the enumeration module */
    if (enum_init() != 0) {
        printf("enum_init() failed\n");
        while (1);
    }

    /* Register enumeration data */
    enum_register_descriptor(ENUM_DESC_DEVICE, (uint8_t*)&device_descriptor, 0);
    enum_register_descriptor(ENUM_DESC_CONFIG, (uint8_t*)&config_descriptor, 0);
    enum_register_descriptor(ENUM_DESC_STRING, lang_id_desc, 0);
    enum_register_descriptor(ENUM_DESC_STRING, mfg_id_desc, 1);
    enum_register_descriptor(ENUM_DESC_STRING, prod_id_desc, 2);

    /* Handle configuration */
    enum_register_callback(ENUM_SETCONFIG, setconfig_callback, NULL);

    /* Handle feature set/clear */
    enum_register_callback(ENUM_SETFEATURE, setfeature_callback, NULL);
    enum_register_callback(ENUM_CLRFEATURE, clrfeature_callback, NULL);

    /* Initialize the class driver */
    if (acm_init() != 0) {
        printf("acm_init() failed\n");
        while (1);
    }

    /* Register callbacks */
    usb_event_enable(MAXUSB_EVENT_NOVBUS, event_callback, NULL);
    usb_event_enable(MAXUSB_EVENT_VBUS, event_callback, NULL);
    acm_register_callback(ACM_CB_SET_LINE_CODING, configure_uart);
    acm_register_callback(ACM_CB_READ_READY, usb_read_callback);
    usb_read_complete = 0;

    if (configure_uart() != 0) {
        printf("configure_uart() failed\n");
        while (1);
    }

    /* Start with USB in low power mode */
    usb_app_sleep();
    NVIC_EnableIRQ(USB_IRQn);
    NVIC_EnableIRQ(UARTn_IRQn);

    printf("\n\n***** MAX32620FTHR USB CDC-ACM Example version %s *****\n", AppVersion);
    printf("Waiting for VBUS...\n");

    /* Wait for events */
    while (1) {

        echo_usb();
        echo_uart();

        if (suspended || !configured) {
            LED_Off(0);
            LED_Off(1);
            LED_Off(2);
        } else {
            LED_On(1);
        }

        if (event_flags) {
            /* Display events */
            if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_NOVBUS)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_NOVBUS);
                printf("VBUS Disconnect\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_VBUS)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_VBUS);
                printf("VBUS Connect\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_BRST)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_BRST);
                printf("Bus Reset\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_SUSP)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_SUSP);
                printf("Suspended\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_DPACT)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_DPACT);
                printf("Resume\n");
            } else if (MXC_GETBIT(&event_flags, EVENT_ENUM_COMP)) {
                MXC_CLRBIT(&event_flags, EVENT_ENUM_COMP);
                printf("Enumeration complete. Waiting for characters...\n");
            } else if (MXC_GETBIT(&event_flags, EVENT_REMOTE_WAKE)) {
                MXC_CLRBIT(&event_flags, EVENT_REMOTE_WAKE);
                printf("Remote Wakeup\n");
            }
        } else {
            LP_EnterLP2();
        }
    }
}

/* ************************************************************************** */
static void echo_usb(void)
{
  int chars;

  if ((chars = acm_canread()) > 0) {

    if (chars > sizeof(uart_tx_data)) {
      chars = sizeof(uart_tx_data);
    }

    // Read the data from USB
    if (acm_read(uart_tx_data, chars) != chars) {
      printf("acm_read() failed\n");
      return;
    }

    // Echo it back
    if (acm_present()) {
      chars++;  /* account for the initial character */
      if (acm_write(uart_tx_data, chars) != chars) {
        printf("acm_write() failed\n");
      }
    }

    // Write to the UART
    UART_Write(MXC_UARTn, uart_tx_data, --chars);
  }
}

/* ************************************************************************** */
static void echo_uart(void)
{
  int available, bytes = 0;

  if (uart_read_complete) {
    available = UART_NumReadAvail(MXC_UARTn);

    /* read additional characters that may have been received */
    if (available > 0) {
      if (UART_Read(MXC_UARTn, &uart_rx_data[1], available, &bytes) != 0) {
        printf("uart_read() failed\n");
        return;
      }
    }

    // Echo it back
    bytes++;  /* account for the initial character */
    UART_Write(MXC_UARTn, uart_rx_data, bytes);

    // Write to the USB
    if (acm_present()) {
      if (acm_write(uart_rx_data, bytes) != bytes) {
        printf("acm_write() failed\n");
      }
    }

    /* Register the next read */
    uart_read_complete = 0;
    uart_req.data = uart_rx_data;
    uart_req.len = 1;
    uart_req.callback = uart_read_callback;
    UART_ReadAsync(MXC_UARTn, &uart_req);
  }
}

/* ************************************************************************** */
static int configure_uart(void)
{
    int err;
    const acm_line_t *params;
    uart_cfg_t uart_cfg;
    sys_cfg_uart_t uart_sys_cfg;

    /* get the current line coding parameters */
    params = acm_line_coding();

    /* some settings are fixed for this implementation */
    if ((params->stopbits != ACM_STOP_1) || (params->databits != 8)) {
        return -1;
    }

    uart_cfg.cts = 0;
    uart_cfg.rts = 0;
    uart_cfg.baud = params->speed;

    if (params->parity == ACM_PARITY_NONE) {
        uart_cfg.parity = UART_PARITY_DISABLE;
    } else if (params->parity == ACM_PARITY_ODD) {
        uart_cfg.parity = UART_PARITY_ODD;
    } else if (params->parity == ACM_PARITY_EVEN) {
        uart_cfg.parity = UART_PARITY_EVEN;
    } else if (params->parity == ACM_PARITY_MARK) {
        uart_cfg.parity = UART_PARITY_MARK;
    } else {
        return -1;
    }

    if (params->databits == 5) {
        uart_cfg.size = UART_DATA_SIZE_5_BITS;
    } else if (params->databits == 6) {
        uart_cfg.size = UART_DATA_SIZE_6_BITS;
    } else if (params->databits == 7) {
        uart_cfg.size = UART_DATA_SIZE_7_BITS;
    } else if (params->databits == 8) {
        uart_cfg.size = UART_DATA_SIZE_8_BITS;
    } else {
        return -1;
    }

    uart_cfg.extra_stop = (params->stopbits == ACM_STOP_1) ? 0 : 1;

    uart_sys_cfg.clk_scale = CLKMAN_SCALE_DIV_4;
    uart_sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_UART(MXC_UART_GET_IDX(MXC_UARTn), IOMAN_MAP_A, IOMAN_MAP_UNUSED, IOMAN_MAP_UNUSED, 1, 0, 0);

    if ((err = UART_Init(MXC_UARTn, &uart_cfg, &uart_sys_cfg)) != 0) {
        return err;
    }

    /* submit the initial read request */
    uart_read_complete = 0;
    uart_req.data = uart_rx_data;
    uart_req.len = 1;
    uart_req.callback = uart_read_callback;
    UART_ReadAsync(MXC_UARTn, &uart_req);

    return 0;
}

/* ************************************************************************** */
static int setconfig_callback(usb_setup_pkt *sud, void *cbdata)
{
    /* Confirm the configuration value */
    if (sud->wValue == config_descriptor.config_descriptor.bConfigurationValue) {
        configured = 1;
        MXC_SETBIT(&event_flags, EVENT_ENUM_COMP);
        return acm_configure(&acm_cfg); /* Configure the device class */
    } else if (sud->wValue == 0) {
        configured = 0;
        return acm_deconfigure();
    }

    return -1;
}

/* ************************************************************************** */
static int setfeature_callback(usb_setup_pkt *sud, void *cbdata)
{
    if(sud->wValue == FEAT_REMOTE_WAKE) {
        remote_wake_en = 1;
    } else {
        // Unknown callback
        return -1;
    }

    return 0;
}

/* ************************************************************************** */
static int clrfeature_callback(usb_setup_pkt *sud, void *cbdata)
{
    if(sud->wValue == FEAT_REMOTE_WAKE) {
        remote_wake_en = 0;
    } else {
        // Unknown callback
        return -1;
    }

    return 0;
}

/* ************************************************************************** */
static void usb_app_sleep(void)
{
    usb_sleep();
    MXC_PWRMAN->pwr_rst_ctrl &= ~MXC_F_PWRMAN_PWR_RST_CTRL_USB_POWERED;
    if (MXC_USB->dev_cn & MXC_F_USB_DEV_CN_CONNECT) {
        usb_event_clear(MAXUSB_EVENT_DPACT);
        usb_event_enable(MAXUSB_EVENT_DPACT, event_callback, NULL);
    } else {
        usb_event_disable(MAXUSB_EVENT_DPACT);
    }
    suspended = 1;
}

/* ************************************************************************** */
static void usb_app_wakeup(void)
{
    usb_event_disable(MAXUSB_EVENT_DPACT);
    MXC_PWRMAN->pwr_rst_ctrl |= MXC_F_PWRMAN_PWR_RST_CTRL_USB_POWERED;
    usb_wakeup();
    suspended = 0;
}

/* ************************************************************************** */
static int event_callback(maxusb_event_t evt, void *data)
{
    /* Set event flag */
    MXC_SETBIT(&event_flags, evt);

    switch (evt) {
        case MAXUSB_EVENT_NOVBUS:
            usb_event_disable(MAXUSB_EVENT_BRST);
            usb_event_disable(MAXUSB_EVENT_SUSP);
            usb_event_disable(MAXUSB_EVENT_DPACT);
            usb_disconnect();
            configured = 0;
            enum_clearconfig();
            acm_deconfigure();
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_VBUS:
            usb_event_clear(MAXUSB_EVENT_BRST);
            usb_event_enable(MAXUSB_EVENT_BRST, event_callback, NULL);
            usb_event_clear(MAXUSB_EVENT_SUSP);
            usb_event_enable(MAXUSB_EVENT_SUSP, event_callback, NULL);
            usb_connect();
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_BRST:
            usb_app_wakeup();
            enum_clearconfig();
            acm_deconfigure();
            configured = 0;
            suspended = 0;
            break;
        case MAXUSB_EVENT_SUSP:
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_DPACT:
            usb_app_wakeup();
            break;
        default:
            break;
    }

    return 0;
}

/* ************************************************************************** */
static int usb_read_callback(void)
{
    usb_read_complete = 1;
    return 0;
}

/* ************************************************************************** */
static void uart_read_callback(uart_req_t *req, int err)
{
    LED_Toggle(1);
    if (configured && suspended && remote_wake_en) {
        /* The bus is suspended. Wake up the host */
        usb_app_wakeup();
        usb_remote_wakeup();
        suspended = 0;
        MXC_SETBIT(&event_flags, EVENT_REMOTE_WAKE);
    }
    uart_read_complete = 1;
}

/* ************************************************************************** */
void USB_IRQHandler(void)
{
    usb_event_handler();
}

/* ************************************************************************** */
void UARTn_IRQHandler(void)
{
    UART_Handler(MXC_UARTn);
}
