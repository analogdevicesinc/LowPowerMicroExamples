# MAX32620FTHR USB CDC ACM DEMO

## Introduction
The basic concept for the example firmware is to make the MAX32620FTHR (FTHR) board look like a USB-to-serial port (with plug and play capabilities) by implementing the USB CDC ACM protocol which requires no additional driver installation (except in Windows). So, the end result should work on multiple operating systems including Linux and MacOS.  The end-user will open two serial port terminals on the PC, one for "serial debug" of the FTHR, and one for the actual CDC-ACM serial port.  Then, typing in one terminal will echo back in the other.  See below for a screenshot of two serial port terminals opened.  COM5 is the CDC ACM serial port and COM10 is the console debug serial port on the MAX32620FTHR.

![image](./images/EchoUart-to-USB.png)

## Hardware Setup
The hardware setup is basically the typical setup of the MAX32620FTHR board with the Daplink debugger (MAX32625PICO) attached with a ribbon cable.  The FTHR and Daplink boards, of course, are connected to spare USB ports of a PC.  See below for an image of the typical setup.

![image](./images/Hardware.png)

## Windows Device Driver Installation
The device driver installation script for Windows is included. It is digitally signed (the one in the kit may or may not be depending on the version).  This will allow for proper installation under Windows 10. To install, with the MAX32620FTHR unplugged from the USB port, simply right-click on the maxim_usb-uart_adapter.inf file and choose "install". Then, with the firmware loaded, plug the MAX32620FTHR board into a spare USB port.  To check if it was a successful install, look in the Windows "Device Manager".  See screenshot of a successful install below (with MAX32620FTHR plugged in).

![image](./images/CDC-ACM_SerialPort.png)

## Serial Port Terminal Settings
